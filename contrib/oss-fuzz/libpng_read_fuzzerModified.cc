// libpng_read_fuzzer.cc
// Copyright 2017-2018 Glenn Randers-Pehrson
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that may
// be found in the LICENSE file https://cs.chromium.org/chromium/src/LICENSE

// The modifications in 2017 by Glenn Randers-Pehrson include
// 1. addition of a PNG_CLEANUP macro,
// 2. setting the option to ignore ADLER32 checksums,
// 3. adding "#include <string.h>" which is needed on some platforms
//    to provide memcpy().
// 4. adding read_end_info() and creating an end_info structure.
// 5. adding calls to png_set_*() transforms commonly used by browsers.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#define PNG_INTERNAL
#include "png.h"

#define PNG_CLEANUP \
  if(png_handler.png_ptr) \
  { \
    if (png_handler.row_ptr) \
      png_free(png_handler.png_ptr, png_handler.row_ptr); \
    if (png_handler.end_info_ptr) \
      png_destroy_read_struct(&png_handler.png_ptr, &png_handler.info_ptr,\
        &png_handler.end_info_ptr); \
    else if (png_handler.info_ptr) \
      png_destroy_read_struct(&png_handler.png_ptr, &png_handler.info_ptr,\
        nullptr); \
    else \
      png_destroy_read_struct(&png_handler.png_ptr, nullptr, nullptr); \
    png_handler.png_ptr = nullptr; \
    png_handler.row_ptr = nullptr; \
    png_handler.info_ptr = nullptr; \
    png_handler.end_info_ptr = nullptr; \
  }

struct BufState {
  const uint8_t* data;
  size_t bytes_left;
};

struct PngObjectHandler {
  png_infop info_ptr = nullptr;
  png_structp png_ptr = nullptr;
  png_infop end_info_ptr = nullptr;
  png_voidp row_ptr = nullptr;
  BufState* buf_state = nullptr;

  ~PngObjectHandler() {
    if (row_ptr)
      png_free(png_ptr, row_ptr);
    if (end_info_ptr)
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);
    else if (info_ptr)
      png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    else
      png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    delete buf_state;
  }
};

// User-defined unknown chunk handler
int handle_unkown_chunk(png_structp png_ptr, png_unknown_chunkp chunk) {
  char name[5];
  memcpy(name, chunk->name, 4);
  name[4] = '\0';

  // Simple log to stdout (or remove for cleaner fuzzing)
  // printf("Unknown chunk encountered: %s (%zu bytes)\n", name, chunk->size);

  // Return 0 or 1 :
  // Return 1 to let libpng know I parsed the file
  // Return 0 to let libpng know it should also parse the file
  return (chunk->size + name[0]) % 2;
}

// // I overwrite the parsing method of libpng
// int handle_unknown_chunk_myself(png_structp png_ptr, png_unknown_chunkp chunk) {
//   char name[5];
//   memcpy(name, chunk->name, 4);
//   name[4] = '\0';

//   // Simple log to stdout (or remove for cleaner fuzzing)
//   // printf("Unknown chunk encountered: %s (%zu bytes)\n", name, chunk->size);

//   // Return 1 to let libpng know I parsed the file
//   return 1;
// }

void user_read_data(png_structp png_ptr, png_bytep data, size_t length) {
  BufState* buf_state = static_cast<BufState*>(png_get_io_ptr(png_ptr));
  if (length > buf_state->bytes_left) {
    png_error(png_ptr, "read error");
  }
  memcpy(data, buf_state->data, length);
  buf_state->bytes_left -= length;
  buf_state->data += length;
}

void* limited_malloc(png_structp, png_alloc_size_t size) {
  // libpng may allocate large amounts of memory that the fuzzer reports as
  // an error. In order to silence these errors, make libpng fail when trying
  // to allocate a large amount. This allocator used to be in the Chromium
  // version of this fuzzer.
  // This number is chosen to match the default png_user_chunk_malloc_max.
  if (size > 8000000)
    return nullptr;

  return malloc(size);
}

void default_free(png_structp, png_voidp ptr) {
  return free(ptr);
}

static const int kPngHeaderSize = 8;

// Entry point for LibFuzzer.
// Roughly follows the libpng book example:
// http://www.libpng.org/pub/png/book/chapter13.html
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < kPngHeaderSize) {
    return 0;
  }

  std::vector<unsigned char> v(data, data + size);
  if (png_sig_cmp(v.data(), 0, kPngHeaderSize)) {
    // not a PNG.
    return 0;
  }

  PngObjectHandler png_handler;
  png_handler.png_ptr = nullptr;
  png_handler.row_ptr = nullptr;
  png_handler.info_ptr = nullptr;
  png_handler.end_info_ptr = nullptr;

  png_handler.png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png_handler.png_ptr) {
    return 0;
  }

  png_handler.info_ptr = png_create_info_struct(png_handler.png_ptr);
  if (!png_handler.info_ptr) {
    PNG_CLEANUP
    return 0;
  }

  png_handler.end_info_ptr = png_create_info_struct(png_handler.png_ptr);
  if (!png_handler.end_info_ptr) {
    PNG_CLEANUP
    return 0;
  }

  // Use a custom allocator that fails for large allocations to avoid OOM.
  png_set_mem_fn(png_handler.png_ptr, nullptr, limited_malloc, default_free);

  png_set_crc_action(png_handler.png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
#ifdef PNG_IGNORE_ADLER32
  png_set_option(png_handler.png_ptr, PNG_IGNORE_ADLER32, PNG_OPTION_ON);
#endif

  // Setting up reading from buffer.
  png_handler.buf_state = new BufState();
  png_handler.buf_state->data = data + kPngHeaderSize;
  png_handler.buf_state->bytes_left = size - kPngHeaderSize;
  png_set_read_fn(png_handler.png_ptr, png_handler.buf_state, user_read_data);
  png_set_sig_bytes(png_handler.png_ptr, kPngHeaderSize);

  if (setjmp(png_jmpbuf(png_handler.png_ptr))) {
    PNG_CLEANUP
    return 0;
  }

  uint8_t randomness = data[size - 1];

  // Register unknown chunk callback (and add tEXT acillary chunk as unknown)
  png_byte chunk1[] = { 'I', 'H', 'D', 'R', '\0' };
  png_byte chunk2[] = { 'P', 'L', 'T', 'E', '\0' };
  png_byte chunk3[] = { 'I', 'D', 'A', 'T', '\0' };
  png_byte chunk4[] = { 'I', 'E', 'N', 'D', '\0' };
  png_byte chunk5[] = { 'b', 'K', 'G', 'D', '\0' };
  png_byte chunk6[] = { 'c', 'H', 'R', 'M', '\0' };
  png_byte chunk7[] = { 'c', 'I', 'C', 'P', '\0' };
  png_byte chunk8[] = { 'd', 'S', 'I', 'G', '\0' };
  png_byte chunk9[] = { 'e', 'X', 'I', 'f', '\0' };
  png_byte chunk10[] = { 'g', 'A', 'M', 'A', '\0' };
  png_byte chunk11[] = { 'h', 'I', 'S', 'T', '\0' };
  png_byte chunk12[] = { 'i', 'C', 'C', 'P', '\0' };
  png_byte chunk13[] = { 'i', 'T', 'X', 't', '\0' };
  png_byte chunk14[] = { 'p', 'H', 'Y', 's', '\0' };
  png_byte chunk15[] = { 's', 'B', 'I', 'T', '\0' };
  png_byte chunk16[] = { 's', 'P', 'L', 'T', '\0' };
  png_byte chunk17[] = { 's', 'R', 'G', 'B', '\0' };
  png_byte chunk18[] = { 's', 'T', 'E', 'R', '\0' };
  png_byte chunk19[] = { 't', 'E', 'X', 't', '\0' };
  png_byte chunk20[] = { 't', 'I', 'M', 'E', '\0' };
  png_byte chunk21[] = { 't', 'R', 'N', 'S', '\0' };
  png_byte chunk22[] = { 'z', 'T', 'X', 't', '\0' };
  png_byte chunk23[] = { data[size - 1], data[size - 2], data[size - 3], data[size - 4], '\0' };

  // Create an array of pointers to the chunks
  png_byte *chunks[] = { chunk1, chunk2, chunk3, chunk4, chunk5, chunk6, chunk7, chunk8, chunk9, chunk10, 
                         chunk11, chunk12, chunk13, chunk14, chunk15, chunk16, chunk17, chunk18, chunk19,
                         chunk20, chunk21, chunk22, chunk23 };

  int chosenChunkIdx = randomness % 23;
  int numAvailableChunks = 23 - chosenChunkIdx;
  int numChunksAffected = (randomness % (numAvailableChunks + 1 + 1)) - 1; // between -1 and 23 (capped not to go over the number of chunks)
  png_set_keep_unknown_chunks(png_handler.png_ptr, randomness % 4, chunks[chosenChunkIdx], numChunksAffected);

  // int (*handler)(png_structp, png_unknown_chunkp);
  // handler = randomness % 2 == 0 ? handle_unknown_chunk_myself : handle_unkown_chunk;
  png_set_read_user_chunk_fn(png_handler.png_ptr, nullptr, handle_unkown_chunk);

  // Reading the file.
  png_read_info(png_handler.png_ptr, png_handler.info_ptr);

  PNG_CLEANUP

  return 0;
}

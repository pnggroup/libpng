/* SPDX-License-Identifier: libpng-2.0
 * Validation driver for the progressive fuzzer; not a fuzz target.
 * The source include lets the test verify callback state as well as the oracle.
 */
#include "libpng_progressive_fuzzer.cc"
#include <vector>

static bool test_file(const char *path, int expected_complete) {
    FILE *file = fopen(path, "rb");
    if (!file) return false;
    std::vector<uint8_t> data;
    for (int ch; (ch = fgetc(file)) != EOF; ) data.push_back((uint8_t)ch);
    bool read_ok = !ferror(file);
    fclose(file);
    if (!read_ok || data.empty()) return false;
    DecodeCtx whole = {}, bytes = {};
    decode_progressive(data.data(), data.size(), data.size(), &whole);
    decode_progressive(data.data(), data.size(), 1, &bytes);
    bool complete = whole.end_seen && !whole.diagnostic_seen;
    bool ok = expected_complete < 0 ||
              (complete == !!expected_complete &&
               !!(bytes.end_seen && !bytes.diagnostic_seen) == !!expected_complete);
    if (expected_complete < 0)
        printf("%s clean_completion=%d diagnostics=%d/%d\n", path,
               complete, whole.diagnostic_seen, bytes.diagnostic_seen);
    free(whole.pixels);
    free(bytes.pixels);
    LLVMFuzzerTestOneInput(data.data(), data.size());
    if (!ok) fprintf(stderr, "Unexpected completion state: %s\n", path);
    return ok;
}

int main(int argc, char **argv) {
    if (argc == 2 && (strcmp(argv[1], "--invalid-row") == 0 ||
                      strcmp(argv[1], "--invalid-pass") == 0)) {
        DecodeCtx context = {};
        context.header_seen = 1;
        context.width = context.height = 1;
        context.rowbytes = 1;
        context.pixels = (unsigned char*)calloc(1, 1);
        png_structp p = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                               &context, on_error, on_warning);
        if (!p || !context.pixels) return 2;
        png_set_progressive_read_fn(p, &context, info_callback, row_callback,
                                    end_callback);
        /* A callback contract violation must not be swallowed as a PNG error. */
        if (setjmp(png_jmpbuf(p))) return 0;
        row_callback(p, NULL, strcmp(argv[1], "--invalid-row") == 0 ? 1 : 0,
                     strcmp(argv[1], "--invalid-pass") == 0 ? 7 : 0);
        return 0;
    }
    if (argc < 3) return 2;
    /* First file: libpng's existing badadler.png, expected to be rejected.
     * Second file: a clean upstream PNG fixture, expected to complete.
     * Remaining files: recovered seeds; record their diagnostic classification.
     */
    if (!test_file(argv[1], false)) return 1;
    if (!test_file(argv[2], true)) return 1;
    for (int i = 3; i < argc; ++i)
        if (!test_file(argv[i], -1)) return 1;
    printf("PASS: malformed Adler stream excluded, clean fixture completes, %d seed oracles agree\n", argc - 3);
    return 0;
}

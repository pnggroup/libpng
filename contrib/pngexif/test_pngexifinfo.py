import struct
import unittest
import zlib

import pngexifinfo


class PngExifInfoDecompressionTests(unittest.TestCase):
    def test_accepts_output_at_limit(self):
        raw = b"II\x2a\x00" + b"A" * (pngexifinfo._READ_DATA_SIZE_MAX - 4)
        self.assertEqual(
            pngexifinfo._extract_png_exif(b"\x00" + zlib.compress(raw)), raw)

    def test_rejects_output_above_limit(self):
        raw = b"II\x2a\x00" + b"A" * (pngexifinfo._READ_DATA_SIZE_MAX - 3)
        with self.assertRaises(RuntimeError) as error:
            pngexifinfo._extract_png_exif(b"\x00" + zlib.compress(raw))
        self.assertIn("too large", str(error.exception))

    def test_rejects_declared_output_above_limit(self):
        size = pngexifinfo._READ_DATA_SIZE_MAX + 1
        raw = b"II\x2a\x00" + b"A" * (size - 4)
        payload = b"\x00" + struct.pack(">I", size) + zlib.compress(raw)
        with self.assertRaises(RuntimeError) as error:
            pngexifinfo._extract_png_exif(payload)
        self.assertIn("too large", str(error.exception))

    def test_preserves_truncated_stream_error(self):
        raw = b"II\x2a\x00" + b"A" * 100
        with self.assertRaises(zlib.error):
            pngexifinfo._extract_png_exif(b"\x00" + zlib.compress(raw)[:-1])


if __name__ == "__main__":
    unittest.main()

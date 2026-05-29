#[no_mangle]
pub extern "C" fn png_rust_safe_rowbytes(pixel_depth: u32, width: u32) -> i64 {
    // Meniru rumus makro C: (pixel_depth * width + 7) >> 3
    // Menggunakan fungsi checked untuk mendeteksi overflow secara mutlak
    let total_bits = match (pixel_depth as u64).checked_mul(width as u64) {
        Some(bits) => bits,
        None => return -1, // Kembalikan -1 jika terdeteksi overflow saat perkalian
    };

    let padded_bits = match total_bits.checked_add(7) {
        Some(bits) => bits,
        None => return -1, // Kembalikan -1 jika overflow saat ditambah 7
    };

    (padded_bits >> 3) as i64
}

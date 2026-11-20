/* riscv/check.h - RISC-V Vector optimized filter functions
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#if defined(__riscv) && defined(__riscv_v) && (__riscv_v >= 1000000) && \
    defined(__riscv_v_intrinsic) && (__riscv_v_intrinsic >= 12000)
#  define PNG_TARGET_CODE_IMPLEMENTATION "riscv/riscv_init.c"
   /* PNG_TARGET_STORES_DATA */
#  define PNG_TARGET_IMPLEMENTS_FILTERS
   /* PNG_TARGET_IMPLEMENTS_EXPAND_PALETTE */
   /* PNG_TARGET_ROW_ALIGNMENT */
#endif /* __riscv && RVV */

/* pngtypes.h - array of chunk-types for libpng
 *
 * libpng 1.0.5c - November 27, 1999
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
 * Copyright (c) 1996, 1997 Andreas Dilger
 * Copyright (c) 1998, 1999 Glenn Randers-Pehrson
 */

/* Constant strings for known chunk types.  If you need to add a chunk,
 * add a string holding the name here.
 *
 * We can't selectively include these, since we still check for chunk in
 * the wrong locations with these labels. (I'm not exactly sure what
 * this comment means.  I inherited it from libpng-0.96 -- glennrp)
 */

const png_byte png_IHDR[5] = { 73,  72,  68,  82, '\0'};
const png_byte png_IDAT[5] = { 73,  68,  65,  84, '\0'};
const png_byte png_IEND[5] = { 73,  69,  78,  68, '\0'};
const png_byte png_PLTE[5] = { 80,  76,  84,  69, '\0'};
const png_byte png_bKGD[5] = { 98,  75,  71,  68, '\0'};
const png_byte png_cHRM[5] = { 99,  72,  82,  77, '\0'};
const png_byte png_gAMA[5] = {103,  65,  77,  65, '\0'};
const png_byte png_hIST[5] = {104,  73,  83,  84, '\0'};
const png_byte png_oFFs[5] = {111,  70,  70, 115, '\0'};
const png_byte png_pCAL[5] = {112,  67,  65,  76, '\0'};
const png_byte png_pHYs[5] = {112,  72,  89, 115, '\0'};
const png_byte png_sBIT[5] = {115,  66,  73,  84, '\0'};
const png_byte png_sRGB[5] = {115,  82,  71,  66, '\0'};
const png_byte png_tEXt[5] = {116,  69,  88, 116, '\0'};
const png_byte png_tIME[5] = {116,  73,  77,  69, '\0'};
const png_byte png_tRNS[5] = {116,  82,  78,  83, '\0'};
const png_byte png_zTXt[5] = {122,  84,  88, 116, '\0'};


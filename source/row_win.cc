/*
 *  Copyright (c) 2011 The LibYuv project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "row.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

#ifdef HAS_ARGBTOYROW_SSSE3

// Constant multiplication table for converting ARGB to I400.
static const vec8 kARGBToY = {
  13, 65, 33, 0, 13, 65, 33, 0, 13, 65, 33, 0, 13, 65, 33, 0
};

static const vec8 kARGBToU = {
  112, -74, -38, 0, 112, -74, -38, 0, 112, -74, -38, 0, 112, -74, -38, 0
};

static const vec8 kARGBToV = {
  -18, -94, 112, 0, -18, -94, 112, 0, -18, -94, 112, 0, -18, -94, 112, 0,
};

// Constants for BGRA
static const vec8 kBGRAToY = {
  0, 33, 65, 13, 0, 33, 65, 13, 0, 33, 65, 13, 0, 33, 65, 13
};

static const vec8 kBGRAToU = {
  0, -38, -74, 112, 0, -38, -74, 112, 0, -38, -74, 112, 0, -38, -74, 112
};

static const vec8 kBGRAToV = {
  0, 112, -94, -18, 0, 112, -94, -18, 0, 112, -94, -18, 0, 112, -94, -18
};

// Constants for ABGR
static const vec8 kABGRToY = {
  33, 65, 13, 0, 33, 65, 13, 0, 33, 65, 13, 0, 33, 65, 13, 0
};

static const vec8 kABGRToU = {
  -38, -74, 112, 0, -38, -74, 112, 0, -38, -74, 112, 0, -38, -74, 112, 0
};

static const vec8 kABGRToV = {
  112, -94, -18, 0, 112, -94, -18, 0, 112, -94, -18, 0, 112, -94, -18, 0
};

static const uvec8 kAddY16 = {
  16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u
};

static const uvec8 kAddUV128 = {
  128u, 128u, 128u, 128u, 128u, 128u, 128u, 128u,
  128u, 128u, 128u, 128u, 128u, 128u, 128u, 128u
};

// Shuffle table for converting RGB24 to ARGB.
static const uvec8 kShuffleMaskRGB24ToARGB = {
  0u, 1u, 2u, 12u, 3u, 4u, 5u, 13u, 6u, 7u, 8u, 14u, 9u, 10u, 11u, 15u
};

// Shuffle table for converting RAW to ARGB.
static const uvec8 kShuffleMaskRAWToARGB = {
  2u, 1u, 0u, 12u, 5u, 4u, 3u, 13u, 8u, 7u, 6u, 14u, 11u, 10u, 9u, 15u
};

// Shuffle table for converting ABGR to ARGB.
static const uvec8 kShuffleMaskABGRToARGB = {
  2u, 1u, 0u, 3u, 6u, 5u, 4u, 7u, 10u, 9u, 8u, 11u, 14u, 13u, 12u, 15u
};

// Shuffle table for converting BGRA to ARGB.
static const uvec8 kShuffleMaskBGRAToARGB = {
  3u, 2u, 1u, 0u, 7u, 6u, 5u, 4u, 11u, 10u, 9u, 8u, 15u, 14u, 13u, 12u
};

// Shuffle table for converting ARGB to RGB24.
static const uvec8 kShuffleMaskARGBToRGB24 = {
  0u, 1u, 2u, 4u, 5u, 6u, 8u, 9u, 10u, 12u, 13u, 14u, 128u, 128u, 128u, 128u };


// Shuffle table for converting ARGB to RAW.
static const uvec8 kShuffleMaskARGBToRAW = {
  2u, 1u,0u, 6u, 5u, 4u, 10u, 9u, 8u, 14u, 13u, 12u, 128u, 128u, 128u, 128u };

__declspec(naked)
void I400ToARGBRow_SSE2(const uint8* src_y, uint8* dst_argb, int pix) {
  __asm {
    mov        eax, [esp + 4]        // src_y
    mov        edx, [esp + 8]        // dst_argb
    mov        ecx, [esp + 12]       // pix
    pcmpeqb    xmm5, xmm5            // generate mask 0xff000000
    pslld      xmm5, 24

  convertloop:
    movq       xmm0, qword ptr [eax]
    lea        eax,  [eax + 8]
    punpcklbw  xmm0, xmm0
    movdqa     xmm1, xmm0
    punpcklwd  xmm0, xmm0
    punpckhwd  xmm1, xmm1
    por        xmm0, xmm5
    por        xmm1, xmm5
    movdqa     [edx], xmm0
    movdqa     [edx + 16], xmm1
    lea        edx, [edx + 32]
    sub        ecx, 8
    ja         convertloop
    ret
  }
}

__declspec(naked)
void ABGRToARGBRow_SSSE3(const uint8* src_abgr, uint8* dst_argb, int pix) {
__asm {
    mov       eax, [esp + 4]   // src_abgr
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    movdqa    xmm5, kShuffleMaskABGRToARGB

 convertloop:
    movdqa    xmm0, [eax]
    lea       eax, [eax + 16]
    pshufb    xmm0, xmm5
    movdqa    [edx], xmm0
    lea       edx, [edx + 16]
    sub       ecx, 4
    ja        convertloop
    ret
  }
}

__declspec(naked)
void BGRAToARGBRow_SSSE3(const uint8* src_bgra, uint8* dst_argb, int pix) {
__asm {
    mov       eax, [esp + 4]   // src_bgra
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    movdqa    xmm5, kShuffleMaskBGRAToARGB

 convertloop:
    movdqa    xmm0, [eax]
    lea       eax, [eax + 16]
    pshufb    xmm0, xmm5
    movdqa    [edx], xmm0
    lea       edx, [edx + 16]
    sub       ecx, 4
    ja        convertloop
    ret
  }
}

__declspec(naked)
void RGB24ToARGBRow_SSSE3(const uint8* src_rgb24, uint8* dst_argb, int pix) {
__asm {
    mov       eax, [esp + 4]   // src_rgb24
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    pcmpeqb   xmm5, xmm5       // generate mask 0xff000000
    pslld     xmm5, 24
    movdqa    xmm4, kShuffleMaskRGB24ToARGB

 convertloop:
    movdqa    xmm0, [eax]
    movdqa    xmm1, [eax + 16]
    movdqa    xmm3, [eax + 32]
    lea       eax, [eax + 48]
    movdqa    xmm2, xmm3
    palignr   xmm2, xmm1, 8    // xmm2 = { xmm3[0:3] xmm1[8:15]}
    pshufb    xmm2, xmm4
    por       xmm2, xmm5
    palignr   xmm1, xmm0, 12   // xmm1 = { xmm3[0:7] xmm0[12:15]}
    pshufb    xmm0, xmm4
    movdqa    [edx + 32], xmm2
    por       xmm0, xmm5
    pshufb    xmm1, xmm4
    movdqa    [edx], xmm0
    por       xmm1, xmm5
    palignr   xmm3, xmm3, 4    // xmm3 = { xmm3[4:15]}
    pshufb    xmm3, xmm4
    movdqa    [edx + 16], xmm1
    por       xmm3, xmm5
    movdqa    [edx + 48], xmm3
    lea       edx, [edx + 64]
    sub       ecx, 16
    ja        convertloop
    ret
  }
}

__declspec(naked)
void RAWToARGBRow_SSSE3(const uint8* src_raw, uint8* dst_argb,
                        int pix) {
__asm {
    mov       eax, [esp + 4]   // src_raw
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    pcmpeqb   xmm5, xmm5       // generate mask 0xff000000
    pslld     xmm5, 24
    movdqa    xmm4, kShuffleMaskRAWToARGB

 convertloop:
    movdqa    xmm0, [eax]
    movdqa    xmm1, [eax + 16]
    movdqa    xmm3, [eax + 32]
    lea       eax, [eax + 48]
    movdqa    xmm2, xmm3
    palignr   xmm2, xmm1, 8    // xmm2 = { xmm3[0:3] xmm1[8:15]}
    pshufb    xmm2, xmm4
    por       xmm2, xmm5
    palignr   xmm1, xmm0, 12   // xmm1 = { xmm3[0:7] xmm0[12:15]}
    pshufb    xmm0, xmm4
    movdqa    [edx + 32], xmm2
    por       xmm0, xmm5
    pshufb    xmm1, xmm4
    movdqa    [edx], xmm0
    por       xmm1, xmm5
    palignr   xmm3, xmm3, 4    // xmm3 = { xmm3[4:15]}
    pshufb    xmm3, xmm4
    movdqa    [edx + 16], xmm1
    por       xmm3, xmm5
    movdqa    [edx + 48], xmm3
    lea       edx, [edx + 64]
    sub       ecx, 16
    ja        convertloop
    ret
  }
}

#ifdef SHIFT565
// Below shift/mask code is efficient and works, but more instructions than
// pmul method
// TODO(fbarchard): Port RGB565ToARGBRow_SSE2 to gcc
// 29 instructions
__declspec(naked)
void OldRGB565ToARGBRow_SSE2(const uint8* src_rgb565, uint8* dst_argb,
                             int pix) {
__asm {
    mov       eax, [esp + 4]   // src_rgb565
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    pcmpeqb   xmm5, xmm5       // generate mask 0xff000000 for Alpha
    pslld     xmm5, 24
    pcmpeqb   xmm4, xmm4       // generate mask 0xf800f800 for Red
    psllw     xmm4, 11
    pcmpeqb   xmm6, xmm6       // generate mask 0x001f001f for Blue
    psrlw     xmm6, 11
    pcmpeqb   xmm7, xmm7       // generate mask 0x00fc00fc for Green
    psrlw     xmm7, 10
    psllw     xmm7, 2

 convertloop:
    movdqa    xmm0, [eax] // fetch 8 pixels of bgr565
    lea       eax, [eax + 16]
    movdqa    xmm1, xmm0
    movdqa    xmm2, xmm0
    pand      xmm1, xmm4    // R in upper 5 bits
    psrlw     xmm2, 13      // R 3 bits
    psllw     xmm2, 8
    por       xmm1, xmm2
    movdqa    xmm2, xmm0
    pand      xmm2, xmm6    // mask B 5 bits
    movdqa    xmm3, xmm2
    psllw     xmm2, 3
    psrlw     xmm3, 2
    por       xmm2, xmm3
    por       xmm1, xmm2    // RB
    psrlw     xmm0, 3       // G in top 6 bits of lower byte
    pand      xmm0, xmm7    // mask G 6 bits
    movdqa    xmm2, xmm0
    psrlw     xmm2, 6
    por       xmm0, xmm2
    por       xmm0, xmm5   // AG
    movdqa    xmm2, xmm1
    punpcklbw xmm1, xmm0
    punpckhbw xmm2, xmm0
    movdqa    [edx], xmm1  // store 4 pixels of ARGB
    movdqa    [edx + 16], xmm2  // store next 4 pixels of ARGB
    lea       edx, [edx + 32]
    sub       ecx, 8
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port ARGB1555ToARGBRow_SSE2 to gcc
// 33 instructions
__declspec(naked)
void OldARGB1555ToARGBRow_SSE2(const uint8* src_argb1555, uint8* dst_argb,
                               int pix) {
__asm {
    mov       eax, [esp + 4]   // src_argb1555
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    pcmpeqb   xmm5, xmm5       // generate mask 0xff00ff00 for Alpha
    psllw     xmm5, 8
    pcmpeqb   xmm4, xmm4       // generate mask 0xf800f800 for Red
    psllw     xmm4, 11
    pcmpeqb   xmm6, xmm6       // generate mask 0x001f001f for Blue
    psrlw     xmm6, 11
    pcmpeqb   xmm7, xmm7       // generate mask 0x00f800f8 for Green
    psrlw     xmm7, 11
    psllw     xmm7, 3

 convertloop:
    movdqa    xmm0, [eax] // fetch 8 pixels of bgr565
    lea       eax, [eax + 16]
    movdqa    xmm1, xmm0
    psllw     xmm1, 1
    movdqa    xmm2, xmm0
    pand      xmm1, xmm4    // R in upper 5 bits
    psrlw     xmm2, 13      // R 3 bits
    psllw     xmm2, 8
    por       xmm1, xmm2
    movdqa    xmm2, xmm0
    pand      xmm2, xmm6    // mask B 5 bits
    movdqa    xmm3, xmm2
    psllw     xmm2, 3
    psrlw     xmm3, 2
    por       xmm2, xmm3
    por       xmm1, xmm2    // RB
    movdqa    xmm2, xmm0
    psrlw     xmm2, 2       // G in top 5 bits of lower byte
    pand      xmm2, xmm7    // mask G 5 bits
    movdqa    xmm3, xmm2
    psrlw     xmm3, 5
    por       xmm2, xmm3
    psraw     xmm0, 8       // A
    pand      xmm0, xmm5
    por       xmm0, xmm2    // AG
    movdqa    xmm2, xmm1
    punpcklbw xmm1, xmm0
    punpckhbw xmm2, xmm0
    movdqa    [edx], xmm1  // store 4 pixels of ARGB
    movdqa    [edx + 16], xmm2  // store next 4 pixels of ARGB
    lea       edx, [edx + 32]
    sub       ecx, 8
    ja        convertloop
    ret
  }
}
#endif

// pmul method to replicate bits
// Math to replicate bits
// (v << 8) | (v << 3)
// v * 256 + v * 8
// v * (256 + 8)
// G shift of 5 is incorporated, so shift is 5 + 8 and 5 + 3
// 20 instructions
__declspec(naked)
void RGB565ToARGBRow_SSE2(const uint8* src_rgb565, uint8* dst_argb,
                          int pix) {
__asm {
    mov       eax, 0x01080108  // generate multiplier to repeat 5 bits
    movd      xmm5, eax
    pshufd    xmm5, xmm5, 0
    mov       eax, 0x20082008  // multiplier shift by 5 and then repeat 6 bits
    movd      xmm6, eax
    pshufd    xmm6, xmm6, 0
    pcmpeqb   xmm3, xmm3       // generate mask 0xf800f800 for Red
    psllw     xmm3, 11
    pcmpeqb   xmm4, xmm4       // generate mask 0x07e007e0 for Green
    psllw     xmm4, 10
    psrlw     xmm4, 5
    pcmpeqb   xmm7, xmm7       // generate mask 0xff00ff00 for Alpha
    psllw     xmm7, 8

    mov       eax, [esp + 4]   // src_rgb565
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    sub       edx, eax
    sub       edx, eax

 convertloop:
    movdqa    xmm0, [eax]   // fetch 8 pixels of bgr565
    movdqa    xmm1, xmm0
    movdqa    xmm2, xmm0
    pand      xmm1, xmm3    // R in upper 5 bits
    psllw     xmm2, 11      // B in upper 5 bits
    pmulhuw   xmm1, xmm5    // * (256 + 8)
    pmulhuw   xmm2, xmm5    // * (256 + 8)
    psllw     xmm1, 8
    por       xmm1, xmm2    // RB
    pand      xmm0, xmm4    // G in middle 6 bits
    pmulhuw   xmm0, xmm6    // << 5 * (256 + 4)
    por       xmm0, xmm7    // AG
    movdqa    xmm2, xmm1
    punpcklbw xmm1, xmm0
    punpckhbw xmm2, xmm0
    movdqa    [eax * 2 + edx], xmm1  // store 4 pixels of ARGB
    movdqa    [eax * 2 + edx + 16], xmm2  // store next 4 pixels of ARGB
    lea       eax, [eax + 16]
    sub       ecx, 8
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port ARGB1555ToARGBRow_SSE2 to gcc
// 24 instructions
__declspec(naked)
void ARGB1555ToARGBRow_SSE2(const uint8* src_argb1555, uint8* dst_argb,
                            int pix) {
__asm {
    mov       eax, 0x01080108  // generate multiplier to repeat 5 bits
    movd      xmm5, eax
    pshufd    xmm5, xmm5, 0
    mov       eax, 0x42004200  // multiplier shift by 6 and then repeat 5 bits
    movd      xmm6, eax
    pshufd    xmm6, xmm6, 0
    pcmpeqb   xmm3, xmm3       // generate mask 0xf800f800 for Red
    psllw     xmm3, 11
    pcmpeqb   xmm4, xmm4       // generate mask 0x03e003e0 for Green
    psllw     xmm4, 11
    psrlw     xmm4, 6
    pcmpeqb   xmm7, xmm7       // generate mask 0xff00ff00 for Alpha
    psllw     xmm7, 8

    mov       eax, [esp + 4]   // src_argb1555
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    sub       edx, eax
    sub       edx, eax

 convertloop:
    movdqa    xmm0, [eax]   // fetch 8 pixels of 1555
    movdqa    xmm1, xmm0
    movdqa    xmm2, xmm0
    psllw     xmm1, 1       // R in upper 5 bits
    psllw     xmm2, 11      // B in upper 5 bits
    pand      xmm1, xmm3
    pmulhuw   xmm2, xmm5    // * (256 + 8)
    pmulhuw   xmm1, xmm5    // * (256 + 8)
    psllw     xmm1, 8
    por       xmm1, xmm2    // RB
    movdqa    xmm2, xmm0
    pand      xmm0, xmm4    // G in middle 5 bits
    psraw     xmm2, 8       // A
    pmulhuw   xmm0, xmm6    // << 6 * (256 + 8)
    pand      xmm2, xmm7
    por       xmm0, xmm2    // AG
    movdqa    xmm2, xmm1
    punpcklbw xmm1, xmm0
    punpckhbw xmm2, xmm0
    movdqa    [eax * 2 + edx], xmm1  // store 4 pixels of ARGB
    movdqa    [eax * 2 + edx + 16], xmm2  // store next 4 pixels of ARGB
    lea       eax, [eax + 16]
    sub       ecx, 8
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port ARGB4444ToARGBRow_SSE2 to gcc
// 18 instructions
__declspec(naked)
void ARGB4444ToARGBRow_SSE2(const uint8* src_argb4444, uint8* dst_argb,
                            int pix) {
__asm {
    mov       eax, 0x0f0f0f0f  // generate mask 0x0f0f0f0f
    movd      xmm4, eax
    pshufd    xmm4, xmm4, 0
    movdqa    xmm5, xmm4       // 0xf0f0f0f0 for high nibbles
    pslld     xmm5, 4
    mov       eax, [esp + 4]   // src_argb4444
    mov       edx, [esp + 8]   // dst_argb
    mov       ecx, [esp + 12]  // pix
    sub       edx, eax
    sub       edx, eax

 convertloop:
    movdqa    xmm0, [eax]   // fetch 8 pixels of bgra4444
    movdqa    xmm2, xmm0
    pand      xmm0, xmm4    // mask low nibbles
    pand      xmm2, xmm5    // mask high nibbles
    movdqa    xmm1, xmm0
    movdqa    xmm3, xmm2
    psllw     xmm1, 4
    psrlw     xmm3, 4
    por       xmm0, xmm1
    por       xmm2, xmm3
    movdqa    xmm1, xmm0
    punpcklbw xmm0, xmm2
    punpckhbw xmm1, xmm2
    movdqa    [eax * 2 + edx], xmm0  // store 4 pixels of ARGB
    movdqa    [eax * 2 + edx + 16], xmm1  // store next 4 pixels of ARGB
    lea       eax, [eax + 16]
    sub       ecx, 8
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port to gcc
__declspec(naked)
void ARGBToRGB24Row_SSSE3(const uint8* src_argb, uint8* dst_rgb, int pix) {
__asm {
    mov       eax, [esp + 4]   // src_argb
    mov       edx, [esp + 8]   // dst_rgb
    mov       ecx, [esp + 12]  // pix
    movdqa    xmm5, kShuffleMaskARGBToRGB24

 convertloop:
    movdqa    xmm0, [eax]   // fetch 16 pixels of argb
    movdqa    xmm1, [eax + 16]
    movdqa    xmm2, [eax + 32]
    movdqa    xmm3, [eax + 48]
    lea       eax, [eax + 64]
    pshufb    xmm0, xmm5    // pack 16 bytes of ARGB to 12 bytes of RGB
    pshufb    xmm1, xmm5
    pshufb    xmm2, xmm5
    pshufb    xmm3, xmm5
    movdqa    xmm4, xmm1
    psllq     xmm4, 12
    por       xmm4, xmm0
    movdqa    [edx], xmm4   // first 16 bytes
    movdqa    xmm4, xmm2
    psrlq     xmm1, 4
    psllq     xmm4, 8
    por       xmm1, xmm4
    movdqa    [edx + 16], xmm1   // middle 16 bytes
    psrlq     xmm2, 8
    psllq     xmm3, 4
    por       xmm2, xmm3
    movdqa    [edx + 32], xmm2   // last 16 bytes
    lea       edx, [edx + 48]
    sub       ecx, 16
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port to gcc
__declspec(naked)
void ARGBToRAWRow_SSSE3(const uint8* src_argb, uint8* dst_rgb, int pix) {
__asm {
    mov       eax, [esp + 4]   // src_argb
    mov       edx, [esp + 8]   // dst_rgb
    mov       ecx, [esp + 12]  // pix
    movdqa    xmm5, kShuffleMaskARGBToRAW

 convertloop:
    movdqa    xmm0, [eax]   // fetch 16 pixels of argb
    movdqa    xmm1, [eax + 16]
    movdqa    xmm2, [eax + 32]
    movdqa    xmm3, [eax + 48]
    lea       eax, [eax + 64]
    pshufb    xmm0, xmm5    // pack 16 bytes of ARGB to 12 bytes of RGB
    pshufb    xmm1, xmm5
    pshufb    xmm2, xmm5
    pshufb    xmm3, xmm5
    movdqa    xmm4, xmm1
    psllq     xmm4, 12
    por       xmm4, xmm0
    movdqa    [edx], xmm4   // first 16 bytes
    movdqa    xmm4, xmm2
    psrlq     xmm1, 4
    psllq     xmm4, 8
    por       xmm1, xmm4
    movdqa    [edx + 16], xmm1   // middle 16 bytes
    psrlq     xmm2, 8
    psllq     xmm3, 4
    por       xmm2, xmm3
    movdqa    [edx + 32], xmm2   // last 16 bytes
    lea       edx, [edx + 48]
    sub       ecx, 16
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port to gcc
__declspec(naked)
void ARGBToRGB565Row_SSE2(const uint8* src_argb, uint8* dst_rgb, int pix) {
__asm {
    pcmpeqb   xmm3, xmm3       // generate mask 0x001f001f
    psrlw     xmm3, 11
    pcmpeqb   xmm4, xmm4       // generate mask 0x07e007e0
    psrlw     xmm4, 10
    psllw     xmm4, 5
    pcmpeqb   xmm5, xmm5       // generate mask 0xf800f800
    psrlw     xmm5, 11

    mov       eax, [esp + 4]   // src_argb
    mov       edx, [esp + 8]   // dst_rgb
    mov       ecx, [esp + 12]  // pix

 convertloop:
    movdqa    xmm0, [eax]   // fetch 4 pixels of argb
    lea       eax, [eax + 16]
    movdqa    xmm1, xmm0    // B
    psrlw     xmm1, 3
    pand      xmm1, xmm3
    movdqa    xmm2, xmm0    // G
    psrlw     xmm2, 5
    pand      xmm2, xmm4
    por       xmm1, xmm2
    psrlw     xmm0, 8       // R
    pand      xmm0, xmm5
    por       xmm0, xmm1
    pslld     xmm0, 16
    psrad     xmm0, 16
    packssdw  xmm0, xmm0
    movq      qword ptr [edx], xmm0  // store 4 pixels of ARGB1555
    lea       edx, [edx + 8]
    sub       ecx, 4
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port to gcc
__declspec(naked)
void ARGBToARGB1555Row_SSE2(const uint8* src_argb, uint8* dst_rgb, int pix) {
__asm {
    pcmpeqb   xmm3, xmm3       // generate mask 0x001f001f
    psrlw     xmm3, 11
    movdqa    xmm4, xmm3       // generate mask 0x03e003e0
    psllw     xmm4, 5
    movdqa    xmm5, xmm3       // generate mask 0x7c007c00
    psllw     xmm5, 10
    pcmpeqb   xmm6, xmm6       // generate mask 0x80008000
    psrlw     xmm6, 15

    mov       eax, [esp + 4]   // src_argb
    mov       edx, [esp + 8]   // dst_rgb
    mov       ecx, [esp + 12]  // pix

 convertloop:
    movdqa    xmm0, [eax]   // fetch 4 pixels of argb
    lea       eax, [eax + 16]
    movdqa    xmm1, xmm0    // B
    psrlw     xmm1, 3
    pand      xmm1, xmm3
    movdqa    xmm2, xmm0    // G
    psrlw     xmm2, 6
    pand      xmm2, xmm4
    por       xmm1, xmm2
    movdqa    xmm2, xmm0    // R
    psrlw     xmm2, 9
    pand      xmm2, xmm5
    por       xmm1, xmm2
    movdqa    xmm2, xmm0    // A
    psrlw     xmm2, 16
    pand      xmm2, xmm6
    por       xmm1, xmm2
    pslld     xmm0, 16
    psrad     xmm0, 16
    packssdw  xmm1, xmm1
    movq      qword ptr [edx], xmm1  // store 4 pixels of ARGB1555
    lea       edx, [edx + 8]
    sub       ecx, 4
    ja        convertloop
    ret
  }
}

// TODO(fbarchard): Port to gcc
__declspec(naked)
void ARGBToARGB4444Row_SSE2(const uint8* src_argb, uint8* dst_rgb, int pix) {
__asm {
    pcmpeqb   xmm4, xmm4       // generate mask 0xf000f000
    psllw     xmm4, 12
    movdqa    xmm3, xmm4       // generate mask 0x00f000f0
    psrlw     xmm3, 8

    mov       eax, [esp + 4]   // src_argb
    mov       edx, [esp + 8]   // dst_rgb
    mov       ecx, [esp + 12]  // pix

 convertloop:
    movdqa    xmm0, [eax]   // fetch 4 pixels of argb
    lea       eax, [eax + 16]
    movdqa    xmm1, xmm0
    pand      xmm0, xmm3    // low nibble
    pand      xmm1, xmm4    // high nibble
    psrl      xmm0, 4
    psrl      xmm1, 8
    por       xmm0, xmm1
    packuswb  xmm0, xmm0
    movq      qword ptr [edx], xmm0  // store 4 pixels of ARGB4444
    lea       edx, [edx + 8]
    sub       ecx, 4
    ja        convertloop
    ret
  }
}

// Convert 16 ARGB pixels (64 bytes) to 16 Y values
__declspec(naked)
void ARGBToYRow_SSSE3(const uint8* src_argb, uint8* dst_y, int pix) {
__asm {
    mov        eax, [esp + 4]   /* src_argb */
    mov        edx, [esp + 8]   /* dst_y */
    mov        ecx, [esp + 12]  /* pix */
    movdqa     xmm5, kAddY16
    movdqa     xmm4, kARGBToY

 convertloop:
    movdqa     xmm0, [eax]
    movdqa     xmm1, [eax + 16]
    movdqa     xmm2, [eax + 32]
    movdqa     xmm3, [eax + 48]
    pmaddubsw  xmm0, xmm4
    pmaddubsw  xmm1, xmm4
    pmaddubsw  xmm2, xmm4
    pmaddubsw  xmm3, xmm4
    lea        eax, [eax + 64]
    phaddw     xmm0, xmm1
    phaddw     xmm2, xmm3
    psrlw      xmm0, 7
    psrlw      xmm2, 7
    packuswb   xmm0, xmm2
    paddb      xmm0, xmm5
    movdqa     [edx], xmm0
    lea        edx, [edx + 16]
    sub        ecx, 16
    ja         convertloop
    ret
  }
}

__declspec(naked)
void BGRAToYRow_SSSE3(const uint8* src_argb, uint8* dst_y, int pix) {
__asm {
    mov        eax, [esp + 4]   /* src_argb */
    mov        edx, [esp + 8]   /* dst_y */
    mov        ecx, [esp + 12]  /* pix */
    movdqa     xmm5, kAddY16
    movdqa     xmm4, kBGRAToY

 convertloop:
    movdqa     xmm0, [eax]
    movdqa     xmm1, [eax + 16]
    movdqa     xmm2, [eax + 32]
    movdqa     xmm3, [eax + 48]
    pmaddubsw  xmm0, xmm4
    pmaddubsw  xmm1, xmm4
    pmaddubsw  xmm2, xmm4
    pmaddubsw  xmm3, xmm4
    lea        eax, [eax + 64]
    phaddw     xmm0, xmm1
    phaddw     xmm2, xmm3
    psrlw      xmm0, 7
    psrlw      xmm2, 7
    packuswb   xmm0, xmm2
    paddb      xmm0, xmm5
    movdqa     [edx], xmm0
    lea        edx, [edx + 16]
    sub        ecx, 16
    ja         convertloop
    ret
  }
}

__declspec(naked)
void ABGRToYRow_SSSE3(const uint8* src_argb, uint8* dst_y, int pix) {
__asm {
    mov        eax, [esp + 4]   /* src_argb */
    mov        edx, [esp + 8]   /* dst_y */
    mov        ecx, [esp + 12]  /* pix */
    movdqa     xmm5, kAddY16
    movdqa     xmm4, kABGRToY

 convertloop:
    movdqa     xmm0, [eax]
    movdqa     xmm1, [eax + 16]
    movdqa     xmm2, [eax + 32]
    movdqa     xmm3, [eax + 48]
    pmaddubsw  xmm0, xmm4
    pmaddubsw  xmm1, xmm4
    pmaddubsw  xmm2, xmm4
    pmaddubsw  xmm3, xmm4
    lea        eax, [eax + 64]
    phaddw     xmm0, xmm1
    phaddw     xmm2, xmm3
    psrlw      xmm0, 7
    psrlw      xmm2, 7
    packuswb   xmm0, xmm2
    paddb      xmm0, xmm5
    movdqa     [edx], xmm0
    lea        edx, [edx + 16]
    sub        ecx, 16
    ja         convertloop
    ret
  }
}

__declspec(naked)
void ARGBToUVRow_SSSE3(const uint8* src_argb0, int src_stride_argb,
                       uint8* dst_u, uint8* dst_v, int width) {
__asm {
    push       esi
    push       edi
    mov        eax, [esp + 8 + 4]   // src_argb
    mov        esi, [esp + 8 + 8]   // src_stride_argb
    mov        edx, [esp + 8 + 12]  // dst_u
    mov        edi, [esp + 8 + 16]  // dst_v
    mov        ecx, [esp + 8 + 20]  // pix
    movdqa     xmm7, kARGBToU
    movdqa     xmm6, kARGBToV
    movdqa     xmm5, kAddUV128
    sub        edi, edx             // stride from u to v

 convertloop:
    /* step 1 - subsample 16x2 argb pixels to 8x1 */
    movdqa     xmm0, [eax]
    movdqa     xmm1, [eax + 16]
    movdqa     xmm2, [eax + 32]
    movdqa     xmm3, [eax + 48]
    pavgb      xmm0, [eax + esi]
    pavgb      xmm1, [eax + esi + 16]
    pavgb      xmm2, [eax + esi + 32]
    pavgb      xmm3, [eax + esi + 48]
    lea        eax,  [eax + 64]
    movdqa     xmm4, xmm0
    shufps     xmm0, xmm1, 0x88
    shufps     xmm4, xmm1, 0xdd
    pavgb      xmm0, xmm4
    movdqa     xmm4, xmm2
    shufps     xmm2, xmm3, 0x88
    shufps     xmm4, xmm3, 0xdd
    pavgb      xmm2, xmm4

    // step 2 - convert to U and V
    // from here down is very similar to Y code except
    // instead of 16 different pixels, its 8 pixels of U and 8 of V
    movdqa     xmm1, xmm0
    movdqa     xmm3, xmm2
    pmaddubsw  xmm0, xmm7  // U
    pmaddubsw  xmm2, xmm7
    pmaddubsw  xmm1, xmm6  // V
    pmaddubsw  xmm3, xmm6
    phaddw     xmm0, xmm2
    phaddw     xmm1, xmm3
    psraw      xmm0, 8
    psraw      xmm1, 8
    packsswb   xmm0, xmm1
    paddb      xmm0, xmm5            // -> unsigned

    // step 3 - store 8 U and 8 V values
    movlps     qword ptr [edx], xmm0 // U
    movhps     qword ptr [edx + edi], xmm0 // V
    lea        edx, [edx + 8]
    sub        ecx, 16
    ja         convertloop
    pop        edi
    pop        esi
    ret
  }
}

__declspec(naked)
void BGRAToUVRow_SSSE3(const uint8* src_argb0, int src_stride_argb,
                       uint8* dst_u, uint8* dst_v, int width) {
__asm {
    push       esi
    push       edi
    mov        eax, [esp + 8 + 4]   // src_argb
    mov        esi, [esp + 8 + 8]   // src_stride_argb
    mov        edx, [esp + 8 + 12]  // dst_u
    mov        edi, [esp + 8 + 16]  // dst_v
    mov        ecx, [esp + 8 + 20]  // pix
    movdqa     xmm7, kBGRAToU
    movdqa     xmm6, kBGRAToV
    movdqa     xmm5, kAddUV128
    sub        edi, edx             // stride from u to v

 convertloop:
    /* step 1 - subsample 16x2 argb pixels to 8x1 */
    movdqa     xmm0, [eax]
    movdqa     xmm1, [eax + 16]
    movdqa     xmm2, [eax + 32]
    movdqa     xmm3, [eax + 48]
    pavgb      xmm0, [eax + esi]
    pavgb      xmm1, [eax + esi + 16]
    pavgb      xmm2, [eax + esi + 32]
    pavgb      xmm3, [eax + esi + 48]
    lea        eax,  [eax + 64]
    movdqa     xmm4, xmm0
    shufps     xmm0, xmm1, 0x88
    shufps     xmm4, xmm1, 0xdd
    pavgb      xmm0, xmm4
    movdqa     xmm4, xmm2
    shufps     xmm2, xmm3, 0x88
    shufps     xmm4, xmm3, 0xdd
    pavgb      xmm2, xmm4

    // step 2 - convert to U and V
    // from here down is very similar to Y code except
    // instead of 16 different pixels, its 8 pixels of U and 8 of V
    movdqa     xmm1, xmm0
    movdqa     xmm3, xmm2
    pmaddubsw  xmm0, xmm7  // U
    pmaddubsw  xmm2, xmm7
    pmaddubsw  xmm1, xmm6  // V
    pmaddubsw  xmm3, xmm6
    phaddw     xmm0, xmm2
    phaddw     xmm1, xmm3
    psraw      xmm0, 8
    psraw      xmm1, 8
    packsswb   xmm0, xmm1
    paddb      xmm0, xmm5            // -> unsigned

    // step 3 - store 8 U and 8 V values
    movlps     qword ptr [edx], xmm0 // U
    movhps     qword ptr [edx + edi], xmm0 // V
    lea        edx, [edx + 8]
    sub        ecx, 16
    ja         convertloop
    pop        edi
    pop        esi
    ret
  }
}

__declspec(naked)
void ABGRToUVRow_SSSE3(const uint8* src_argb0, int src_stride_argb,
                       uint8* dst_u, uint8* dst_v, int width) {
__asm {
    push       esi
    push       edi
    mov        eax, [esp + 8 + 4]   // src_argb
    mov        esi, [esp + 8 + 8]   // src_stride_argb
    mov        edx, [esp + 8 + 12]  // dst_u
    mov        edi, [esp + 8 + 16]  // dst_v
    mov        ecx, [esp + 8 + 20]  // pix
    movdqa     xmm7, kABGRToU
    movdqa     xmm6, kABGRToV
    movdqa     xmm5, kAddUV128
    sub        edi, edx             // stride from u to v

 convertloop:
    /* step 1 - subsample 16x2 argb pixels to 8x1 */
    movdqa     xmm0, [eax]
    movdqa     xmm1, [eax + 16]
    movdqa     xmm2, [eax + 32]
    movdqa     xmm3, [eax + 48]
    pavgb      xmm0, [eax + esi]
    pavgb      xmm1, [eax + esi + 16]
    pavgb      xmm2, [eax + esi + 32]
    pavgb      xmm3, [eax + esi + 48]
    lea        eax,  [eax + 64]
    movdqa     xmm4, xmm0
    shufps     xmm0, xmm1, 0x88
    shufps     xmm4, xmm1, 0xdd
    pavgb      xmm0, xmm4
    movdqa     xmm4, xmm2
    shufps     xmm2, xmm3, 0x88
    shufps     xmm4, xmm3, 0xdd
    pavgb      xmm2, xmm4

    // step 2 - convert to U and V
    // from here down is very similar to Y code except
    // instead of 16 different pixels, its 8 pixels of U and 8 of V
    movdqa     xmm1, xmm0
    movdqa     xmm3, xmm2
    pmaddubsw  xmm0, xmm7  // U
    pmaddubsw  xmm2, xmm7
    pmaddubsw  xmm1, xmm6  // V
    pmaddubsw  xmm3, xmm6
    phaddw     xmm0, xmm2
    phaddw     xmm1, xmm3
    psraw      xmm0, 8
    psraw      xmm1, 8
    packsswb   xmm0, xmm1
    paddb      xmm0, xmm5            // -> unsigned

    // step 3 - store 8 U and 8 V values
    movlps     qword ptr [edx], xmm0 // U
    movhps     qword ptr [edx + edi], xmm0 // V
    lea        edx, [edx + 8]
    sub        ecx, 16
    ja         convertloop
    pop        edi
    pop        esi
    ret
  }
}

#ifdef HAS_FASTCONVERTYUVTOARGBROW_SSSE3

#define YG 74 /* static_cast<int8>(1.164 * 64 + 0.5) */

#define UB 127 /* min(63,static_cast<int8>(2.018 * 64)) */
#define UG -25 /* static_cast<int8>(-0.391 * 64 - 0.5) */
#define UR 0

#define VB 0
#define VG -52 /* static_cast<int8>(-0.813 * 64 - 0.5) */
#define VR 102 /* static_cast<int8>(1.596 * 64 + 0.5) */

// Bias
#define BB UB * 128 + VB * 128
#define BG UG * 128 + VG * 128
#define BR UR * 128 + VR * 128

static const vec8 kUVToB = {
  UB, VB, UB, VB, UB, VB, UB, VB, UB, VB, UB, VB, UB, VB, UB, VB
};

static const vec8 kUVToR = {
  UR, VR, UR, VR, UR, VR, UR, VR, UR, VR, UR, VR, UR, VR, UR, VR
};

static const vec8 kUVToG = {
  UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG
};

static const vec16 kYToRgb = { YG, YG, YG, YG, YG, YG, YG, YG };
static const vec16 kYSub16 = { 16, 16, 16, 16, 16, 16, 16, 16 };
static const vec16 kUVBiasB = { BB, BB, BB, BB, BB, BB, BB, BB };
static const vec16 kUVBiasG = { BG, BG, BG, BG, BG, BG, BG, BG };
static const vec16 kUVBiasR = { BR, BR, BR, BR, BR, BR, BR, BR };

#define YUVTORGB __asm {                                                       \
    /* Step 1: Find 4 UV contributions to 8 R,G,B values */                    \
    __asm movd       xmm0, [esi]          /* U */                              \
    __asm movd       xmm1, [esi + edi]    /* V */                              \
    __asm lea        esi,  [esi + 4]                                           \
    __asm punpcklbw  xmm0, xmm1           /* UV */                             \
    __asm punpcklwd  xmm0, xmm0           /* UVUV (upsample) */                \
    __asm movdqa     xmm1, xmm0                                                \
    __asm movdqa     xmm2, xmm0                                                \
    __asm pmaddubsw  xmm0, kUVToB        /* scale B UV */                      \
    __asm pmaddubsw  xmm1, kUVToG        /* scale G UV */                      \
    __asm pmaddubsw  xmm2, kUVToR        /* scale R UV */                      \
    __asm psubw      xmm0, kUVBiasB      /* unbias back to signed */           \
    __asm psubw      xmm1, kUVBiasG                                            \
    __asm psubw      xmm2, kUVBiasR                                            \
    /* Step 2: Find Y contribution to 8 R,G,B values */                        \
    __asm movq       xmm3, qword ptr [eax]                                     \
    __asm lea        eax, [eax + 8]                                            \
    __asm punpcklbw  xmm3, xmm4                                                \
    __asm psubsw     xmm3, kYSub16                                             \
    __asm pmullw     xmm3, kYToRgb                                             \
    __asm paddsw     xmm0, xmm3           /* B += Y */                         \
    __asm paddsw     xmm1, xmm3           /* G += Y */                         \
    __asm paddsw     xmm2, xmm3           /* R += Y */                         \
    __asm psraw      xmm0, 6                                                   \
    __asm psraw      xmm1, 6                                                   \
    __asm psraw      xmm2, 6                                                   \
    __asm packuswb   xmm0, xmm0           /* B */                              \
    __asm packuswb   xmm1, xmm1           /* G */                              \
    __asm packuswb   xmm2, xmm2           /* R */                              \
  }

__declspec(naked)
void FastConvertYUVToARGBRow_SSSE3(const uint8* y_buf,
                                   const uint8* u_buf,
                                   const uint8* v_buf,
                                   uint8* rgb_buf,
                                   int width) {
  __asm {
    push       esi
    push       edi
    mov        eax, [esp + 8 + 4]   // Y
    mov        esi, [esp + 8 + 8]   // U
    mov        edi, [esp + 8 + 12]  // V
    mov        edx, [esp + 8 + 16]  // rgb
    mov        ecx, [esp + 8 + 20]  // width
    sub        edi, esi
    pcmpeqb    xmm5, xmm5           // generate 0xffffffff for alpha
    pxor       xmm4, xmm4

 convertloop:
    YUVTORGB

    // Step 3: Weave into ARGB
    punpcklbw  xmm0, xmm1           // BG
    punpcklbw  xmm2, xmm5           // RA
    movdqa     xmm1, xmm0
    punpcklwd  xmm0, xmm2           // BGRA first 4 pixels
    punpckhwd  xmm1, xmm2           // BGRA next 4 pixels
    movdqa     [edx], xmm0
    movdqa     [edx + 16], xmm1
    lea        edx,  [edx + 32]

    sub        ecx, 8
    ja         convertloop

    pop        edi
    pop        esi
    ret
  }
}

__declspec(naked)
void FastConvertYUVToBGRARow_SSSE3(const uint8* y_buf,
                                   const uint8* u_buf,
                                   const uint8* v_buf,
                                   uint8* rgb_buf,
                                   int width) {
  __asm {
    push       esi
    push       edi
    mov        eax, [esp + 8 + 4]   // Y
    mov        esi, [esp + 8 + 8]   // U
    mov        edi, [esp + 8 + 12]  // V
    mov        edx, [esp + 8 + 16]  // rgb
    mov        ecx, [esp + 8 + 20]  // width
    sub        edi, esi
    pxor       xmm4, xmm4

 convertloop:
    YUVTORGB

    // Step 3: Weave into BGRA
    pcmpeqb    xmm5, xmm5           // generate 0xffffffff for alpha
    punpcklbw  xmm1, xmm0           // GB
    punpcklbw  xmm5, xmm2           // AR
    movdqa     xmm0, xmm5
    punpcklwd  xmm5, xmm1           // BGRA first 4 pixels
    punpckhwd  xmm0, xmm1           // BGRA next 4 pixels
    movdqa     [edx], xmm5
    movdqa     [edx + 16], xmm0
    lea        edx,  [edx + 32]

    sub        ecx, 8
    ja         convertloop

    pop        edi
    pop        esi
    ret
  }
}

__declspec(naked)
void FastConvertYUVToABGRRow_SSSE3(const uint8* y_buf,
                                   const uint8* u_buf,
                                   const uint8* v_buf,
                                   uint8* rgb_buf,
                                   int width) {
  __asm {
    push       esi
    push       edi
    mov        eax, [esp + 8 + 4]   // Y
    mov        esi, [esp + 8 + 8]   // U
    mov        edi, [esp + 8 + 12]  // V
    mov        edx, [esp + 8 + 16]  // rgb
    mov        ecx, [esp + 8 + 20]  // width
    sub        edi, esi
    pcmpeqb    xmm5, xmm5           // generate 0xffffffff for alpha
    pxor       xmm4, xmm4

 convertloop:
    YUVTORGB

    // Step 3: Weave into ARGB
    punpcklbw  xmm2, xmm1           // RG
    punpcklbw  xmm0, xmm5           // BA
    movdqa     xmm1, xmm2
    punpcklwd  xmm2, xmm0           // RGBA first 4 pixels
    punpckhwd  xmm1, xmm0           // RGBA next 4 pixels
    movdqa     [edx], xmm2
    movdqa     [edx + 16], xmm1
    lea        edx,  [edx + 32]

    sub        ecx, 8
    ja         convertloop

    pop        edi
    pop        esi
    ret
  }
}

__declspec(naked)
void FastConvertYUV444ToARGBRow_SSSE3(const uint8* y_buf,
                                      const uint8* u_buf,
                                      const uint8* v_buf,
                                      uint8* rgb_buf,
                                      int width) {
  __asm {
    push       esi
    push       edi
    mov        eax, [esp + 8 + 4]   // Y
    mov        esi, [esp + 8 + 8]   // U
    mov        edi, [esp + 8 + 12]  // V
    mov        edx, [esp + 8 + 16]  // rgb
    mov        ecx, [esp + 8 + 20]  // width
    sub        edi, esi
    pcmpeqb    xmm5, xmm5            // generate 0xffffffff for alpha
    pxor       xmm4, xmm4

 convertloop:
    // Step 1: Find 4 UV contributions to 4 R,G,B values
    movd       xmm0, [esi]          // U
    movd       xmm1, [esi + edi]    // V
    lea        esi,  [esi + 4]
    punpcklbw  xmm0, xmm1           // UV
    movdqa     xmm1, xmm0
    movdqa     xmm2, xmm0
    pmaddubsw  xmm0, kUVToB        // scale B UV
    pmaddubsw  xmm1, kUVToG        // scale G UV
    pmaddubsw  xmm2, kUVToR        // scale R UV
    psubw      xmm0, kUVBiasB      // unbias back to signed
    psubw      xmm1, kUVBiasG
    psubw      xmm2, kUVBiasR

    // Step 2: Find Y contribution to 4 R,G,B values
    movd       xmm3, [eax]
    lea        eax, [eax + 4]
    punpcklbw  xmm3, xmm4
    psubsw     xmm3, kYSub16
    pmullw     xmm3, kYToRgb
    paddsw     xmm0, xmm3           // B += Y
    paddsw     xmm1, xmm3           // G += Y
    paddsw     xmm2, xmm3           // R += Y
    psraw      xmm0, 6
    psraw      xmm1, 6
    psraw      xmm2, 6
    packuswb   xmm0, xmm0           // B
    packuswb   xmm1, xmm1           // G
    packuswb   xmm2, xmm2           // R

    // Step 3: Weave into ARGB
    punpcklbw  xmm0, xmm1           // BG
    punpcklbw  xmm2, xmm5           // RA
    punpcklwd  xmm0, xmm2           // BGRA 4 pixels
    movdqa     [edx], xmm0
    lea        edx,  [edx + 16]

    sub        ecx, 4
    ja         convertloop

    pop        edi
    pop        esi
    ret
  }
}
#endif

#ifdef HAS_FASTCONVERTYTOARGBROW_SSE2
__declspec(naked)
void FastConvertYToARGBRow_SSE2(const uint8* y_buf,
                                uint8* rgb_buf,
                                int width) {
  __asm {
    pcmpeqb    xmm4, xmm4           // generate mask 0xff000000
    pslld      xmm4, 24
    mov        eax,0x10001000
    movd       xmm3,eax
    pshufd     xmm3,xmm3,0
    mov        eax,0x012a012a
    movd       xmm2,eax
    pshufd     xmm2,xmm2,0
    mov        eax, [esp + 4]       // Y
    mov        edx, [esp + 8]       // rgb
    mov        ecx, [esp + 12]      // width

 convertloop:
    // Step 1: Scale Y contribution to 8 G values. G = (y - 16) * 1.164
    movq       xmm0, qword ptr [eax]
    lea        eax, [eax + 8]
    punpcklbw  xmm0, xmm0           // Y.Y
    psubusw    xmm0, xmm3
    pmulhuw    xmm0, xmm2
    packuswb   xmm0, xmm0           // G

    // Step 2: Weave into ARGB
    punpcklbw  xmm0, xmm0           // GG
    movdqa     xmm1, xmm0
    punpcklwd  xmm0, xmm0           // BGRA first 4 pixels
    punpckhwd  xmm1, xmm1           // BGRA next 4 pixels
    por        xmm0, xmm4
    por        xmm1, xmm4
    movdqa     [edx], xmm0
    movdqa     [edx + 16], xmm1
    lea        edx,  [edx + 32]

    sub        ecx, 8
    ja         convertloop

    ret
  }
}
#endif
#endif

#ifdef HAS_REVERSE_ROW_SSSE3

// Shuffle table for reversing the bytes.
static const uvec8 kShuffleReverse = {
  15u, 14u, 13u, 12u, 11u, 10u, 9u, 8u, 7u, 6u, 5u, 4u, 3u, 2u, 1u, 0u
};

__declspec(naked)
void ReverseRow_SSSE3(const uint8* src, uint8* dst, int width) {
__asm {
    mov       eax, [esp + 4]   // src
    mov       edx, [esp + 8]   // dst
    mov       ecx, [esp + 12]  // width
    movdqa    xmm5, kShuffleReverse
    lea       eax, [eax + ecx - 16]
 convertloop:
    movdqa    xmm0, [eax]
    lea       eax, [eax - 16]
    pshufb    xmm0, xmm5
    movdqa    [edx], xmm0
    lea       edx, [edx + 16]
    sub       ecx, 16
    ja        convertloop
    ret
  }
}
#endif

#ifdef HAS_REVERSE_ROW_SSE2

__declspec(naked)
void ReverseRow_SSE2(const uint8* src, uint8* dst, int width) {
__asm {
    mov       eax, [esp + 4]   // src
    mov       edx, [esp + 8]   // dst
    mov       ecx, [esp + 12]  // width
    lea       eax, [eax + ecx - 16]
 convertloop:
    movdqa    xmm0, [eax]
    lea       eax, [eax - 16]
    movdqa    xmm1, xmm0        // swap bytes
    psllw     xmm0, 8
    psrlw     xmm1, 8
    por       xmm0, xmm1
    pshuflw   xmm0, xmm0, 0x1b  // swap words
    pshufhw   xmm0, xmm0, 0x1b
    pshufd    xmm0, xmm0, 0x4e  // swap qwords
    movdqa    [edx], xmm0
    lea       edx, [edx + 16]
    sub       ecx, 16
    ja        convertloop
    ret
  }
}
#endif
#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

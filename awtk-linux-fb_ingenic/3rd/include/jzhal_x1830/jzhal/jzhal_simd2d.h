#ifndef __JZHAL_SIMD2D_H__
#define __JZHAL_SIMD2D_H__

#include "jzhal_gralloc.h"

typedef struct _simd2d_info_t {
    int (*fill)(JH_MEM *dst, JH_RECT *rect, uint32_t color);
    int (*rotate)(JH_MEM* src, JH_MEM* dst, JH_RECT* src_rect, int ro_mode);
    int (*blend)(JH_MEM* src, JH_MEM* dst, JH_RECT* src_rect, JH_RECT* dst_rect, uint8_t global_alpha);
    int (*copy)(JH_MEM* src, JH_MEM* dst, JH_RECT* src_rect, int32_t x, int32_t y);
} simd2d_info_t;

extern simd2d_info_t * jzhal_simd2d_open(char *id);
extern void jzhal_simd2d_close(char *id);

#endif /* __JZHAL_SIMD2D_H__ */

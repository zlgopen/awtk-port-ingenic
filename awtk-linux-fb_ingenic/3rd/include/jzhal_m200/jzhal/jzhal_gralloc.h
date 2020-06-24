#ifndef __JZHAL_GRALLOC_H__
#define __JZHAL_GRALLOC_H__

/* JH : JUN ZHENG HAL */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fb.h>


enum jh_mem_format {
    JH_FMT_NONE = 0,
    JH_FMT_RGBA8888,
    JH_FMT_ABGR8888,
    JH_FMT_BGRA8888,
    JH_FMT_ARGB8888,
    JH_FMT_RGB565,
    JH_FMT_BGR565,
    JH_FMT_RGB888,
    JH_FMT_BGR888,
    JH_FMT_NV12 = 20,
    JH_FMT_NV21,
    JH_FMT_YUV420_TILE, // special NV12.
    JH_FMT_YV12,
    JH_FMT_I420, // = YU12
    JH_FMT_YUY2,
    JH_FMT_UYVY,
    JH_FMT_ALPHA,
    JH_FMT_JPEG,
};

typedef struct _jh_rect_t {
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
} JH_RECT;

enum jh_mem_flag {
    ISFB = 0x1,
    SURF_T_USER = 0x2,
    SURF_T_WRAP = 0x4,
    SURF_T_GPU = 0x8
};

/*
 *   ┄┄┄┄┄┄┄┄ w_align ┄┄┄┄┄┄┄
 *  ╱                        ╲
 * ┏━━━━━━━━━━━━━━━━━━━━━┓┅┅┅┅┓
 * ┃ ╲------- w --------/┃    ┇ ╲
 * ┃ ┊                   ┃    ┇ ┊
 * ┃ ┊     ┌┄┄┄┄┄┄┄┐     ┃    ┇ ┊
 * ┃ ┊     ┊       ┊     ┃    ┇ ┊
 * ┃ ┊ h   ┊       ┊     ┃    ┇ ┊
 * ┃ ┊     ┊       ┊     ┃    ┇ ┊---> w_align
 * ┃ |     └┄┄┄┄┄┄┄┘     ┃    ┇ ┊
 * ┃ ┊              ╲ ---┃----┇-┊--------- rect. (sub area of user buffer)
 * ┃ ╱                   ┃    ┇ ┊
 * ┣━━━━━━━━━━━━━━━━━━━━━┛    ┇ ┊
 * ┋                      ╲   ┇ ╱
 * ┗┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅╲┅┅┛
 *                          ╲  \---------- real buffer area. (for phycial align)
 *                           ╲------------ user buffer area. (width and height of user specified)
 *
 * */

typedef struct _jh_mem_t {
    int32_t w; // user width specified, unit: pixel.
    int32_t h; // user height specified, unit: pixel.
    uint32_t w_align;  // width align, unit: pixel. for GPU: 64.
    uint32_t h_align; // height align, unit: pixel. for GPU: 16.
    uint32_t w_align_unit;  // width align specified, unit: pixel. for GPU: 64.
    uint32_t h_align_unit; // height align specified, unit: pixel. for GPU: 16.

    uint32_t stride[3]; // unit: byte.

    enum jh_mem_format format; // buffer format, reference enum jh_mem_format.
    uint8_t planenum; //The number of separation buffer.
    uint32_t Bpp; // bytes per pixel.

    uint32_t mem_align; // real buffer aligned size, unit: byte. for IPU: 4k.
    uint8_t* virt[3]; // reseverd 3 virtual address for yuv format, for RGB only used virt[0].
    uint32_t phycial[3]; // reseverd 3 phycial address for yuv format, for RGB only used phycial[0].

    uint32_t size; // buffer size, equal w_length * h_length.

    uint32_t flags;

    union {
#define RESEVERD_SIZE 20
        uint32_t reseverd[RESEVERD_SIZE]; // Do not modified!
    };
} JH_MEM;

#define JHMEM_LOAD(mem) \
    JH_MEM mem_bak_##mem; \
    memcpy((void *)&mem_bak_##mem, (void *)mem, sizeof(JH_MEM) - sizeof(uint32_t) * RESEVERD_SIZE);

#define JHMEM_STORE(mem) \
    memcpy((void *)mem, (void *)&mem_bak_##mem, sizeof(JH_MEM) - sizeof(uint32_t) * RESEVERD_SIZE);

typedef struct _gralloc_info_t {
    /* void * (*graphic_alloc)(int align, int size); */
    /* void (*graphic_free)(void *mem); */
    /* JH_MEM * (*new_jh_mem)(void); */
    /* void (*free_jh_mem)(JH_MEM *mem); */
    /* void (*unwrap_mem)(JH_MEM *gpu_mem); */

    JH_MEM * (*wrap_mem)(int w, int h, enum jh_mem_format format, uint8_t* vir, uint32_t size);
    JH_MEM * (*alloc_mem)(int w, int h, enum jh_mem_format format);
    JH_MEM * (*alloc_mem1)(uint32_t size);
    JH_MEM * (*alloc_gpu_mem)(int w, int h, enum jh_mem_format format);
    /* JH_MEM * (*wrap_framebuffer)(int index); */

    int (*reset_mem)(JH_MEM *mem, int w, int h, enum jh_mem_format format);

    void (*free_mem)(JH_MEM *mem);
} gralloc_info_t;

extern void dump_jhmem(JH_MEM* mem, char *str);

#define jzhal_gralloc_open(id) \
    jzhal_gralloc__open(id, JZHAL_VERSION)
extern gralloc_info_t * jzhal_gralloc__open(char *id, char *version);
extern void jzhal_gralloc_close(char *id);

// debug
extern int save_buffer(JH_MEM* mem, char *str);
extern void clear_buffer(JH_MEM * buf);
extern void fill_v_colorbar(JH_MEM * buf, JH_RECT *rect);
extern void fill_h_colorbar(JH_MEM * buf, JH_RECT *rect);

#ifdef __cplusplus
}
#endif

#endif /* __JZHAL_GRALLOC_H__ */

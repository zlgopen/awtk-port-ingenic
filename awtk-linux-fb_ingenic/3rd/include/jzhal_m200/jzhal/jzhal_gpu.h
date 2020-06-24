#ifndef __JZHAL_GPU_H__
#define __JZHAL_GPU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "jzhal_gralloc.h"

enum jh_shaders {
    // 畸变矫正
    DC_NV12_to_NV12,
    DC_NV12_to_I420,
    DC_RGB565_to_RGB8888,

    SHADERS_END,
};

typedef enum _GPU_BLEND_MODE {
    GPU_BLEND_COPY,
    GPU_BLEND_ALPHA
} GPU_BLEND_MODE;

typedef struct _gpu_info_t {

    JH_MEM * (*alloc_video_buffer)(int w, int h, enum jh_mem_format format);
    /* void (*free_buffer)(JH_MEM *buf); */
    /* JH_MEM * (*wrap_buffer)(int w, int h, int format, uint8_t *vir, int phy); */
    /* int (*unwrap_buffer)(JH_MEM *gpu_buf); */
    /* JH_MEM * (*wrap_framebuffer)(int index); */

    /* GPU 2D 加速接口 */
    struct {
        /*
         * buffer 填充
         *
         * dst: 指定buffer的JH_MEM结构
         * rect: 指定填充的范围
         * color: 指定填充颜色
         *
         * */
        int (*fill)(JH_MEM *dst, JH_RECT *rect, uint32_t color);

        /*
         * buffer 旋转
         *
         * src: 指定源buffer的JH_MEM结构
         * srcrect: 指定源buffer待旋转的区域
         * dst: 指定目标buffer的JH_MEM结构
         * ro_mode: 旋转角度: 0, 90, 180, 270
         *
         * */
        int (*rotate)(JH_MEM *src, JH_RECT *srcrect, JH_MEM *dst, int ro_mode);

        /*
         * buffer 混合
         *
         * src: 指定源buffer的JH_MEM结构
         * srcrect: 指定源buffer待混合的区域
         * dst: 指定目标buffer的JH_MEM结构
         * dstrect: 指定目标buffer待混合的区域
         * global_alpha: 以全局alpha混合时，指定alpha值
         * blend_mode: 指定源到目标的混合方式
         *
         * */
        int (*bitblt)(JH_MEM *src, JH_RECT *srcrect, JH_MEM *dst, JH_RECT *dstrect, uint8_t global_alpha, GPU_BLEND_MODE blend_mode);

        /*
         * buffer 拷贝
         *
         * src: 指定源buffer的JH_MEM结构
         * dst: 指定目标buffer的JH_MEM结构
         * rect: 指定源buffer待拷贝的区域
         *
         * */
        int (*fbmerge)(JH_MEM *src, JH_MEM *dst, JH_RECT *rect);

        /*
         * buffer 拷贝, 性能低于CPU大约50%.
         * 注意：此功能对buffer地址有要求，要求两个buffer必须页内地址偏移一致。
         *
         * src: 指定源buffer的地址
         * dst: 指定目标buffer的地址
         * size: 指定源buffer待拷贝的大小, 单位字节。
         *
         * */
        int (*memcpy)(void *src, void *dst, int size);
        /* int (*csc)(JH_MEM *src, JH_MEM *dst); */

    } gpu2d_function;

    /* 封装部分GPU 3D 接口, 亦可以使用原生GLES接口 */
    struct  {
        int (*shaderInit)(const char *vert, const char *frag);
        void (*shaderDeinit)(int index);
        int (*getAttribLocation)(char *str);
        int (*useProgram)(int index);
        void (*setUniform1i)(int index, const char *name, int val);
        void (*setUniform3f)(int index, const char *name, float v0, float v1, float v2);
        /* void (*setUniformTexture)(int index, const char *name, uint32_t texNum, JH_MEM* buf); */
        uint32_t (*bindTexture)(JH_MEM *buf);
        uint32_t (*getProgramHandle)(int shader_index);
        int (*makeCurrent)(void);
#if 0
        int (*wrapTextureEXT)(JH_MEM *buf);
        JH_MEM * (*wrapTexture)(int w, int h, int format, uint8_t *vir, int phy);
        int (*unWrapTexture)(JH_MEM *buf);
        JH_MEM * (*newTexture)(int w, int h, int format);
        void (*freeTexture)(JH_MEM *buf);

        int (*wrapFBOEXT)(JH_MEM *buf);
        JH_MEM * (*wrapFBO)(int w, int h, int format, uint8_t *vir, int phy);
        int (*unWrapFBO)(JH_MEM *buf);
        JH_MEM * (*newFBO)(int w, int h, int format);
        void (*freeFBO)(JH_MEM *buf);
        JH_MEM * (*wrapFramebuffer)(int index);
#endif

        /*
         * 设置GL指定输出
         *
         * buf: 指定buffer的JH_MEM结构
         *
         * */
        uint32_t (*setFBO)(JH_MEM *buf);

        // plugins
        uint32_t (*gpu_dc)(JH_MEM *src, JH_MEM *dst);
        uint32_t (*gpu_cc)(JH_MEM *src, JH_MEM *dst);

    } gpu3d_function;
}gpu_info_t;

extern gpu_info_t * jzhal_gpu2d_open(char *id);
extern void jzhal_gpu2d_close(char *id);

extern gpu_info_t * jzhal_gpu3d_open(char *id);
extern void jzhal_gpu3d_close(char *id);

#ifdef __cplusplus
}
#endif

#endif /* __JZHAL_GPU_H__ */

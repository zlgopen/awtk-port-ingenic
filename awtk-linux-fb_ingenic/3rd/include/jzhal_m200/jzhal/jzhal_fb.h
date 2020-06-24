#ifndef __JZHAL_FB_H__
#define __JZHAL_FB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fb.h>
#include "jzhal_gralloc.h"

/* 12种OSD混合模式，参考Android porterduff */
enum osd_blend_mode {
    OSD_CLEAR,
    OSD_SRC,
    OSD_DST,
    OSD_SRC_OVER,
    OSD_DST_OVER,
    OSD_SRC_IN,
    OSD_DST_IN,
    OSD_SRC_OUT,
    OSD_DST_OUT,
    OSD_SRC_ATOP,
    OSD_DST_ATOP,
    OSD_XOR
};

enum alpha_mode {
    GLOBAL_ALPHA, // use global alpha.
    PIXEL_ALPHA,  // use pixel alpha.
    DISALBE_ALPHA // ignore alpha bit.
};

enum colorkey_mode {
    COLORKEY,        // 指定的颜色不会显示
    MASK_COLORKEY,   // 指定的颜色将会显示
    DISALBE_COLORKEY // disable colorkey.
};

typedef struct _fb_info_t {
    int fd;                       // fb设备句柄
    void* bits;                   // mmap首地址
    struct fb_fix_screeninfo fix; // 参考标准framebuffer结构
    struct fb_var_screeninfo var; // 参考标准framebuffer结构
    uint32_t framesize;           // 每个framebuffer的大小, 单位字节.

    uint32_t *buf;                // framebuffer虚拟地址数组指针, 访问方法: buf[0], buf[1]...
    uint32_t *buf_phy;            // framebuffer物理地址数组指针, 访问方法同buf.
    uint32_t buf_num;             // 内核提供的buf总数量

    /*
     * 设置OSD混合模式
     *
     * mode: 指定混合模式
     *
     * */
    void (*set_blend_mode)(enum osd_blend_mode mode);

    /*
     * 设置指定层的alpha方法
     *
     * fg_index: 层索引, 起始为0
     * mode: 指定alpha模式
     * global_alpha: 使用全局alpha时, 指定的alpha值
     *
     * */
    int (*set_alpha)(int fg_index, enum alpha_mode mode, int global_alpha);

    /*
     * 设置指定层相对于屏幕的大小
     * 修改此值后，需要重新enable_fg.
     *
     * fg_index: 层索引, 起始为0
     * w: 宽度
     * h: 高度
     *
     * */
    int (*set_fg_size)(int fg_index, int w, int h);

    /*
     * 设置指定层相对于屏幕的位置
     * 修改此值后，需要重新enable_fg.
     *
     * fg_index: 层索引, 起始为0
     * x: 左上角坐标x
     * y: 左上角坐标y
     *
     * */
    int (*set_fg_pos)(int fg_index, int x, int y);

    /*
     * 设置指定层的色键方法
     *
     * fg_index: 层索引, 起始为0
     * mode: 色键方法
     * color: 指定色键颜色
     *
     * */
    int (*set_colorkey)(int fg_index, enum colorkey_mode mode, uint32_t color);

    /*
     * 执行同步, 直到同步完成, 需要开启硬件同步
     *
     * */
    int (*fb_sync)(void);

    /*
     * 使能指定层
     *
     * fg_index: 层索引, 起始为0
     *
     * */
    int (*enable_fg)(int fg_index);

    /*
     * 禁用指定层
     *
     * fg_index: 层索引, 起始为0
     *
     * */
    int (*disable_fg)(int fg_index);

    /*
     * 切换指定层 使用指定framebuffer
     *
     * fg_index: 层索引, 起始为0
     * fb_index:
     *
     * */
    int (*pan_display)(int fg_index, int fb_index);

    /*
     * 设置指定层的默认背景填充色
     *
     * bg_index: 层索引, 起始为0
     * color: 指定背景色
     *
     * */
    int (*set_background_color)(int bg_index, uint32_t color);

    /*
     * 封装指定framebuffer到JH_MEM结构
     *
     * index: framebuffer索引, 起始为0
     *
     * */
    JH_MEM * (*wrap_framebuffer)(uint32_t index);

    /*
     * 设置硬件同步，以防止屏幕撕裂
     *
     * en: 使能: 1, 禁用: 0
     *
     * */
    int (*enable_vsync)(int en);
} fb_info_t;

extern fb_info_t * jzhal_fb_open(char *id);
extern void jzhal_fb_close(char *id);

#ifdef __cplusplus
}
#endif

#endif /* __JZHAL_FB_H__ */

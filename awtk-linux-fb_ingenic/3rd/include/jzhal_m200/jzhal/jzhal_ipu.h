#ifndef __JZHAL_IPU_H__
#define __JZHAL_IPU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "jzhal_gralloc.h"

/*
 * IPU图像缩放及格式转换
 * src: 指定源的JH_MEM
 * dst: 指定目标的JH_MEM
 * dst_rect: 指定转换到目标的位置及大小
 *
 * */
extern int jzhal_ipu_open(char *id);
extern void jzhal_ipu_close(char *id);
extern int ipu_csc_resize(JH_MEM *src, JH_MEM *dst, JH_RECT *dst_rect);
extern int ipu_memcopy(void *dst, void *src, int size);

#ifdef __cplusplus
}
#endif

#endif /* __JZHAL_IPU_H__ */

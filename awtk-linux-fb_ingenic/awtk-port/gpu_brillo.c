
int jhmem_alloc_gpu_buffer(bitmap_t *bitmap_info)
{
    if (bitmap_info->buffer == NULL) {
        bitmap_info->buffer = calloc(1, sizeof(graphic_buffer_jzgpu_t));
    }
    JH_MEM *data = gralloc->alloc_gpu_mem(bitmap_info->w, bitmap_info->h, convert(bitmap_info->format));
    _JHMEM_PTR(bitmap_info) = data;
    GRAPHIC_BUFFER_JZGPU(bitmap_info->buffer)->data = data->virt[0];
    bitmap_info->line_length = data->stride[0];
    /* jhmem_check_and_fix_gpu2d(data); */

    return 0;
}


ret_t g2d_fill_rect(bitmap_t* fb, rect_t* dst, color_t c)
{
    linux_fb_pwdata_deal(fb,NULL);

    if (dst->h == 1)
        return RET_NOT_IMPL;

    color_t c_gpu = c;
    if(fb->format == BITMAP_FMT_BGRA8888){
        rgba_t bgra = {c.rgba.b, c.rgba.g, c.rgba.r, c.rgba.a};
        c_gpu.rgba = bgra;
    }
    if(jzhal_fill(JHMEM_PTR(fb),(JH_RECT*)dst, c_gpu.color, TRUE) == 0){
        return RET_OK;
    }else{
        return RET_NOT_IMPL;
    }
}

ret_t g2d_copy_image(bitmap_t* fb, bitmap_t* img, rect_t* src, xy_t x, xy_t y)
{
    linux_fb_pwdata_deal(fb,img);
    JH_RECT dst = {x,y,src->w,src->h};
    if(jzhal_bitblt(JHMEM_PTR(img), (JH_RECT*)src, JHMEM_PTR(fb), &dst, 0xff, GPU_BLEND_COPY,TRUE) == 0){
        return RET_OK;
    }else{
        return RET_NOT_IMPL;
    }
}

ret_t g2d_blend_image(bitmap_t* fb, bitmap_t* img, rect_t* dst, rect_t* src, uint8_t global_alpha)
{
    linux_fb_pwdata_deal(fb,img);
    JH_RECT dst1 = {dst->x,dst->y,dst->w,dst->h};
    if(jzhal_bitblt(JHMEM_PTR(img), (JH_RECT*)src, JHMEM_PTR(fb), &dst1, global_alpha, GPU_BLEND_ALPHA,TRUE) == 0){
        return RET_OK;
    }else{
        return RET_NOT_IMPL;
    }
}

ret_t g2d_rotate_image(bitmap_t* fb, bitmap_t* img, rect_t* src, lcd_orientation_t o)
{
    linux_fb_pwdata_deal(fb,img);
    if(o == LCD_ORIENTATION_90)
        o = LCD_ORIENTATION_270;
    else if(o == LCD_ORIENTATION_270)
        o = LCD_ORIENTATION_90;

    if(jzhal_rotate(JHMEM_PTR(img), (JH_RECT*)src, JHMEM_PTR(fb), o,TRUE) == 0){
        return RET_OK;
    }else{
        return RET_NOT_IMPL;
    }
}

int gpu_fb_merge(bitmap_t *src_img, bitmap_t *dst_img, rect_t *rect)
{
    if(jzhal_fbmerge(JHMEM_PTR(src_img), JHMEM_PTR(dst_img), (JH_RECT*)rect,TRUE) == 0){
        return 0;
    }else{
        return -1;
    }
}

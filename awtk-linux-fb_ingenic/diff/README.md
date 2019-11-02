# awtk 支持 linux-fb 模式下启用 opengles
#君正为加速m200开发板提供的GPU加速方案中，需要修改awtk的部分
- ### 合并 diff

  1. 把 diff 合并到 awtk 中，在 awtk 目录下输入命令：

     ```
     git apply awtk适配ingenic_GPU.diff
     ```


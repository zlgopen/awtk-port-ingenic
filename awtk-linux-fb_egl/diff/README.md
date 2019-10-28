# awtk 支持 linux-fb 模式下启用 opengles

- ### 合并 diff

  1. 把 diff 合并到 awtk 中，在 awtk 目录下输入命令：

     ```
     git apply awtk增加linux-fb.diff
     ```

  2. 把 native_window_fb_gl.c 和 native_window_fb_gl.h 文件放到 awtk\src\native_window 目录下。

- ### 适配平台的 opengles

  1. 修改 awtk-linux-fb\awtk-port\main_loop_linux.c 文件中 Init_GLES 函数来启用 opengles，修改 DeInit_GLES 函数来关闭 opengles。

  2. 修改 awtk-linux-fb\awtk-port\main_loop_linux.c 文件中 gles_swap_buffer 和 gles_make_current 函数来兼容不同的板子的方法。

     1. gles_swap_buffer 函数是画完所有东西后调用的，用来交换两个贴图的。
     2. gles_make_current 函数是在绘图之前调用的，用来给画图设置 opengles 的摄像机等参数或者 opengles 的 MakeCurrent 等函数。

  3. 修改 awtk-linux-fb\awtk_config.py中的链接 opengles 类库，例如下面中的 “GLESv2” 和 “EGL”。

     ```
     OS_LIBS = OS_LIBS + ['stdc++', 'pthread', 'm', 'dl', "GLESv2", "EGL"]
     ```

  备注：由于这个本项目是支持周立功的 XMI6 板子开启opengles （系统为嵌入式 linux），所以其他的板子可能需要适当修改。
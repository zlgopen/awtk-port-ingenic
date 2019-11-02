import os
import platform

OS_NAME = platform.system()

def joinPath(root, subdir):
  return os.path.normpath(os.path.join(root, subdir))

CWD=os.path.normpath(os.path.abspath(os.path.dirname(__file__)));

TK_LINUX_FB_ROOT = CWD
TK_ROOT          = joinPath(TK_LINUX_FB_ROOT, '../awtk')
TK_SRC           = joinPath(TK_ROOT, 'src')
TK_3RD_ROOT      = joinPath(TK_ROOT, '3rd')
GTEST_ROOT       = joinPath(TK_ROOT, '3rd/gtest/googletest')

BUILD_DIR        = joinPath(TK_LINUX_FB_ROOT, 'build')
BIN_DIR          = joinPath(BUILD_DIR, 'bin')
LIB_DIR          = joinPath(BUILD_DIR, 'lib')
VAR_DIR          = joinPath(BUILD_DIR, 'var')
TK_DEMO_ROOT     = joinPath(TK_ROOT, 'demos')

LCD='LINUX_FB'
INPUT_ENGINE='pinyin'
#NANOVG_BACKEND='AGGE'
NANOVG_BACKEND='GLES2'
#BOARD_PLATFORM= 'x1830'
BOARD_PLATFORM= ''

COMMON_CCFLAGS=' -DHAS_STD_MALLOC -DWITH_FS_RES -DHAS_STDIO -DWITH_VGCANVAS -DWITH_UNICODE_BREAK '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DSTBTT_STATIC -DSTB_IMAGE_STATIC -DWITH_STB_IMAGE -DWITH_STB_FONT '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_WIDGET_TYPE_CHECK'


#only for c compiler flags
COMMON_CFLAGS=''
COMMON_CFLAGS=COMMON_CFLAGS+' -std=gnu99 '

OS_LIBS=[]
OS_LIBPATH=joinPath(TK_LINUX_FB_ROOT, 'awtk-port-libs')
OS_CPPPATH=[]
OS_LINKFLAGS=''
OS_SUBSYSTEM_CONSOLE=''
OS_SUBSYSTEM_WINDOWS=''
OS_FLAGS='-g -Wall -Os '
#OS_FLAGS='-g -Wall -Os -mfloat-abi=hard '

#for build tslib
TSLIB_INC_DIR=[joinPath(TK_LINUX_FB_ROOT, '3rd/include'), joinPath(TK_LINUX_FB_ROOT, '3rd/include/HAL')]
TSLIB_LIB_DIR=joinPath(TK_LINUX_FB_ROOT, '3rd/libs')
TOOLS_PREFIX=os.getenv("COMPILER_PATH")+'/bin/mips-linux-gnu-'

#for prebuild tslib
#TSLIB_LIB_DIR='/opt/28x/tslib/lib'
#TSLIB_INC_DIR='/opt/28x/tslib/include'
#TOOLS_PREFIX='/opt/poky/1.7/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-'

#for pc build
#TOOLS_PREFIX=''
#TSLIB_LIB_DIR=''
#OS_FLAGS='-g -Wall '

OS_LIBS = OS_LIBS + ['stdc++', 'pthread', 'm', 'dl','rt']
COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DLINUX -DHAS_PTHREAD -DWITH_BITMAP_BGRA'
# -DENABLE_CURSOR

if TSLIB_LIB_DIR != '':
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DHAS_TSLIB -fPIC'

if NANOVG_BACKEND == 'AGGE':
    NANOVG_BACKEND_LIBS=['nanovg-agge', 'nanovg', 'agge'];
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_NANOVG_AGGE '
    if BOARD_PLATFORM == 'x1830':
        COMMON_CCFLAGS = COMMON_CCFLAGS + '-DX1830 -mmxu2'
        LIBS=['awtk', 'extwidgets', 'awtk_linux_fb_x1830', 'widgets', 'base', 'gpinyin', 'tkc', 'linebreak'];
    else :
        LIBS=['awtk', 'extwidgets', 'awtk_linux_fb_cpu', 'widgets', 'base', 'gpinyin', 'tkc', 'linebreak'];

    GRAPHIC_BUFFER="default"
elif NANOVG_BACKEND == 'GLES2':
    NANOVG_BACKEND_LIBS=['nanovg','glad',\
     'galUtil','GAL','EGL', 'VIVANTE', 'VDK', 'GLSLC', 'VSC', 'GLESv2'];
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_NANOVG_GLES2 -DWITH_NANOVG_GL -DWITH_G2D' \
            + ' -DWITH_NANOVG_GPU -DWITH_VGCANVAS_LCD -DWITH_JZGPU'
    GRAPHIC_BUFFER="jzgpu"
    LIBS=['awtk', 'extwidgets', 'awtk_linux_fb_gpu', 'widgets', 'base', 'gpinyin', 'tkc', 'linebreak'];

LIBS=LIBS + OS_LIBS + NANOVG_BACKEND_LIBS

CFLAGS=COMMON_CFLAGS
LINKFLAGS=OS_LINKFLAGS;
LIBPATH=[LIB_DIR] + [OS_LIBPATH]
CCFLAGS=OS_FLAGS + COMMON_CCFLAGS

CPPPATH=[TK_ROOT,
  TK_SRC,
  TK_3RD_ROOT,
  joinPath(TK_SRC, 'ext_widgets'),
  joinPath(TK_ROOT, 'tools'),
  joinPath(TK_3RD_ROOT, 'agge'),
  joinPath(TK_3RD_ROOT, 'agg/include'),
  joinPath(TK_3RD_ROOT, 'nanovg'),
  joinPath(TK_3RD_ROOT, 'nanovg/base'),
  joinPath(TK_3RD_ROOT, 'nanovg/gl'),
  joinPath(TK_3RD_ROOT, 'libunibreak'),
  joinPath(TK_3RD_ROOT, 'gpinyin/include'),
  ] + OS_CPPPATH

if TSLIB_LIB_DIR != '':
  LIBS = ['ts'] + LIBS
  LIBPATH = [TSLIB_LIB_DIR] + LIBPATH;
  CPPPATH = [TSLIB_INC_DIR] + CPPPATH;

os.environ['LCD'] = LCD
os.environ['TARGET_ARCH'] = ''
os.environ['BIN_DIR'] = BIN_DIR;
os.environ['LIB_DIR'] = LIB_DIR;
os.environ['TK_ROOT'] = TK_ROOT;
os.environ['CCFLAGS'] = CCFLAGS;
os.environ['VGCANVAS'] = 'NANOVG'
os.environ['INPUT_ENGINE'] = INPUT_ENGINE;
os.environ['TSLIB_LIB_DIR'] = TSLIB_LIB_DIR;
os.environ['NANOVG_BACKEND'] = NANOVG_BACKEND;
os.environ['TK_3RD_ROOT'] = TK_3RD_ROOT;
os.environ['GTEST_ROOT'] = GTEST_ROOT;
os.environ['NATIVE_WINDOW'] = '';
os.environ['GRAPHIC_BUFFER'] = GRAPHIC_BUFFER;
os.environ['BOARD_PLATFORM'] = BOARD_PLATFORM;

CC=TOOLS_PREFIX+'gcc',
CXX=TOOLS_PREFIX+'g++',
LD=TOOLS_PREFIX+'g++',
AR=TOOLS_PREFIX+'ar',
STRIP=TOOLS_PREFIX+'strip',


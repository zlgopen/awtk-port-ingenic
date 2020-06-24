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

#INPUT_ENGINE='null'
#INPUT_ENGINE='spinyin'
#INPUT_ENGINE='t9'
#INPUT_ENGINE='t9ext'
INPUT_ENGINE='pinyin'

IFACCEL=os.getenv("AWTK_ACCEL")

if IFACCEL == 'M200_GPU3D':
    NANOVG_BACKEND='GLES2'
else:
    NANOVG_BACKEND='AGGE'

COMMON_CCFLAGS=' -DHAS_STD_MALLOC -DWITH_UNICODE_BREAK -DHAS_STDIO -DHAS_FAST_MEMCPY -DWITH_VGCANVAS'
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_ASSET_LOADER -DWITH_FS_RES -D_FILE_OFFSET_BITS=64'

COMMON_CCFLAGS=COMMON_CCFLAGS+' -DSTBTT_STATIC -DSTB_IMAGE_STATIC -DWITH_STB_IMAGE -DWITH_STB_FONT '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_WIDGET_TYPE_CHECK'

if INPUT_ENGINE == 't9':
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_T9 '
elif INPUT_ENGINE == 't9ext' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_T9EXT'
elif INPUT_ENGINE == 'pinyin' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_PINYIN '
elif INPUT_ENGINE == 'spinyin' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_SPINYIN '
elif INPUT_ENGINE == 'null' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_NULL '

OS_LIBS=[]
OS_LIBPATH=[]
OS_CPPPATH=[]
OS_LINKFLAGS='-Wl,-rpath-link,3rd/libs'
OS_SUBSYSTEM_CONSOLE=''
OS_SUBSYSTEM_WINDOWS=''
OS_FLAGS='-Wall -Os'
#OS_FLAGS='-g -Wall -Os -mfloat-abi=hard '

#for build tslib
TSLIB_INC_DIR=[joinPath(TK_LINUX_FB_ROOT, '3rd/include')]
TSLIB_LIB_DIR=joinPath(TK_LINUX_FB_ROOT, '3rd/libs')
TOOLS_PREFIX=os.getenv("COMPILER_PATH")+'/bin/mips-linux-gnu-'

#-DHAS_STDIO ##if print log
LIBS =['awtk_global', 'extwidgets', 'widgets', 'awtk_linux_fb', 'base', 'awtk_linux_fb', 'gpinyin', 'streams', 'conf_io', 'compressors', 'miniz', 'ubjson', 'tkc', 'linebreak']
OS_LIBS = ['stdc++', 'pthread', 'rt', 'm', 'dl']
BOARD_PLATFORM=''
if NANOVG_BACKEND == 'AGGE':
    AWTK_DLL_DEPS_LIBS=['nanovg-agge', 'nanovg', 'agge'];
    NANOVG_BACKEND_PROJS=['3rd/agge/SConscript'];
    GRAPHIC_BUFFER="jzgpu"
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_NANOVG_AGGE'
    if IFACCEL == 'M200_GPU2D':
        COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_G2D -DWITH_JZGPU -DACCEL_M200_GPU2D'
        TSLIB_INC_DIR=TSLIB_INC_DIR+[joinPath(TK_LINUX_FB_ROOT, '3rd/include/jzhal_m200')]
        OS_LIBS = OS_LIBS + ['jzhal_m200']
    elif IFACCEL == 'X1830_MXU':
        BOARD_PLATFORM='x1830'
        COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_G2D -DWITH_JZGPU -DWITH_BITMAP_BGRA -DTK_MAX_FPS=30 -DACCEL_X1830_MXU -mmxu2'
        TSLIB_INC_DIR=[TSLIB_INC_DIR]+[joinPath(TK_LINUX_FB_ROOT, '3rd/include/jzhal_x1830')]
        OS_LIBS = OS_LIBS + ['jzhal_x1830']
    else:
        COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_BITMAP_BGRA'
        GRAPHIC_BUFFER="default"
        TSLIB_INC_DIR=[TSLIB_INC_DIR]+[joinPath(TK_LINUX_FB_ROOT, '3rd/include/jzhal_m200')]
        OS_LIBS = OS_LIBS + ['jzhal_m200']
    LIBS=LIBS + OS_LIBS + AWTK_DLL_DEPS_LIBS
elif NANOVG_BACKEND == 'GLES2':
    AWTK_DLL_DEPS_LIBS=['nanovg','glad'];
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_NANOVG_GLES2 -DWITH_NANOVG_GL -DWITH_G2D' \
            + ' -DWITH_NANOVG_GPU -DWITH_VGCANVAS_LCD -DWITH_JZGPU -DWITH_SCREEN_GL_FLIP_VERTICAL -DWITH_SCREEN_GL_BGRA -DACCEL_M200_GPU3D'
    LIBS=LIBS + OS_LIBS + AWTK_DLL_DEPS_LIBS
    GRAPHIC_BUFFER="jzgpu"
    TSLIB_INC_DIR=[TSLIB_INC_DIR]+[joinPath(TK_LINUX_FB_ROOT, '3rd/include/jzhal_m200')]
    OS_LIBS = OS_LIBS + ['jzhal_m200']
#-DENABLE_CURSOR '

CC=TOOLS_PREFIX+'gcc',
CXX=TOOLS_PREFIX+'g++',
LD=TOOLS_PREFIX+'ld',
AR=TOOLS_PREFIX+'ar',
STRIP=TOOLS_PREFIX+'strip',

COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DLINUX -DHAS_PTHREAD -DENABLE_CURSOR -fPIC '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_DATA_READER_WRITER=1 '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_EVENT_RECORDER_PLAYER=1 '
COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_WIDGET_TYPE_CHECK=1 '

if TSLIB_LIB_DIR != '':
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DHAS_TSLIB '

COMMON_CFLAGS='-std=gnu99 '
CFLAGS=COMMON_CFLAGS
LINKFLAGS=OS_LINKFLAGS;
LIBPATH=[LIB_DIR, BIN_DIR] + OS_LIBPATH
CCFLAGS=OS_FLAGS + COMMON_CCFLAGS


AWTK_DLL_DEPS_LIBS = AWTK_DLL_DEPS_LIBS + OS_LIBS

OS_WHOLE_ARCHIVE =' -Wl,--whole-archive -lawtk_global -lextwidgets -lwidgets -lawtk_linux_fb -lbase -lgpinyin -ltkc -lstreams -lconf_io -lubjson -lcompressors -lminiz -llinebreak -Wl,--no-whole-archive'

STATIC_LIBS=LIBS

CPPPATH=[TK_ROOT,
  TK_SRC,
   TK_3RD_ROOT,
  joinPath(TK_SRC, 'ext_widgets'),
  joinPath(TK_3RD_ROOT, 'pixman'),
  joinPath(TK_3RD_ROOT, 'agge'),
  joinPath(TK_3RD_ROOT, 'agg/include'),
  joinPath(TK_3RD_ROOT, 'nanovg'),
  joinPath(TK_3RD_ROOT, 'nanovg/gl'),
  joinPath(TK_3RD_ROOT, 'nanovg/base'),
  joinPath(TK_3RD_ROOT, 'nanovg/agge'),
  joinPath(TK_3RD_ROOT, 'agge/src'),
  joinPath(TK_3RD_ROOT, 'agge/include'),
  joinPath(TK_3RD_ROOT, 'gpinyin/include'),
  joinPath(TK_3RD_ROOT, 'libunibreak'),
  joinPath(TK_3RD_ROOT, 'gpinyin/include'),
  joinPath(TK_3RD_ROOT, 'gtest/googletest'),
  joinPath(TK_3RD_ROOT, 'gtest/googletest/include'),
  ] + OS_CPPPATH

if TSLIB_LIB_DIR != '':
  SHARED_LIBS=['awtk', 'ts'] + OS_LIBS;
  LIBS = ['ts'] + LIBS
  LIBPATH = [TSLIB_LIB_DIR] + LIBPATH;
  CPPPATH = [TSLIB_INC_DIR] + CPPPATH;
else:
  SHARED_LIBS=['awtk'] + OS_LIBS;


os.environ['LCD'] = LCD
os.environ['TARGET_ARCH'] = 'arm'
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
os.environ['BOARD_PLATFORM'] = BOARD_PLATFORM;
os.environ['NATIVE_WINDOW'] = 'raw';
os.environ['GRAPHIC_BUFFER'] = GRAPHIC_BUFFER;

os.environ['OS_WHOLE_ARCHIVE'] = OS_WHOLE_ARCHIVE;
os.environ['AWTK_DLL_DEPS_LIBS'] = ';'.join(AWTK_DLL_DEPS_LIBS)
os.environ['STATIC_LIBS'] = ';'.join(STATIC_LIBS)

def has_custom_cc():
    return True


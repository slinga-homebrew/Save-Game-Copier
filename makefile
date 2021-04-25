JO_COMPILE_WITH_VIDEO_MODULE = 0
JO_COMPILE_WITH_BACKUP_MODULE = 1
JO_COMPILE_WITH_TGA_MODULE = 0
JO_COMPILE_WITH_AUDIO_MODULE = 1
JO_COMPILE_WITH_3D_MODULE = 0
JO_COMPILE_WITH_PSEUDO_MODE7_MODULE = 0
JO_COMPILE_WITH_EFFECTS_MODULE = 0
JO_PSEUDO_SATURN_KAI_SUPPORT = 1
JO_COMPILE_WITH_DUAL_CPU_MODULE = 0
JO_DEBUG = 0
JO_NTSC = 1
JO_COMPILE_USING_SGL = 1
SRCS=main.c bup_header.c util.c backends/backend.c backends/saturn.c backends/satiator.c backends/cd.c backends/actionreplay.c backends/sat.c md5/md5.c backends/satiator/satiator.c backends/satiator/cd.c backends/mode.c
LIBS=backends/mode/mode_intf.a
JO_ENGINE_SRC_DIR=../../jo_engine
COMPILER_DIR=../../Compiler
include $(COMPILER_DIR)/COMMON/jo_engine_makefile

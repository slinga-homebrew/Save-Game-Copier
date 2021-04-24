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
SRCS=main.c util.c backup.c backup-saturn.c backup-satiator.c backup-cd.c backends/actionreplay.c backends/sat.c bup_header.c md5/md5.c satiator/satiator.c satiator/cd.c backup-mode.c
LIBS=mode/mode_intf.a
JO_ENGINE_SRC_DIR=../../jo_engine
COMPILER_DIR=../../Compiler
include $(COMPILER_DIR)/COMMON/jo_engine_makefile

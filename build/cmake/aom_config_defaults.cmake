##
## Copyright (c) 2016, Alliance for Open Media. All rights reserved
##
## This source code is subject to the terms of the BSD 2 Clause License and
## the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
## was not distributed with this source code in the LICENSE file, you can
## obtain it at www.aomedia.org/license/software. If the Alliance for Open
## Media Patent License 1.0 was not distributed with this source code in the
## PATENTS file, you can obtain it at www.aomedia.org/license/patent.
##
# Defaults for every libaom configuration variable. Here we add all libaom
# config variables to the cmake variable cache, but omit the FORCE parameter to
# allow users to specify values when executing cmake to generate build files.
# Values here are used only if not set by the user.
set(INLINE "" CACHE STRING "Sets INLINE value for current target.")
set(RESTRICT "" CACHE STRING "Sets RESTRICT value for current target.")

# CPUs.
set(ARCH_ARM 0 CACHE NUMBER "Enables ARM architecture.")
set(ARCH_MIPS 0 CACHE NUMBER "Enables MIPS architecture.")
set(ARCH_X86 0 CACHE NUMBER "Enables X86 architecture.")
set(ARCH_X86_64 0 CACHE NUMBER "Enables X86_64 architecture.")

# ARM optimization flags.
set(HAVE_NEON 0 CACHE NUMBER "Enables NEON intrinsics optimizations.")
set(HAVE_NEON_ASM 0 CACHE NUMBER "Enables NEON assembly optimizations.")

# MIPS optimization flags.
set(HAVE_DSPR2 0 CACHE NUMBER "Enables DSPR2 optimizations.")
set(HAVE_MIPS32 0 CACHE NUMBER "Enables MIPS32 optimizations.")
set(HAVE_MIPS64 0 CACHE NUMBER "Enables MIPS64 optimizations. ")
set(HAVE_MSA 0 CACHE NUMBER "Enables MSA optimizations.")

# x86/x86_64 optimization flags.
set(HAVE_AVX 0 CACHE NUMBER "Enables AVX optimizations.")
set(HAVE_AVX2 0 CACHE NUMBER "Enables AVX2 optimizations.")
set(HAVE_MMX 0 CACHE NUMBER "Enables MMX optimizations. ")
set(HAVE_SSE 0 CACHE NUMBER "Enables SSE optimizations.")
set(HAVE_SSE2 0 CACHE NUMBER "Enables SSE2 optimizations.")
set(HAVE_SSE3 0 CACHE NUMBER "Enables SSE3 optimizations.")
set(HAVE_SSE4_1 0 CACHE NUMBER "Enables SSE 4.1 optimizations.")
set(HAVE_SSE4_2 0 CACHE NUMBER "Enables SSE 4.2 optimizations.")
set(HAVE_SSSE3 0 CACHE NUMBER "Enables SSSE3 optimizations.")

# Flags describing the build environment.
set(HAVE_FEXCEPT 0 CACHE NUMBER "Internal flag, GNU fenv.h present for target.")
set(HAVE_PTHREAD_H 0 CACHE NUMBER "Internal flag, target pthread support.")
set(HAVE_UNISTD_H 0 CACHE NUMBER "Internal flag, unistd.h present for target.")
set(HAVE_WXWIDGETS 0 CACHE NUMBER "WxWidgets present.")

# Build configuration flags.
set(CONFIG_AV1 1 CACHE NUMBER "Internal flag.")
set(CONFIG_AV1_DECODER 1 CACHE NUMBER "Enable AV1 decoder.")
set(CONFIG_AV1_ENCODER 1 CACHE NUMBER "Enable AV1 encoder.")
set(CONFIG_BIG_ENDIAN 0 CACHE NUMBER "Internal flag.")
set(CONFIG_GCC 0 CACHE NUMBER "Building with GCC (detected).")
set(CONFIG_GCOV 0 CACHE NUMBER "Enable gcov support.")
set(CONFIG_GPROF 0 CACHE NUMBER "Enable gprof support.")
set(CONFIG_LIBYUV 1 CACHE NUMBER "Enables libyuv scaling/conversion support.")
set(CONFIG_MSVS 0 CACHE NUMBER "Building with MS Visual Studio (detected).")
set(CONFIG_MULTITHREAD 1 CACHE NUMBER "Multithread support.")
set(CONFIG_OS_SUPPORT 0 CACHE NUMBER "Internal flag.")
set(CONFIG_PIC 0 CACHE NUMBER "Build with PIC enabled.")
set(CONFIG_RUNTIME_CPU_DETECT 1 CACHE NUMBER "Runtime CPU detection support.")
set(CONFIG_SHARED 0 CACHE NUMBER "Build shared libs.")
set(CONFIG_STATIC 1 CACHE NUMBER "Build static libs.")
set(CONFIG_WEBM_IO 1 CACHE NUMBER "Enables WebM support.")

# Debugging flags.
set(CONFIG_BITSTREAM_DEBUG 0 CACHE NUMBER "Bitstream debugging flag.")
set(CONFIG_DEBUG 0 CACHE NUMBER "Debug build flag.")
set(CONFIG_MISMATCH_DEBUG 0 CACHE NUMBER "Mismatch debugging flag.")

# Testing flags.
set(CONFIG_DECODE_PERF_TESTS 0 CACHE NUMBER "Enables decoder performance test.")
set(CONFIG_ENCODE_PERF_TESTS 0 CACHE NUMBER "Enables encoder performance test.")
set(CONFIG_UNIT_TESTS 1 CACHE NUMBER "Enables unit tests.")

# AV1 feature flags.
set(CONFIG_ACCOUNTING 0 CACHE NUMBER "Enables bit accounting.")
set(CONFIG_ANALYZER 0 CACHE NUMBER "Enables bit stream analyzer.")
set(CONFIG_COEFFICIENT_RANGE_CHECKING 0 CACHE NUMBER "Coefficient range check.")
set(CONFIG_INSPECTION 0 CACHE NUMBER "Enables bitstream inspection.")
set(CONFIG_INTERNAL_STATS 0 CACHE NUMBER "Codec stats.")
set(CONFIG_LOWBITDEPTH 0 CACHE NUMBER "Enables 8-bit optimized pipeline.")
set(CONFIG_SIZE_LIMIT 0 CACHE NUMBER "Limit max decode width/height.")
set(CONFIG_SPATIAL_RESAMPLING 1 CACHE NUMBER "Spatial resampling.")

# AV1 experiment flags.
set(CONFIG_AMVR 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_AOM_QM_EXT 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_CDF_UPDATE_MODE 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_DIST_8X8 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_ENTROPY_STATS 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_EXPLICIT_ORDER_HINT 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_EXT_DELTA_Q 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_FILM_GRAIN 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_FILM_GRAIN_SHOWEX 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_FP_MB_STATS 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_FRAME_COUNTER_FOR_SCALABILITY 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_FRAME_REFS_SIGNALING 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_HASH_ME 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_INTER_STATS_ONLY 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_LOWPRECISION_BLEND 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_MONO_VIDEO 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_OBU_FRAME 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_OBU_REDUNDANT_FRAME_HEADER 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_OPERATING_POINTS 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_RD_DEBUG 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_SCALABILITY 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_SKIP_SGR 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_SPATIAL_SEGMENTATION 1 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_TRAILING_BITS 0 CACHE NUMBER "AV1 experiment flag.")
set(CONFIG_FILEOPTIONS 1 CACHE NUMBER "AV1 config option flag.")

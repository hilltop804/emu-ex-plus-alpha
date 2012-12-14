# general Android ARM setup, included by ARMv6/ARMv7 makefiles
ifndef android_minSDK 
 android_minSDK := 9
endif
android_ndkArch := arm
ARCH := arm

ifneq ($(config_compiler),clang)
 ifeq ($(origin CC), default)
  CC := arm-linux-androideabi-gcc
 endif
endif

android_cpuFlags += -mthumb-interwork
CPPFLAGS += -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__
COMPILE_FLAGS += -fno-short-enums
ifneq ($(config_compiler),clang)
 COMPILE_FLAGS += -fsingle-precision-constant
 WARNINGS_CFLAGS += -Wdouble-promotion
endif
noDoubleFloat=1

include $(currPath)/android-gcc.mk

# Included by arch-specific Android makefiles

ENV := android
ENV_KERNEL := linux
CROSS_COMPILE := 1

ifeq ($(ANDROID_NDK_PATH),)
 $(error setAndroidNDKPath.mk was not included in base makefile)
endif

ANDROID_CLANG_TOOLCHAIN_PATH ?= $(wildcard $(ANDROID_NDK_PATH)/toolchains/llvm/prebuilt/*)
ANDROID_CLANG_TOOLCHAIN_BIN_PATH := $(ANDROID_CLANG_TOOLCHAIN_PATH)/bin

ifdef V
 $(info NDK Clang path: $(ANDROID_CLANG_TOOLCHAIN_BIN_PATH))
endif

ifneq ($(wildcard $(ANDROID_NDK_PATH)/sysroot),)
 $(error your NDK contains a deprecated sysroot directory, please upgrade to at least r23)
endif

ifneq ($(filter 9 16, $(android_ndkSDK)),)
 # SDK 9 no longer supported since NDK r16 & SDK 16 since NDK r24, enable compatibilty work-arounds
 CPPFLAGS += -DANDROID_COMPAT_API=$(android_ndkSDK)
 android_ndkLinkSysroot := $(IMAGINE_PATH)/bundle/android/$(CHOST)/$(android_ndkSDK)
endif

ifdef android_ndkLinkSysroot
 VPATH += $(android_ndkLinkSysroot)/usr/lib
 ifdef V
  $(info NDK link sysroot path: $(android_ndkLinkSysroot))
 endif
else
 VPATH += $(ANDROID_CLANG_TOOLCHAIN_PATH)/sysroot/usr/lib/$(CHOST)/$(android_ndkSDK)
endif

config_compiler ?= clang

ifeq ($(origin CC), default)
 CC := $(ANDROID_CLANG_TOOLCHAIN_BIN_PATH)/clang
 CXX := $(CC)++
 AR := $(ANDROID_CLANG_TOOLCHAIN_BIN_PATH)/llvm-ar
 RANLIB := $(AR) s
 STRIP := $(ANDROID_CLANG_TOOLCHAIN_BIN_PATH)/llvm-strip
 OBJDUMP := $(ANDROID_CLANG_TOOLCHAIN_BIN_PATH)/llvm-objdump
 CLANG_TIDY := $(ANDROID_CLANG_TOOLCHAIN_BIN_PATH)/clang-tidy
 toolchainEnvParams += STRIP="$(STRIP)" OBJDUMP="$(OBJDUMP)"
else
 # TODO: user-defined compiler
 ifneq ($(findstring $(shell $(CC) -v), "clang version"),)
  $(info detected clang compiler)
  config_compiler = clang
 endif
 $(error user-defined compiler not yet supported)
endif

ifneq ($(config_compiler),clang)
 $(error config_compiler must be set to clang)
endif

include $(buildSysPath)/clang.mk
CFLAGS_TARGET += -target $(clangTarget)

# libc++
android_useExternalLibcxx := 1
ifdef android_useExternalLibcxx
 ifneq ($(pkgName),libcxx) # check we aren't building lib++ itself
  STDCXXLIB = -nostdlib++ -lc++ -lc++abi $(android_cxxSupportLibs)
  CPPFLAGS += -nostdinc++ -I$(IMAGINE_SDK_PLATFORM_PATH)/include/c++/v1
  android_staticLibcxxName := libc++.a
 endif
else
 STDCXXLIB = -static-libstdc++
 android_staticLibcxxName := libc++_static.a
endif

ifdef ANDROID_APK_SIGNATURE_HASH
 CPPFLAGS += -DANDROID_APK_SIGNATURE_HASH=$(ANDROID_APK_SIGNATURE_HASH)
endif

CFLAGS_TARGET += $(android_cpuFlags) -no-canonical-prefixes
ASMFLAGS ?= $(CFLAGS_TARGET) -Wa,--noexecstack
ifdef android_ndkLinkSysroot
 LDFLAGS_SYSTEM += --sysroot=$(android_ndkLinkSysroot)
endif
LDFLAGS_SYSTEM += -no-canonical-prefixes \
-Wl,--no-undefined,-z,noexecstack,-z,relro,-z,now
linkAction = -Wl,-soname,lib$(android_metadata_soName).so -shared
LDLIBS_SYSTEM += -lm
LDLIBS += $(LDLIBS_SYSTEM)
CPPFLAGS += -DANDROID
LDFLAGS_SYSTEM += -s \
-Wl,-O3,--gc-sections,--compress-debug-sections=$(COMPRESS_DEBUG_SECTIONS),--icf=all,--as-needed,--warn-shared-textrel,--fatal-warnings \
-Wl,--exclude-libs,libgcc.a,--exclude-libs,libgcc_real.a -Wl,--exclude-libs,libatomic.a,--lto-whole-program-visibility

# Don't include public libc++ symbols in main shared object file unless other linked objects need them  
ifndef cxxStdLibLinkedObjects
 LDFLAGS_SYSTEM += -Wl,--exclude-libs,libc++abi.a,--exclude-libs,$(android_staticLibcxxName)
endif

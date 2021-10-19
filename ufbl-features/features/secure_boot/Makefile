# Copyright (c) 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved

#
# Common list of objects files we want to include
#
SECURE_BOOT_OBJECTS = image_verify.o

# Common set of CFLAGS we need
SECURE_BOOT_CFLAGS = \
	-DSUPPORT_UBOOT \
	-std=c99 \
	-ffunction-sections -fdata-sections \
	-DUFBL_FEATURE_SECURE_BOOT

SECURE_BOOT_HEADERS =

ifeq (yes,$(strip $(UFBL_PROJ_ABC)))
SECURE_BOOT_CFLAGS += -DUFBL_PROJ_ABC
endif

ifeq (true,$(strip $(FEATURE_COMMON_OPENSSL)))
SECURE_BOOT_CFLAGS += -DUFBL_FEATURE_SECURE_BOOT_OPENSSL

SECURE_BOOT_HEADERS += \
	-I$(UFBL_PATH)/features/common_openssl/include \
	-I$(UFBL_PATH)/features/common_openssl/include/openssl \
	-I$(UFBL_PATH)/features/common_openssl/crypto \
	-I$(UFBL_PATH)/features/common_openssl/crypto/bn \
	-I$(UFBL_PATH)/features/common_openssl/crypto/asn1 \
	-I$(UFBL_PATH)/features/common_openssl/bf \
	-I$(UFBL_PATH)/features/common_openssl/conf \
	-I$(UFBL_PATH)/features/common_openssl/evp \
	-I$(UFBL_PATH)/features/common_openssl/modes \
	-I$(UFBL_PATH)/features/common_openssl/objects \
	-I$(UFBL_PATH)/features/common_openssl/ripemd \
	-I$(UFBL_PATH)/features/common_openssl/rsa \
	-I$(UFBL_PATH)/features/common_openssl/sha \
	-I$(UFBL_PATH)/features/common_openssl/x509v3

# Default OpenSSL CFLAGS
SECURE_BOOT_CFLAGS += -DOPENSSL_BN_ASM_MONT \
	-DAES_ASM \
	-DSHA1_ASM \
	-DSHA256_ASM \
	-DSHA512_ASM \
	-DNO_WINDOWS_BRAINDEATH \
	-DOPENSSL_IMPLEMENTS_strncasecmp \
	-DGETPID_IS_MEANINGLESS \
	-DOPENSSL_THREADS \
	-D_REENTRANT \
	-DDSO_DLFCN \
	-DHAVE_DLFCN_H \
	-DZLIB \
	-DL_ENDIAN

# Disable un-necessary crypto algos
SECURE_BOOT_CFLAGS += -DOPENSSL_NO_CAMELLIA \
	-DOPENSSL_NO_CAPIENG \
	-DOPENSSL_NO_CAST \
	-DOPENSSL_NO_CMS \
	-DOPENSSL_NO_STDIO \
	-DOPENSSL_NO_GMP \
	-DOPENSSL_NO_IDEA \
	-DOPENSSL_NO_JPAKE \
	-DOPENSSL_NO_MD2 \
	-DOPENSSL_NO_MDC2 \
	-DOPENSSL_NO_RC2 \
	-DOPENSSL_NO_RC4 \
	-DOPENSSL_NO_RC5 \
	-DOPENSSL_NO_BF \
	-DOPENSSL_NO_SHA0 \
	-DOPENSSL_NO_RFC3779 \
	-DOPENSSL_NO_SEED \
	-DOPENSSL_NO_STORE \
	-DOPENSSL_NO_WHIRLPOOL \
	-DOPENSSL_NO_MD4 \
	-DOPENSSL_NO_MD5 \
	-DOPENSSL_NO_DES \
	-DOPENSSL_NO_OCSP \
	-DOPENSSL_NO_AES \
	-DOPENSSL_NO_DSA \
	-DOPENSSL_NO_DH \
	-DOPENSSL_NO_SHA512 \
	-DOPENSSL_NO_HW \
	-DOPENSSL_NO_ENGINE \
	-DOPENSSL_NO_EC2M \
	-DOPENSSL_NO_PKCS12 \
	-DOPENSSL_NO_EC \
	-DOPENSSL_NO_ECDSA \
	-DNO_SYS_TYPES_H

SECURE_BOOT_CFLAGS += \
	-DOPENSSL_SYS_LK -DOPENSSL_NO_HMAC -DNO_STDERR -DNO_ABORT -DNO_TIME \
	-DNO_STRCASECMP -DOPENSSL_NO_FP_API -DOPENSSL_NO_SSCANF -DNO_SSLV23 \
	-DOPENSSL_NO_OBJECT

SECURE_BOOT_OBJECTS += image_verify_openssl.o
endif

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
SECURE_BOOT_CFLAGS += -DUFBL_FEATURE_SECURE_BOOT_LTC

SECURE_BOOT_HEADERS += \
	-I$(UFBL_PATH)/features/libtomcrypt/ \
	-I$(UFBL_PATH)/features/libtomcrypt/src/headers

SECURE_BOOT_OBJECTS += image_verify_ltc.o

SECURE_BOOT_CFLAGS += \
	-DLTC_SOURCE \
	-DLTC_NO_FILE \
	-DUSE_LTM -DLTM_DESC \
	-D_WCHAR_T -DLTC_CLEAN_STACK
endif

# Begin u-boot specific rules
ifeq ($(UFBL_CONFIG_KBUILD),true)

UBOOTINCLUDE += $(SECURE_BOOT_HEADERS)
ccflags-y    += $(SECURE_BOOT_CFLAGS)
obj-y        += $(SECURE_BOOT_OBJECTS)

else # default rules

include $(TOPDIR)/config.mk

SECURE_BOOT_LIB = $(obj)libsecure_boot.o
SOBJS	=

# Default CFLAGS
CFLAGS += \
	$(SECURE_BOOT_CFLAGS) \
	$(SECURE_BOOT_HEADERS)

COBJS-y += \
	$(SECURE_BOOT_OBJECTS)

COBJS	= $(COBJS-y)
SRCS 	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

$(SECURE_BOOT_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif #UFBL_CONFIG_KBUILD

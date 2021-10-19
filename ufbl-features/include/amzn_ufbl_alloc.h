/*
 * Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
 */
#ifndef __AMZN_UFBL_ALLOC_H
#define __AMZN_UFBL_ALLOC_H

#if defined(SUPPORT_KERNELFLINGER)

#define amzn_plat_alloc AllocatePool
#define amzn_plat_free FreePool

#else /* defined(SUPPORT_KERNELFLINGER) */

#ifndef UFBL_SIG_TESTS
/* Leaving this in while compiling secure_boot on */
/* a host causes errors to appear in stdio2.h */
#include <malloc.h>
#endif

#define amzn_plat_alloc malloc
#define amzn_plat_free free

#endif /* defined(SUPPORT_KERNELFLINGER) */

#endif /* AMZN_UFBL_ALLOC */

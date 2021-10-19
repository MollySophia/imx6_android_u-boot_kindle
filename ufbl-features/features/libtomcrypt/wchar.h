/*
 * Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
 */
#ifndef __WCHAR_H
#define __WCHAR_H

#ifndef FILE
#define FILE void
#endif

/*
 * We do a hack here to define wchar_t as uint8.
 * This is probably OK as CodeSigner do not sign
 * certificates with non 7-bit ASCII characters
 */
typedef unsigned char wchar_t;

#endif

/*
* Base64 encoding/decoding (RFC1341)
* Copyright (c) 2005, Jouni Malinen <j@w1.fi>
*
* This software may be distributed under the terms of the BSD license.
* See README for more details.
*/
#include <stdlib.h>
#include <math.h>

#include "dns.h"

#ifndef BASE64_H
#define BASE64_H

unsigned char * base64_encode(const unsigned char *src, size_t len,
                            size_t *out_len);
unsigned char * base64_decode(const unsigned char *src, size_t len,
                            size_t *out_len);

#define MAX_BASE64_LENGTH  141

#endif /* BASE64_H */

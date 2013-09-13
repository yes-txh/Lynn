// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/14/2011 03:03:07 PM
// Description:
// from http://googlesitemapgenerator.googlecode.com/svn/trunk/src/common/hash.cc

// Copyright 2009 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef COMMON_CRYPTO_HASH_FINGERPRINT_HPP
#define COMMON_CRYPTO_HASH_FINGERPRINT_HPP

#include <stddef.h>
#include "common/base/stdint.h"

// Generate finger print for a string.
// Both 31 and 33 are fast multipliers, which can be implemented by shift and
// add operator. And they are also widely adopted hash function multiplier.
uint64_t FingerPrint(const void *buf, size_t len);
uint64_t FingerPrint(const char *s);

#endif // COMMON_CRYPTO_HASH_FINGERPRINT_HPP

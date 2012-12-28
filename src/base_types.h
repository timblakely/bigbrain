/*
 Copyright 2012 Google Inc. All Rights Reserved.
 Author: blakely@google.com (Tim Blakely)

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#ifndef BASE_TYPES_H_
#define BASE_TYPES_H_

#include <stddef.h>
#include "boost/cstdint.hpp"

typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#define USE_EXPX

// Synthesis used an approximation of EXP, so reproduce it here if necessary
#ifdef USE_EXPX

#include "math.h"

#define EXPX_A 1048576/M_LN2
#define EXPX_C (1072693248 - 60801)

inline double expx(double value) {
  union {
    double d;
    struct {
      int j, i;
    } n;
  } _eco;
  _eco.n.i = static_cast<int>((EXPX_A * value) + EXPX_C);
  _eco.n.j = 0;

  return _eco.d;
}
#else
#define expx(value) exp(value)
#endif

#endif  // BASE_TYPES_H_

/*
   Copyright 2018 AGC.

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


#pragma once
#ifndef KOZET_FIXED_POINT_KFP_EXTRA_H
#define KOZET_FIXED_POINT_KFP_EXTRA_H

#include "./kfp.h"

namespace kfp {
  static constexpr size_t CORDIC_ITERATIONS = 30;
  // CORDIC K constant in 2.30 format.
  static constexpr s2_30 CORDIC_K = s2_30::raw(0x26DD3B6A);

  // The arctangents of 2**(-i) for i = 0, 1, ... 29, in turns (NOT radians).
  // Generated by this Perl6 expression:
  // ((^30).map({"frac32::raw(0x"~(2**32*atan(2**-$_)/tau).floor.base(16)~")"})).join(", ")
  static constexpr frac32 arctangentsT[] = {
    frac32::raw(0x20000000), frac32::raw(0x12E4051D), frac32::raw(0x9FB385B), frac32::raw(0x51111D4),
    frac32::raw(0x28B0D43), frac32::raw(0x145D7E1), frac32::raw(0xA2F61E), frac32::raw(0x517C55),
    frac32::raw(0x28BE53), frac32::raw(0x145F2E), frac32::raw(0xA2F98), frac32::raw(0x517CC),
    frac32::raw(0x28BE6), frac32::raw(0x145F3), frac32::raw(0xA2F9), frac32::raw(0x517C),
    frac32::raw(0x28BE), frac32::raw(0x145F), frac32::raw(0xA2F), frac32::raw(0x517),
    frac32::raw(0x28B), frac32::raw(0x145), frac32::raw(0xA2), frac32::raw(0x51),
    frac32::raw(0x28), frac32::raw(0x14), frac32::raw(0xA), frac32::raw(0x5),
    frac32::raw(0x2), frac32::raw(0x1),
  };

  // A list of ratios to intermediate K-values, in 2.30 format.
  // That is, K_i = 1 / sqrt(1 + 2**(-2i)) for i = 0, 1, ... 31.
  // Generated by this Perl6 expression:
  // my @inv = (0..^32).map({1/sqrt(1+2**(-2*$_))});
  // say flat(1,[\*] @inv).map({"s2_30::raw(0x" ~ floor(2**30*$_).base(16)~")"}).join(", ");
  static constexpr s2_30 intermediateK[] = {
    s2_30::raw(0x40000000), s2_30::raw(0x2D413CCC), s2_30::raw(0x287A26C4), s2_30::raw(0x2744C374),
    s2_30::raw(0x26F72283), s2_30::raw(0x26E3B583), s2_30::raw(0x26DED9F5), s2_30::raw(0x26DDA30D),
    s2_30::raw(0x26DD5552), s2_30::raw(0x26DD41E4), s2_30::raw(0x26DD3D08), s2_30::raw(0x26DD3BD1),
    s2_30::raw(0x26DD3B83), s2_30::raw(0x26DD3B70), s2_30::raw(0x26DD3B6B),
  };

  // A list of ratios of intermediate K-values to CORDIC_K, in 2.30 format.
  // That is, K(32) / K(i) for i = 0, 1, ... 
  // Generated by this Perl6 expression:
  // my @inv = (0..^32).map({1/sqrt(1+2**(-2*$_))});
  // say flat(1,[\*] @inv).map({"s2_30::raw(0x" ~ floor(2**30*$_/([*] @inv)).base(16)~")"}).join(", ");
  static constexpr s2_30 intermediateKRatio[] = {
    s2_30::raw(0x69648523), s2_30::raw(0x4A861BD3), s2_30::raw(0x42A7FAAB), s2_30::raw(0x40AA7DCD),
    s2_30::raw(0x402AA7D5), s2_30::raw(0x400AAA7D), s2_30::raw(0x4002AAA7), s2_30::raw(0x4000AAAA),
    s2_30::raw(0x40002AAA), s2_30::raw(0x40000AAA), s2_30::raw(0x400002AA), s2_30::raw(0x400000AA),
    s2_30::raw(0x4000002A), s2_30::raw(0x4000000A), s2_30::raw(0x40000002),
  };

  // Calculates sine and cosine using CORDIC
  // (https://en.wikipedia.org/wiki/CORDIC)
  // t = angle stored in a frac32 of a turn
  // c = reference to where you want cosine to be stored
  // s = reference to where you want sine to be stored
  // Note: cosine and sine values are returned as s2_30 in order to represent both -1 and 1 correctly.
  void sincos(frac32 t, s2_30& c, s2_30& s) {
    // Check if angle (in radians) is greater than pi/2 or less than pi/2
    bool inv = t > frac32::raw(0x40000000) && t < frac32::raw(0xC0000000u);
    // If so, then rotate by pi
    t += frac32::raw(0x80000000u * inv);
    s2_30 vx = CORDIC_K;
    s2_30 vy = 0;
    unsigned int i;
    for (i = 0; i < CORDIC_ITERATIONS && t != frac32(0); ++i) {
      // new vector = [1, -factor; factor, 1] old vector
      s2_30 nx, ny;
      bool isNeg = (t.underlying & 0x8000'0000u) != 0;
      if (!isNeg) {
        nx = vx - (vy >> i);
        ny = (vx >> i) + vy;
      } else {
        nx = vx + (vy >> i);
        ny = -(vx >> i) + vy;
      }
      vx = nx;
      vy = ny;
      t += (isNeg) ? arctangentsT[i] : -arctangentsT[i];
    }
    if (i < (sizeof(intermediateKRatio) / sizeof(intermediateKRatio[0]))) {
      vx *= intermediateKRatio[i];
      vy *= intermediateKRatio[i];
    }
    c = inv ? -vx : vx;
    s = inv ? -vy : vy;
  }

  // Calculating atan2 using CORDIC
  // c = cosine value
  // s = sine value
  // r = stores hypot(c, s)
  // t = stores atan2(s, c)
  template<typename F>
  void rectp(F c, F s, F& r, frac32& t) {
    bool inv = c < F(0); // Left of y-axis?
    if (inv) {
      c = -c;
      s = -s;
    }
    frac32 a = 0;
    F vx = c;
    F vy = s;
    unsigned int i;
    for (i = 0; i < CORDIC_ITERATIONS && vy != F(0); ++i) {
      F nx, ny;
      if (vy <= F(0)) {
        nx = vx - (vy >> i);
        ny = (vx >> i) + vy;
      } else {
        nx = vx + (vy >> i);
        ny = -(vx >> i) + vy;
      }
      vx = nx;
      a += (vy > F(0)) ? arctangentsT[i] : -arctangentsT[i];
      vy = ny;
    }
    t = a;
    if (i < (sizeof(intermediateK) / sizeof(intermediateK[0])))
      r = vx * intermediateK[i];
    else
      r = vx * CORDIC_K;
    if (inv) t += frac32::raw(0x80000000u);
  }

  template<typename I, size_t d>
  bool isInterior(Fixed<I, d> x, Fixed<I, d> y, Fixed<I, d> r) {
    using D = DoubleType<I>;
    D hypot =
      ((D) x.underlying) * x.underlying +
      ((D) y.underlying) * y.underlying;
    D r2 =
      ((D) r.underlying) * r.underlying;
    return hypot <= r2;
  }
  // Adapted from:
  // http://www.codecodex.com/wiki/Calculate_an_integer_square_root
  template<typename I>
  I sqrtiFast(I n) {
    I est = (I) sqrt((float) n);
    while (est * est < n) ++est;
    while (est * est > n) --est;
    return est;
  }
  template<typename I>
  I sqrti(I nn) {
    if (nn < 0) {
      std::cerr << "Positive n expected in kfp::sqrti";
      abort();
    }
    using U = std::make_unsigned_t<I>;
    U n = (U) nn;
    U root = 0;
    U rem = n;
    U place = (U) 1 << (CHAR_BIT * sizeof(U) - 2);
    while (place > rem)
      place >>= 2;
    while (place != 0) {
      if (rem >= root + place) {
        rem -= root + place;
        root += place *= 2;
      }
      root >>= 1;
      place >>= 2;
    }
    return root;
  }
  template<typename I, size_t d>
  Fixed<I, d> sqrt(Fixed<DoubleTypeExact<I>, 2 * d> x) {
    DoubleTypeExact<I> s = sqrtiFast(x.underlying);
    I max = std::numeric_limits<I>::max();
    if (s >= max)
      return Fixed<I, d>::raw(max);
    return Fixed<I, d>::raw((I) s);
  }
  template<typename I, size_t d>
  Fixed<I, d> hypot(Fixed<I, d> x, Fixed<I, d> y) {
    auto h2 = longMultiply(x, x) + longMultiply(y, y);
    return sqrt(h2);
  }
}

#endif // KOZET_FIXED_POINT_KFP_EXTRA_H

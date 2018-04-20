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
#ifndef KOZET_FIXED_POINT_KFP_H
#define KOZET_FIXED_POINT_KFP_H

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <iosfwd>
#include <limits>
#include <type_traits>
#include <utility>

namespace kfp {
  // Check for prescence of __int128
#ifdef __SIZEOF_INT128__
  using int128_t = __int128_t;
  using uint128_t = __uint128_t;
#else
  // Use a library that provides a 128-bit int type with the usual operations
  // and define kfp::int128_t and kfp::uint128_t to the signed/unsigned variant
  // respectively if not done already.
#endif
  template<typename T>
  struct DTI;
  template<> struct DTI<int8_t> { typedef int_fast16_t type; };
  template<> struct DTI<int16_t> { typedef int_fast32_t type; };
  template<> struct DTI<int32_t> { typedef int_fast64_t type; };
  template<> struct DTI<int64_t> { typedef int128_t type; };
  template<> struct DTI<uint8_t> { typedef uint_fast16_t type; };
  template<> struct DTI<uint16_t> { typedef uint_fast32_t type; };
  template<> struct DTI<uint32_t> { typedef uint_fast64_t type; };
  template<> struct DTI<uint64_t> { typedef uint128_t type; };
  template<typename T>
  using DoubleType = typename DTI<T>::type;
  // I = underlying int type
  // d = number of bits to the right of the decimal
  template<typename I, size_t d>
  struct Fixed {
    using F = kfp::Fixed<I, d>;
    using Underlying = I;
    I underlying;
    // Sanity checks
    // Is I an integer?
    static_assert(std::numeric_limits<I>::is_integer,
      "I must be an integer type");
    template<typename I2, size_t d2>
    using HasIntegerBits = std::enable_if_t<(d2 < CHAR_BIT * sizeof(I2))>;
    template<typename I2, size_t d2>
    using HasNoIntegerBits = std::enable_if_t<(d2 >= CHAR_BIT * sizeof(I2)), int>;
    // Constructors
    constexpr Fixed() : underlying(0) {}
    template<typename I2 = I, size_t d2 = d>
    constexpr Fixed(I value, HasIntegerBits<I2, d2>* dummy = nullptr) : underlying(value << d) {}
    template<typename I2 = I, size_t d2 = d>
    constexpr Fixed(I value, HasNoIntegerBits<I2, d2>* dummy = nullptr) : underlying(0) {}
    // Copy constructor not used because it makes the type
    // not trivially constructible
    // constexpr Fixed(const F& value) : underlying(value.underlying) {}
    // Implicit cast from smaller type
    template<typename I2, size_t d2>
    constexpr Fixed(const Fixed<I2, d2>& other) {
      static_assert(
        (std::numeric_limits<I>::digits - d) >=
          (std::numeric_limits<I2>::digits - d2),
        "Cannot implicitly cast into a type with fewer integral digits");
      static_assert(d >= d2,
        "Cannot implicitly cast into a type with fewer fractional bits");
      // How much left should we shift?
      underlying = other.underlying;
      underlying <<= (fractionalBits() - other.fractionalBits());
    }
    constexpr static F raw(I underlying) {
      F ret;
      ret.underlying = underlying;
      return ret;
    }
    // Traits
    static constexpr size_t integralBits() noexcept {
      return CHAR_BIT * sizeof(I) - d;
    }
    static constexpr size_t integralDigits() noexcept {
      return std::numeric_limits<I>::digits - d;
    }
    static constexpr size_t fractionalBits() noexcept {
      return d;
    }
    // Operator defines
    // Copy assignment not used because it makes the type
    // not trivially constructible
    /*F& operator=(const F& other) {
      underlying = other.underlying;
      return *this;
    }*/
    F& operator=(const I& i) {
      underlying = i << d;
      return *this;
    }
    template<typename I2, size_t d2>
    F& operator=(const Fixed<I2, d2>& other) {
      static_assert(integralDigits() >= other.integralDigits(),
        "Cannot implicitly cast into a type with fewer integral digits");
      static_assert(fractionalBits() >= other.fractionalBits(),
        "Cannot implicitly cast into a type with fewer fractional bits");
      // How much left should we shift?
      underlying = other.underlying;
      underlying <<= (fractionalBits() - other.fractionalBits());
      return *this;
    }
#define DEF_OP_BOILERPLATE(o) \
      template<typename X> \
      F operator o(const X& other) const { \
        F ret = *this; \
        ret o##= other; \
        return ret; \
      }
#define DEF_OP_BOILERPLATE2(o) \
      DEF_OP_BOILERPLATE(o) \
      F& operator o##=(const I& other) { \
        *this o##= F(other); \
        return *this; \
      }
#define DEF_OP(o) \
      DEF_OP_BOILERPLATE2(o) \
      F& operator o##=(const F& other)
    // end define
    DEF_OP(+) {
      underlying += other.underlying;
      return *this;
    }
    DEF_OP(-) {
      underlying -= other.underlying;
      return *this;
    }
    DEF_OP_BOILERPLATE(*)
    template<size_t d2>
    F& operator*=(const Fixed<I, d2>& other) {
      DoubleType<I> prod = ((DoubleType<I>) underlying) * other.underlying;
      underlying = (I) (prod >> other.fractionalBits());
      return *this;
    }
    F& operator*=(I other) {
      underlying *= other;
      return *this;
    }
    DEF_OP_BOILERPLATE(/)
    F& operator /=(const F& other) {
      DoubleType<I> dividend = ((DoubleType<I>) underlying) << d;
      underlying = (I) (dividend / other.underlying);
      return *this;
    }
    F& operator/=(I other) {
      underlying /= other;
      return *this;
    }
    DEF_OP_BOILERPLATE(<<)
    F& operator<<=(int shift) {
      underlying <<= shift;
      return *this;
    }
    DEF_OP_BOILERPLATE(>>)
    F& operator>>=(int shift) {
      underlying >>= shift;
      return *this;
    }
    F operator-() const {
      F ret = *this;
      ret.underlying = -ret.underlying;
      return ret;
    }
#undef DEF_OP
#undef DEF_OP_BOILERPLATE
#undef DEF_OP_BOILERPLATE2
    // Explicit conversions
    explicit operator I() const {
      return floor();
    }
    // Other functions
    I floor() const {
      return (d < sizeof(I) * CHAR_BIT) ?
        underlying >> d :
        0;
    }
    double toDouble() const {
      return ((double) underlying) / exp2(d);
    }
  };
  // Relational operators
#define DEF_RELATION(o) \
    template<typename I, size_t d> \
    bool operator o(const Fixed<I, d>& a, I b) { \
      return (b >> Fixed<I, d>::integralBits()) == 0 && \
        a.underlying o (b << d); \
    } \
    template<typename I, size_t d> \
    bool operator o(I a, const Fixed<I, d>& b) { \
      return (a >> Fixed<I, d>::integralBits()) == 0 && \
        (a << d) o b.underlying; \
    } \
    template<typename I, size_t d> \
    bool operator o(const Fixed<I, d>& a, const Fixed<I, d>& b) { \
      return a.underlying o b.underlying; \
    }
  DEF_RELATION(==)
  DEF_RELATION(!=)
  DEF_RELATION(<)
  DEF_RELATION(<=)
  DEF_RELATION(>)
  DEF_RELATION(>=)
#undef DEF_RELATION
  // end
  template<typename I, size_t d>
  std::ostream& operator<<(std::ostream& fh, const Fixed<I, d>& x) {
    return fh << x.toDouble();
  }
  using s16_16 = Fixed<int32_t, 16>;
  using u16_16 = Fixed<uint32_t, 16>;
  using s2_30 = Fixed<int32_t, 30>;
  using s34_30 = Fixed<int64_t, 30>;
  using frac32 = Fixed<uint32_t, 32>;
  // User-defined literals
  template<typename I, size_t d>
  constexpr Fixed<I, d> convert(const char* s) {
    using F = Fixed<I, d>;
    // Decimal point present?
    const char* t = s;
    while (*t != '\0' && *t != '.') ++t;
    bool hasDecimal = *t == '.';
    const char* u = s;
    I iPart = 0;
    while (u != t) {
      iPart *= 10;
      iPart += (*u) - '0';
      ++u;
    }
    F res(iPart);
    if (hasDecimal) {
      u = t + 1;
      F mult(1);
      while (*u != '\0') {
        mult /= 10;
        res += mult * ((*u) - '0');
        ++u;
      }
    }
    return res;
  }
#define DEFINE_OPERATOR_LITERAL(type) \
  constexpr type operator""_##type(const char* s, size_t) { \
    return convert<type::Underlying, type::fractionalBits()>(s); \
  }
  namespace literals {
    DEFINE_OPERATOR_LITERAL(s16_16)
    DEFINE_OPERATOR_LITERAL(u16_16)
    DEFINE_OPERATOR_LITERAL(s2_30)
    DEFINE_OPERATOR_LITERAL(s34_30)
    DEFINE_OPERATOR_LITERAL(frac32)
  }
#undef DEFINE_OPERATOR_LITERAL
}

template<typename I, size_t d>
struct std::numeric_limits<kfp::Fixed<I, d>> {
  static constexpr double log10of2 = 0.301029995663981;
  static constexpr int ctl10o2(int x) {
    return (31 * x + 99) / 100;
  }
  using F = kfp::Fixed<I, d>;
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = std::numeric_limits<I>::is_signed;
  static constexpr bool is_integer = false;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
  static constexpr bool has_denorm = false;
  static constexpr bool has_denorm_loss = false;
  // TODO set a proper value for this
  static constexpr std::float_round_style round_style =
    std::float_round_style::round_indeterminate;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = std::numeric_limits<I>::is_bounded;
  static constexpr bool is_modulo = std::numeric_limits<I>::is_modulo;
  static constexpr int digits = std::numeric_limits<I>::digits;
  static constexpr int digits10 = std::numeric_limits<I>::digits10;
  static constexpr int max_digits10 =
    ctl10o2(std::numeric_limits<I>::digits) + 1;
  static constexpr int radix = 2;
  static constexpr int min_exponent = -d + 1;
  static constexpr int max_exponent =
    std::numeric_limits<I>::digits - d;
  static constexpr int min_exponent10 =
    ctl10o2(-d + 1);
  static constexpr int max_exponent10 =
    ctl10o2(std::numeric_limits<I>::digits - d);
  static constexpr bool traps = true;
  static constexpr bool tinyness_before = false;
  static constexpr F min() {
    return F::raw(std::numeric_limits<I>::min());
  }
  static constexpr F max() {
    return F::raw(std::numeric_limits<I>::max());
  }
  static constexpr F lowest() {
    return F::raw(std::numeric_limits<I>::lowest());
  }
  static constexpr F epsilon() {
    return F::raw(1);
  }
  static constexpr F round_error() {
    return F::raw(1 << (d - 1));
  }
  static constexpr F infinity() {
    return F(0);
  }
  static constexpr F quiet_NaN() {
    return F(0);
  }
  static constexpr F signaling_NaN() {
    return F(0);
  }
  static constexpr F denorm_min() {
    return F(0);
  }
};

template<typename I, size_t d>
struct std::hash<kfp::Fixed<I, d>> {
  using F = kfp::Fixed<I, d>;
  size_t operator()(const F& f) const {
    return std::hash<I>()(f.underlying);
  }
};

#endif // KOZET_FIXED_POINT_KFP_H
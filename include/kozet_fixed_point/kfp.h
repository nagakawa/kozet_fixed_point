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
#define MAX_NATIVE_BITS 64
#else
#define MAX_NATIVE_BITS 32
#endif
  /*
    The following functions are defined:
    std::pair<I, U> mulOverflow(I a, I b);
    * Multiplies a and b and returns a pair representing the product across two integers of the same size
    I divOverflow(I ah, U al, I b);
    * Does the opposite: ah and al are the high and low bits of the dividend, and the function returns the remainder
  */
  // Fallback for generic types
  template<typename I>
  std::enable_if_t<
    !std::numeric_limits<I>::is_integer ||
    (CHAR_BIT * sizeof(I) > MAX_NATIVE_BITS),
    std::pair<I, std::make_unsigned_t<I>>
  > mulOverflow(I a, I b) {
    // FIXME: this might work only with unsigned values
    static_assert(std::numeric_limits<I>::is_integer,
      "I must be an integer type");
    constexpr int bits = CHAR_BIT * sizeof(I);
    constexpr int sh = bits >> 1;
    using U = std::make_unsigned_t<I>;
    I ah = a >> sh;
    I al = a & ((1 << sh) - 1);
    I bh = b >> sh;
    I bl = b & ((1 << sh) - 1);
    U partial = al * bl;
    I mid1 = ah * bl;
    I mid2 = al * bh;
    I carry = ah * bh;
    carry += (mid1 >> sh);
    carry += (mid2 >> sh);
    partial += mid1 & (((1 << sh) - 1));
    partial += mid2 & (((1 << sh) - 1));
    return {carry, partial};
  }
  // TODO: divOverflow for the general case
  // For unbounded types
  template<typename I>
  std::enable_if_t<
    std::numeric_limits<I>::is_integer &&
    !std::numeric_limits<I>::is_bounded,
    std::pair<I, uint64_t>
  > mulOverflow(I a, I b) {
    constexpr int bits = 64;
    I prod = a;
    prod *= b;
    return {prod >> bits, (uint64_t) prod};
  }
  template<typename I>
  std::enable_if_t<
    std::numeric_limits<I>::is_integer &&
    !std::numeric_limits<I>::is_bounded,
    I
  > divOverflow(I ah, uint64_t al, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    I prod = ah;
    prod <<= bits;
    prod |= al;
    prod /= b;
    return prod;
  }
  // For types 32 bits or smaller
  template<typename I>
  std::enable_if_t<
    CHAR_BIT * sizeof(I) <= 32 &&
    std::numeric_limits<I>::is_signed,
    std::pair<I, std::make_unsigned_t<I>>
  > mulOverflow(I a, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    using U = std::make_unsigned_t<I>;
    int64_t prod = a;
    prod *= b;
    return {(I) (prod >> bits), (U) prod};
  }
  template<typename I>
  std::enable_if_t<
    CHAR_BIT * sizeof(I) <= 32 &&
    !std::numeric_limits<I>::is_signed,
    std::pair<I, std::make_unsigned_t<I>>
  > mulOverflow(I a, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    using U = std::make_unsigned_t<I>;
    uint64_t prod = a;
    prod *= b;
    return {(I) (prod >> bits), (U) prod};
  }
  template<typename I>
  std::enable_if_t<
    CHAR_BIT * sizeof(I) <= 32 &&
    std::numeric_limits<I>::is_signed,
    I
  > divOverflow(I ah, std::make_unsigned_t<I> al, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    int64_t prod = ah;
    prod <<= bits;
    prod |= al;
    prod /= b;
    int64_t high = (prod >> bits);
    if (high > 0) return std::numeric_limits<I>::max();
    if (high < 0) return std::numeric_limits<I>::min();
    return prod;
  }
  template<typename I>
  std::enable_if_t<
    CHAR_BIT * sizeof(I) <= 32 &&
    !std::numeric_limits<I>::is_signed,
    I
  > divOverflow(I ah, std::make_unsigned_t<I> al, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    uint64_t prod = ah;
    prod <<= bits;
    prod |= al;
    prod /= b;
    return ((prod >> bits) != 0) ? prod : std::numeric_limits<I>::max;
  }
#ifdef __SIZEOF_INT128__
  // For types 64 bits or smaller
  template<typename I>
  std::enable_if_t<
    (CHAR_BIT * sizeof(I) > 32) &&
    (CHAR_BIT * sizeof(I) <= 64) &&
    std::numeric_limits<I>::is_signed,
    std::pair<I, std::make_unsigned_t<I>>
  > mulOverflow(I a, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    using U = std::make_unsigned_t<I>;
    __int128_t prod = a;
    prod *= b;
    return {(I) (prod >> bits), (U) prod};
  }
  template<typename I>
  std::enable_if_t<
    (CHAR_BIT * sizeof(I) > 32) &&
    (CHAR_BIT * sizeof(I) <= 64) &&
    !std::numeric_limits<I>::is_signed,
    std::pair<I, std::make_unsigned_t<I>>
  > mulOverflow(I a, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    using U = std::make_unsigned_t<I>;
    __uint128_t prod = a;
    prod *= b;
    return {(I) (prod >> bits), (U) prod};
  }
  template<typename I>
  std::enable_if_t<
    (CHAR_BIT * sizeof(I) > 32) &&
    (CHAR_BIT * sizeof(I) <= 64) &&
    std::numeric_limits<I>::is_signed,
    I
  > divOverflow(I ah, std::make_unsigned_t<I> al, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    __int128_t prod = ah;
    prod <<= bits;
    prod |= al;
    prod /= b;
    int64_t high = (prod >> bits);
    if (high > 0) return std::numeric_limits<I>::max();
    if (high < 0) return std::numeric_limits<I>::min();
  }
  template<typename I>
  std::enable_if_t<
    (CHAR_BIT * sizeof(I) > 32) &&
    (CHAR_BIT * sizeof(I) <= 64) &&
    !std::numeric_limits<I>::is_signed,
    I
  > divOverflow(I ah, std::make_unsigned_t<I> al, I b) {
    constexpr int bits = CHAR_BIT * sizeof(I);
    __uint128_t prod = ah;
    prod <<= bits;
    prod |= al;
    prod /= b;
    return ((prod >> bits) != 0) ? prod : std::numeric_limits<I>::max;
  }
#endif
#undef MAX_NATIVE_BITS
  // I = underlying int type
  // d = number of bits to the right of the decimal
  template<typename I, size_t d>
  struct Fixed {
    using F = kfp::Fixed<I, d>;
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
    constexpr Fixed(const F& value) : underlying(value.underlying) {}
    // Implicit cast from smaller type
    template<typename I2, size_t d2>
    constexpr Fixed(const Fixed<I2, d2>& other) {
      static_assert(integralDigits() >= other.integralDigits(),
        "Cannot implicitly cast into a type with fewer integral digits");
      static_assert(fractionalBits() >= other.fractionalBits(),
        "Cannot implicitly cast into a type with fewer fractional bits");
      // How much left should we shift?
      underlying = other.underlying << (fractionalBits() - other.fractionalBits());
    }
    constexpr static F raw(I underlying) {
      F ret;
      ret.underlying = underlying;
      return ret;
    }
    // Traits
    static constexpr size_t integralBits() {
      return CHAR_BIT * sizeof(I) - d;
    }
    static constexpr size_t integralDigits() {
      return std::numeric_limits<I>::digits - d;
    }
    static constexpr size_t fractionalBits() {
      return d;
    }
    // Operator defines
    F& operator=(const F& other) {
      underlying = other.underlying;
      return *this;
    }
    F& operator=(const I& i) {
      underlying = i << d;
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
    DEF_OP_BOILERPLATE2(*)
    template<size_t d2>
    F& operator*=(const Fixed<I, d2>& other) {
      // TODO: should we support heterogenous types for this?
      auto p = mulOverflow(underlying, other.underlying);
      underlying =
        (p.second >> other.fractionalBits()) |
        (p.first << other.integralBits());
      return *this;
    }
    DEF_OP(/) {
      I ah = underlying >> d;
      std::make_unsigned_t<I> al = underlying;
      al <<= d;
      underlying = divOverflow(ah, al, other.underlying);
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
    // Other functions
    I floor() const {
      return underlying >> d;
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
}

template<typename I, size_t d>
struct std::numeric_limits<kfp::Fixed<I, d>> {
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
    (int) ceil(std::numeric_limits<I>::digits * std::log10(2) + 1);
  static constexpr int radix = 2;
  static constexpr int min_exponent = -d + 1;
  static constexpr int max_exponent =
    std::numeric_limits<I>::digits - d;
  static constexpr int min_exponent10 =
    (int) ((-d + 1) * std::log10(2));
  static constexpr int max_exponent10 =
    (int) ((std::numeric_limits<I>::digits - d) * std::log10(2));
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

#endif // KOZET_FIXED_POINT_KFP_H
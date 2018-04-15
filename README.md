## kozet_fixed_point

A header-only library for fixed-point numbers.

* `kozet_fixed_point/kfp.h` provides the types and the basic functionality.
* `kozet_fixed_point/kfp_extra.h` provides trigonometric functions that work
  on angles represented as 32-bit fractions of a turn.

Uses C++14 features.

### Rationale

It's sometimes necessary to have an alternative to floating-point numbers
that behaves the same across many platforms, regardless of the compiler used
to build a program or the hardware it runs on.

### Running the sample program

If you're on an operating system that isn't brain-damaged, then compiling the
program is as simple as using `make`. Then the executable is `build/test`.

The program will perform several sanity checks, then it will print out a
table like this:

    0 -> (1, 0) -> 0.999985 angle 0
    0.00390625 -> (0.999699, 0.0245412) -> 1.00014 angle 0.00390432
    0.0078125 -> (0.998795, 0.0490677) -> 1.00003 angle 0.00780897
    0.0117188 -> (0.99729, 0.0735646) -> 1.00002 angle 0.0117212
    0.015625 -> (0.995185, 0.0980171) -> 1.00014 angle 0.015618

(angle → (cos, sin) → length and angle). That is, the second-to-last number
of each line should be close to 1, and the last one should be close to the
first one.

Afterwards, the program will test the performance of trigonometric
operations, printing out the average time taken by each iteration of the
loop.

### Using the library

The basic functionality is contained in `kozet_fixed_point/kfp.h`, and you
can use the class `kfp::Fixed<I, d>` where `I` is the underlying integer
type to use and `d` is the number of bits after the decimal point.

The library defines some aliases for convenience:

    using s16_16 = Fixed<int32_t, 16>;
    using u16_16 = Fixed<uint32_t, 16>;
    using s2_30 = Fixed<int32_t, 30>;
    using s34_30 = Fixed<int64_t, 30>;
    using frac32 = Fixed<uint32_t, 32>;

#### Note about `int128_t` and `uint128_t`

In order to use this library, you must have `int128_t` and `uint128_t`
defined. With compilers that support `__int128_t` and `__uint128_t` (such as
GCC), no further action is required. Otherwise, you must define `int128_t`
and `uint128_t` appropriately, using such types provided by your compiler
or from a library.

#### Constructors

(From here on, replace `F` with `decltype(*this)`.)

    Fixed();                           // zero
    Fixed(I value);                    // convert from integer value
    Fixed(const F& value);             // copy
    Fixed(const Fixed<I2, d2>& other); // implicit cast when safe
    static F raw(I underlying);        // create with underlying value

Examples:

    s16_16 zero; // 0
    s16_16 two = 2; // 2
    s16_16 anotherTwo = two; // also 2
    s34_30 biggerTwo = two; // also 2
    // Error: fewer integer and fractional bits
    // kfp::fixed<int16_t, 8> smallerTwo = two;
    s16_16 oneHalf = s16_16::raw(0x8000); // 0.5

For the aliases, you can use the user-defined literals:

    s16_16 knockoffPi = "3.141592"_s16_16;
    frac32 x = "0.3792825"_frac32;

#### Type info

    static size_t integralBits();   // number of integral bits
    static size_t integralDigits(); // number of integral digits (bits - 1)
    static size_t fractionalBits(); // number of fractional bits

In addition, `std::numeric_limits` is specialised for `kfp::Fixed<I, d>`.

#### Assignment

    F& operator=(const F& other);
    F& operator=(const I& i);

#### Arithmetic operations

The operators `+ - * / << >>` are available with their obvious meanings,
as well as their assignment variants. The relations `== != < <= > >=` are
defined as well.

`*` can be applied as long as the underlying integer type is the same between
the left-hand and right-hand sides, even if the number of fractional bits
is different. `>>` and `<<` take integer arguments for the right-hand side.
Other operators can be applied only to the exact same types.

#### Others

    I floor();         // integer part of value
    double toDouble(); // convert to double (e. g. for output)
    // ostream overload (non-member function)
    std::ostream& operator<<(std::ostream& fh, const Fixed<I, d>& x);

#### Trigonometry

The trigonometric functions are defined in `kozet_fixed_point/kfp_extra.h`.

    void sincos(frac32 t, s2_30& c, s2_30& s);

Computes the sine and cosine of an angle `t` (in a frac32 of a turn) at the
same time. These are stored in `s` and `c` respectively.

A 2.30 representation is used in order to properly represent both -1 and 1.

    void rectp(F c, F s, F& r, frac32& t);

Given two fixed-point values `c` and `s`, computes `r = hypot(c, s)` and
`t = atan2(s, c)` at the same time, effectively converting from rectangular
to polar coördinates.

    bool isInterior(F x, F y, F r)

Returns true if the point `(x, y)` is inside the circle centred around the
origin with radius `r`.

#### Licence

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
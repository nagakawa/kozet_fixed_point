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
#ifndef KOZET_FIXED_POINT_KFP_RANDOM_H
#define KOZET_FIXED_POINT_KFP_RANDOM_H

#include "./kfp.h"

#include <iostream>
#include <random>

namespace kfp {
  template<typename T>
  class UniformFixedDistribution {
    /*
      This class satisfies the RandomNumberDistribution
      concept.
    */
  public:
    typedef T result_type;
    struct param_type {
      using distribution_type = UniformFixedDistribution<T>;
      result_type a, b;
    };
    UniformFixedDistribution() : params{0, 1} {}
    UniformFixedDistribution(result_type a, result_type b) :
      params{a, b} {}
    void reset() {}
    param_type param() const { return params; }
    result_type a() const { return params.a; }
    result_type b() const { return params.b; }
    void param(const param_type& p) {
      params = p;
    }
    template<typename R>
    result_type operator()(R& r) {
      return (*this)(r, params);
    }
    template<typename R>
    result_type operator()(R& r, const param_type& p) {
      std::uniform_int_distribution<I> auxdist(
        p.a.underlying, p.b.underlying - 1);
      return result_type::raw(auxdist(r));
    }
    result_type min() const {
      return params.a;
    }
    result_type max() const {
      return result_type::raw(params.b.underlying - 1);
    }
    bool operator==(const UniformFixedDistribution<T>& other) const {
      return
        params.a == other.params.a &&
        params.b == other.params.b;
    }
    bool operator!=(const UniformFixedDistribution<T>& other) const {
      return
        params.a != other.params.a ||
        params.b != other.params.b;
    }
  private:
    using I = typename T::Underlying;
    param_type params;
  };
  template<typename T>
  std::ostream& operator<<(
      std::ostream& fh, UniformFixedDistribution<T> dist) {
    fh << dist.a() << " " << dist.b();
    return fh;
  }
  template<typename T>
  std::istream& operator>>(
      std::istream& fh, UniformFixedDistribution<T> dist) {
    fh >> dist.a() >> dist.b();
    return fh;
  }
}

#endif // KOZET_FIXED_POINT_KFP_RANDOM_H

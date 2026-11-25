// ppl.h - MinGW 用の最小シム (MSVC Parallel Patterns Library の代替)
// ffdshow r4422 で Concurrency の使用は Tconvert.cpp の
// parallel_for_each 1 箇所だけ (実測)。直列実行で意味は保存される。
#pragma once
#include <algorithm>
namespace Concurrency {
    template<class _It, class _Fn>
    void parallel_for_each(_It _first, _It _last, _Fn _fn) {
        std::for_each(_first, _last, _fn);
    }
    template<class _It, class _Fn>
    void parallel_for(_It _first, _It _last, _Fn _fn) {
        std::for_each(_first, _last, _fn);
    }
}

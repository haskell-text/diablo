"""Benchmark of optimized count-eq versus baseline."""
import os
import time
from cffi import FFI  # type: ignore
import pytest

ffi = FFI()

ffi.cdef("""
size_t diablo_count_eq (uint8_t const * const src,
                        size_t const off,
                        size_t const len,
                        uint8_t const byte);
""")

optimized_impl = ffi.dlopen(os.environ['OPTIMIZED_LIB'])

ffi.cdef("""
size_t count_eq_baseline (uint8_t const * const src,
                          size_t const off,
                          size_t const len,
                          uint8_t const byte);
""")

baseline_impl = ffi.dlopen(os.environ['FALLBACK_LIB'])


@pytest.mark.benchmark(group="count-eq", timer=time.process_time)
def test_count_eq_baseline(benchmark):
    """Benchmark the fallback count_eq"""
    src = ffi.new("uint8_t[1048576]")  # one megabyte
    for i in range(1048576):
        src[i] = i % 256

    @benchmark
    def result():
        return baseline_impl.count_eq_baseline(src, 0, 1048576, 0)


@pytest.mark.benchmark(group="count-eq", timer=time.process_time)
def test_count_eq_optimal(benchmark):
    """Benchmark the optimal count_eq"""
    src = ffi.new("uint8_t[1048576]")  # one megabyte
    for i in range(1048576):
        src[i] = i % 256

    @benchmark
    def result():
        return optimized_impl.diablo_count_eq(src, 0, 1048576, 0)

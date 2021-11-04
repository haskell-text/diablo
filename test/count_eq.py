"""Property tests for diablo_count_eq function."""
import weakref
import sys
from cffi import FFI  # type: ignore
from hypothesis import given
from hypothesis.strategies import composite, binary, integers

ffi = FFI()

global_weakkeydict: weakref.WeakKeyDictionary = weakref.WeakKeyDictionary()

ffi.cdef("""
typedef struct {
    uint8_t* src;
    size_t full_len, off, len;
    uint8_t byte;
    } count_eq_data;
""")

ffi.cdef("""
size_t diablo_count_eq (uint8_t const * const src, 
                        size_t const off,
                        size_t const len,
                        uint8_t const byte);
""")

C = ffi.dlopen(sys.argv[1])


@composite
def mk_count_eq_data(draw):
    """Generator for input data appropriate to diablo_count_eq"""
    src = draw(binary(min_size=0, max_size=1000))
    full_len = len(src)
    if full_len == 0:
        off = 0
        length = 0
    else:
        off = draw(integers(min_value=0, max_value=full_len - 1))
        length = draw(integers(min_value=0, max_value=full_len - off))
    byte = draw(integers(min_value=0, max_value=255))
    src_c = ffi.new("uint8_t[]", full_len)
    for i in range(full_len):
        src_c[i] = src[i]
    dat_c = ffi.new("count_eq_data*")
    dat_c.src = src_c
    dat_c.full_len = full_len
    dat_c.off = off
    dat_c.len = length
    dat_c.byte = byte
    global_weakkeydict[dat_c] = src_c
    return dat_c


@given(mk_count_eq_data())
def test_count_eq(dat_c):
    """Tests that diablo_count_eq behaves correctly versus a reference spec."""
    expected_count = 0
    for i in range(dat_c.len):
        if dat_c.src[dat_c.off + i] == dat_c.byte:
            expected_count = expected_count + 1
    actual_count = C.diablo_count_eq(dat_c.src, dat_c.off, dat_c.len,
                                     dat_c.byte)
    assert expected_count == actual_count


if __name__ == "__main__":
    test_count_eq()  # pylint: disable=no-value-for-parameter

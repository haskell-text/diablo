from cffi import FFI

ffi = FFI()

ffi.cdef("""
typedef struct {
    uint8_t* src;
    size_t full_len, off, len;
    uint8_t byte;
    } count_eq_data;
""")

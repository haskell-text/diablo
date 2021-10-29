local ffi = require('ffi')

ffi.cdef[[
typedef struct {
  size_t total_size;
  size_t off;
  size_t len;
  uint8_t * src;
  uint8_t byte
} count_eq_data;
]]

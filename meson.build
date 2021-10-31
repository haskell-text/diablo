project('diablo', 'c',
  license: 'Apache2',
  version: '1.0',
  default_options: ['c_std=c11'])

compiler = meson.get_compiler('c')

assert(compiler.get_id() == 'gcc' or compiler.get_id() == 'clang', 
       'Diablo only supports GCC and Clang.')

asan_args = ['-fsanitize=address']

# https://stackoverflow.com/a/40215639/2629787
if compiler.get_id() == 'clang'
  asan_args += ['-shared-libasan']
endif

srcs = ['src/count-eq.c']

luajit = find_program('luajit', '/usr/bin/luajit')

libs = both_libraries('diablo', srcs,
  c_args: ['-Wall', '-Wextra', '-Wpedantic', '-fsanitize=address'],
  link_args: asan_args)

count_eq_test_file = files('test/count-eq.lua')

test('count_eq', luajit,
  args: [count_eq_test_file],
  depends: libs.get_shared_lib()
  )

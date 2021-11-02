project('diablo', 'c',
  license: 'Apache2',
  version: '1.0',
  default_options: ['c_std=c11'])

compiler = meson.get_compiler('c')

assert(compiler.get_id() == 'gcc' or compiler.get_id() == 'clang', 
       'Diablo only supports GCC and Clang.')

srcs = ['src/count-eq.c']

libs = both_libraries('diablo', srcs,
  c_args: ['-Wall', '-Wextra', '-Wpedantic']
  )

py = import('python').find_installation('python3', modules : ['cffi'])

count_eq_test_file = files('test/count-eq.py')

test('count_eq', py,
  args: [count_eq_test_file],
  depends: libs.get_shared_lib()
  )

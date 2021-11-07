project('diablo', 'c',
  license: 'Apache2',
  version: '1.0',
  default_options: ['c_std=c11', 'warning_level=3'])

# Programs

compiler = meson.get_compiler('c')

assert(compiler.get_id() == 'gcc' or compiler.get_id() == 'clang', 
       'Diablo only supports GCC and Clang.')

py = import('python')

testing_py = py.find_installation('python3',
  modules: ['cffi', 'hypothesis'],
  )

benching_py = py.find_installation('python3',
  modules: ['cffi', 'pytest_benchmark']
  )

pytest = find_program('pytest', '/usr/bin/pytest', '/usr/bin/pytest-3')

# Library

srcs = files('src/count-eq.c')

libs = both_libraries('diablo', srcs)

# Tests

test('count-eq', testing_py,
  args: [files('test/count_eq.py'), libs.get_shared_lib().full_path()],
  depends: libs.get_shared_lib()
  )

# Benchmarks

count_eq_base_srcs = files('bench/count-eq-baseline.c')

count_eq_base_lib = shared_library('count-eq-baseline', count_eq_base_srcs)

run_target('count-eq-bench', 
  command: [pytest, files('bench/count_eq_bench.py')],
  depends: [count_eq_base_lib, libs.get_shared_lib()],
  env: { 'OPTIMIZED_LIB': libs.get_shared_lib().full_path(), 
         'FALLBACK_LIB': count_eq_base_lib.full_path() }
  )

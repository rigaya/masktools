masktools_filters_sources = []
masktools_filters_avx_sources = []
masktools_filters_avx2_sources = []

subdir('binarize')
subdir('blur')
subdir('convolution')
subdir('gradient')
subdir('invert')
subdir('logic')
subdir('lut')
subdir('mask')
subdir('merge')
subdir('morphologic')
subdir('support')

masktools_filters_lib = static_library('masktools_filters',
  masktools_filters_sources,
  include_directories : [include_directories('../../..', '../../../KUtil', '../../../common'), masktools_inc],
  dependencies : [avisynth_dep, threads_dep, masktools_common_dep],
  cpp_args : cpp_args + ['-Wno-reorder'],
)

masktools_filters_avx_lib = static_library('masktools_filters_avx',
  masktools_filters_avx_sources,
  include_directories : [include_directories('../../..', '../../../KUtil', '../../../common'), masktools_inc],
  dependencies : [avisynth_dep, threads_dep, masktools_common_dep],
  cpp_args : cpp_args + avx_args + ['-Wno-reorder'],
)

masktools_filters_avx2_lib = static_library('masktools_filters_avx2',
  masktools_filters_avx2_sources,
  include_directories : [include_directories('../../..', '../../../KUtil', '../../../common'), masktools_inc],
  dependencies : [avisynth_dep, threads_dep, masktools_common_dep],
  cpp_args : cpp_args + avx2_args + ['-Wno-reorder'],
)

masktools_filters_dep = declare_dependency(
  link_with : [masktools_filters_lib, masktools_filters_avx_lib, masktools_filters_avx2_lib],
  include_directories : include_directories('.')
) 
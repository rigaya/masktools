masktools_common_sources = [
  'clip/inputconfig.cpp'
]

masktools_common_headers = [
  '16bit.h',
  'common.h',
  'mt_resource.h',
  'simd.h',
  'base/filter.h',
  'clip/inputconfig.h',
  'params/params.h',
]

masktools_common_lib = static_library('masktools_common',
  masktools_common_sources,
  include_directories : [include_directories('../../..', '../../../KUtil', '../../../common'), masktools_inc],
  dependencies : [avisynth_dep, threads_dep],
  cpp_args : cpp_args + ['-Wno-reorder'],
)

masktools_common_dep = declare_dependency(
  link_with : masktools_common_lib,
  include_directories : include_directories('.')
) 
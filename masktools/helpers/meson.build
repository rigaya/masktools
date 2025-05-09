masktools_helpers_sources = [
  'CudaHelper.cpp',
  'avs2x/helpers_avs2x.cpp',
  'forms/forms.cpp',
  'parser/spirit.cpp'
]

masktools_helpers_lib = static_library('masktools_helpers',
  masktools_helpers_sources,
  include_directories : [include_directories('../../..', '../../../KUtil', '../../../common'), masktools_inc],
  dependencies : [avisynth_dep, cuda_dep, threads_dep, masktools_common_dep],
  cpp_args : cpp_args + ['-Wno-reorder'],
)

masktools_helpers_dep = declare_dependency(
  link_with : masktools_helpers_lib,
  include_directories : include_directories('.')
) 
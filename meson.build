masktools_inc = include_directories('masktools', 'masktools/common', 'masktools/filters', 'masktools/helpers')

subdir('common')
subdir('masktools/common')
subdir('masktools/filters')
subdir('masktools/helpers')

masktools_sources = [
  'masktools/avs2x/wrapper.cpp',
]

shared_module('mt_masktools',
  masktools_sources,
  include_directories : [include_directories('..', '../KUtil', '../common'), masktools_inc],
  dependencies : [avisynth_dep, cuda_dep, threads_dep, masktools_common_functions_dep, masktools_common_dep, masktools_filters_dep, masktools_helpers_dep],
  cpp_args : cpp_args,
  link_args : link_args,
  name_prefix : '',
  install : true,
  install_dir : get_option('libdir')
) 
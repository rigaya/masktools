masktools_common_functions_sources = [
  'functions/functions.cpp',
  'functions/functions_cuda.cu',
  'parser/parser.cpp',
  'parser/symbol.cpp',
  'constraints/constraints.cpp'
]

masktools_common_functions_headers = [
  'frame/frame.h',
  'clip/clip.h',
  'functions/functions.h',
  'params/params.h',
  'parser/parser.h',
  'parser/symbol.h',
  'plane/plane.h',
  'utils/utils.h',
  'constraints/constraints.h'
]

# 静的ライブラリとして構築
masktools_common_functions_lib = static_library('masktools_common',
  masktools_common_functions_sources,
  include_directories : [include_directories('../..', '../../KUtil', '../../common')],
  dependencies : [avisynth_dep, cuda_dep, threads_dep],
  cpp_args : cpp_args,
  cuda_args : cuda_args,
)

# 依存関係を宣言
masktools_common_functions_dep = declare_dependency(
  link_with : masktools_common_functions_lib,
  include_directories : include_directories('.')
) 
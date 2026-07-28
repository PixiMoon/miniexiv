project('miniexiv', 'c', 'cpp',
  version : '0.6.12',
  meson_version : '>=0.63.0',
  default_options : [
    'c_std=c99',
    'cpp_std=c++17',
    'warning_level=2',
    'default_library=both',
    'cpp_rtti=true'
  ]
)

fs = import('fs')
pkg = import('pkgconfig')

# 1. Версионирование
project_ver = meson.project_version()

git = find_program('git', required : false)
if git.found() and fs.exists('.git')
  git_res = run_command(git, 'describe', '--tags', '--match', 'v[0-9]*.[0-9]*.[0-9]*', '--always', check : false)
  if git_res.returncode() == 0
    raw_version = git_res.stdout().strip()
    if raw_version.startswith('v')
      project_ver = raw_version.substring(1)
    else
      project_ver = raw_version
    endif
  endif
endif

ver_clean = project_ver.split('-')[0]
ver_parts = ver_clean.split('.')

ver_major = ver_parts.length() > 0 ? ver_parts[0].to_int() : 0
ver_minor = ver_parts.length() > 1 ? ver_parts[1].to_int() : 0
ver_patch = ver_parts.length() > 2 ? ver_parts[2].to_int() : 0

# 2. Флаги видимости
cpp_comp = meson.get_compiler('cpp')
add_project_arguments(
  cpp_comp.get_supported_arguments([
    '-fvisibility=hidden',
    '-fvisibility-inlines-hidden'
  ]),
  language : ['c', 'cpp']
)

# 3. Зависимость Exiv2 с принудительным fallback при включенной опции


build_deps = get_option('build_dependencies')

if build_deps
  exiv2_dep = dependency('exiv2', static:true, fallback : ['exiv2', 'exiv2lib_dep'])
else
  exiv2_dep = dependency('exiv2', required : true)
endif

inc = include_directories('include')
inc_private = include_directories('src')

# 4. Генерация version.cpp
cdata = configuration_data()
cdata.set('PROJECT_VERSION', project_ver)
cdata.set('PROJECT_VERSION_MAJOR', ver_major)
cdata.set('PROJECT_VERSION_MINOR', ver_minor)
cdata.set('PROJECT_VERSION_PATCH', ver_patch)

version_cpp = configure_file(
  input : 'src/version.cpp.in',
  output : 'version.cpp',
  configuration : cdata
)

sources = files(
  'src/miniexiv.cpp',
  'src/miniexiv_errors.cpp'
) + [version_cpp]

# 5. Сборка таргета
miniexiv_lib = library('miniexiv',
  sources,
  include_directories : [inc, inc_private],
  dependencies : [exiv2_dep],
  install : true,
  version : ver_clean,
  soversion : ver_major.to_string()
)

miniexiv_dep = declare_dependency(
  include_directories : inc,
  link_with : miniexiv_lib
)

install_subdir('include/miniexiv', install_dir : get_option('includedir'))

install_data([
  'README.md',
  'LICENSE'
], install_dir : get_option('datadir') / 'doc' / 'miniexiv')

pkg.generate(miniexiv_lib,
  name : 'miniexiv',
  filebase : 'miniexiv',
  description : 'Lightweight C wrapper around the Exiv2 metadata library',
  subdirs : 'miniexiv'
)
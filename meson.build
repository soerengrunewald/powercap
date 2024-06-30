# SPDX-License-Identifier: LGPL-2.1-or-later
project('powercap', 'cpp',
   version : '1.0',
   license: 'GPL2+',
   default_options : [
      'cpp_std=c++17',
      'warning_level=3',
   ],
   meson_version : '>=1.0'
)

subdir('data')

executable(meson.project_name(), 'main.cc', install : true)

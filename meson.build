project('shh-portable-utils', ['c'], version: '0.0.0.1', default_options: ['c_std=c99'])

cc = meson.get_compiler('c')

libskarnet = [cc.find_library('skarnet', required: true)]

subdir('src')

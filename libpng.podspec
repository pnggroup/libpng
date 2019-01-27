Pod::Spec.new do |spec|
  spec.name = 'libpng'
  spec.version = '1.6.35'
  spec.license = { 
    :type => 'Open Source',
    :file => 'LICENSE'
  }
  spec.homepage = 'http://www.libpng.org/'
  spec.summary = 'The free reference library for reading and writing PNGs'
  spec.authors = { 
    'Alexey Komnin' => 'interfere.work@gmail.com' 
  }
  spec.source = { 
    :git => 'https://github.com/glennrp/libpng.git',
    :tag => 'v1.6.35'
  }
  spec.platforms = { 
    :ios => '7.0',
    :osx => '10.7'
  }
  spec.requires_arc = false
  spec.prepare_command = 'cp -v scripts/pnglibconf.h.prebuilt ./pnglibconf.h'
  spec.libraries = 'z'
  spec.public_header_files = '*.h'
  spec.private_header_files = 'pngpriv.h', 'pnginfo.h'
  spec.source_files = '*.{c,h}', 'arm/*.{c,S}', 'intel/*.c'
  spec.exclude_files = 'example.c', 'pngpread.c', 'pngtest.c'
  spec.compiler_flags = '-DPNG_NO_CONFIG_H',
                        '-DHAVE_DLFCN_H=1',
                        '-DHAVE_INTTYPES_H=1',
                        '-DHAVE_LIBZ=1',
                        '-DHAVE_MEMORY_H=1',
                        '-DHAVE_MEMSET=1',
                        '-DHAVE_POW=1',
                        '-DHAVE_STDINT_H=1',
                        '-DHAVE_STDLIB_H=1',
                        '-DHAVE_STRINGS_H=1',
                        '-DHAVE_STRING_H=1',
                        '-DHAVE_SYS_STAT_H=1',
                        '-DHAVE_SYS_TYPES_H=1',
                        '-DHAVE_UNISTD_H=1',
                        '-DSTDC_HEADERS=1',
                        '-Drestrict=__restrict'
end

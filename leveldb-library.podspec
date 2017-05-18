Pod::Spec.new do |s|
  s.name             = 'leveldb-library'
  s.version          = '1.18.3'
  s.author           = 'The LevelDB Authors'
  s.license          = { :type => 'Apache', :file => 'LICENSE' }
  s.homepage         = 'https://github.com/google/leveldb'
  s.source           = { :git => 'https://github.com/matehat/leveldb.git',
                         :tag => "v1.18.1" }
  s.summary          = 'A fast key-value storage library'
  s.description      = <<-DESC
      LevelDB is a fast key-value storage library written at Google that provides an ordered mapping from string keys to string values.
                       DESC

  s.osx.deployment_target = '10.7'
  s.ios.deployment_target = '5.0'
  s.tvos.deployment_target = '9.0'

  s.requires_arc = false

  s.source_files =
      'db/*.{cc}',
      'port/*.{cc}',
      'table/*.{cc}',
      'port/*.{cc}',
      'util/*.{cc}',
      'include/leveldb/*.h'

  s.compiler_flags =
      '-DOS_MACOSX',
      '-DLEVELDB_PLATFORM_POSIX'

  s.preserve_paths =
      'db',
      'port',
      'table',
      'util'

  s.exclude_files =
          '**/*_test.cc',
          '**/*_bench.cc',
          'port/win'

  s.xcconfig = { 'HEADER_SEARCH_PATHS' => '$(inherited) "${PODS_ROOT}/Firebase/Firebase/Firebase"',
                 "CC" => "clang",
                 "CXX" => "clang++",
                 "HEADER_SEARCH_PATHS" => "\"${PODS_ROOT}/leveldb-library/\" \"${PODS_ROOT}/leveldb-library/include\"",
                 "WARNING_CFLAGS" => "-Wno-shorten-64-to-32",
                 "OTHER_LDFLAGS" => "-lc++"
               }

end

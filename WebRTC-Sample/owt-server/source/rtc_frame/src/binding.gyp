{
  'variables': {
    'owt_root': '../../../third_party/owt-server',
    'owt_root_abs': '<!(pwd)/<(owt_root)',
    'im_deps': '../../../build/libdeps/build',
    'im_deps_abs': '<!(pwd)/<(im_deps)',
    'libwebrtc360': '../../libwebrtc360',
  },
  'targets': [{
    'target_name': 'rtcFrame',
    'sources': [
      '<(owt_root)/source/core/owt_base/AudioFrameConstructor.cpp',
      '<(owt_root)/source/core/owt_base/AudioFramePacketizer.cpp',
      '<(owt_root)/source/core/owt_base/VideoFrameConstructor.cpp',
      '<(owt_root)/source/core/owt_base/MediaFramePipeline.cpp',
      '<(owt_root)/source/core/common/JobTimer.cpp',
      '<(owt_root)/source/agent/webrtc/rtcFrame/AudioFrameConstructorWrapper.cc',
      '<(owt_root)/source/agent/webrtc/rtcFrame/AudioFramePacketizerWrapper.cc',
      '<(owt_root)/source/agent/webrtc/rtcFrame/VideoFrameConstructorWrapper.cc',
      '<(owt_root)/source/agent/webrtc/rtcFrame/VideoFramePacketizerWrapper.cc',
      '<(owt_root)/source/agent/webrtc/rtcFrame/addon.cc',
      '<(libwebrtc360)/src/WebRTC360HEVCTilesMerger.cpp',
      'VideoFramePacketizer.cpp',
    ],
    'dependencies': ['librtcadapter'],
    'cflags_cc': [
      '-DWEBRTC_POSIX',
      '-DWEBRTC_LINUX',
      '-DLINUX',
      '-DNOLINUXIF',
      '-DNO_REG_RPC=1',
      '-DHAVE_VFPRINTF=1',
      '-DRETSIGTYPE=void',
      '-DNEW_STDIO',
      '-DHAVE_STRDUP=1',
      '-DHAVE_STRLCPY=1',
      '-DHAVE_LIBM=1',
      '-DHAVE_SYS_TIME_H=1',
      '-DTIME_WITH_SYS_TIME_H=1',
      '-DOWT_ENABLE_H265',
      '-D_LIBCPP_ABI_UNSTABLE',
      '-D_ENABLE_HEVC_TILES_MERGER_',
    ],
    'include_dirs': [
      './',
      '../../common',
      '<(owt_root)/node_modules/nan',
      '<(owt_root)/source/agent/webrtc/rtcConn/erizo/src/erizo',
      '<(owt_root)/source/core/common',
      '<(owt_root)/source/core/owt_base',
      '<(owt_root)/source/core/rtc_adapter',
      '<(owt_root)/build/libdeps/build/include',
      '<!@(pkg-config glib-2.0 --cflags-only-I | sed s/-I//g)',
      '<(im_deps_abs)/include',
    ],
    'libraries': [
      '-L<(owt_root_abs)/build/libdeps/build/lib',
      '-lsrtp2',
      '-lssl',
      '-ldl',
      '-lcrypto',
      '-llog4cxx',
      '-lboost_thread',
      '-lboost_system',
      '-lnice',
      '-Wl,-rpath,<!(pwd)/build/$(BUILDTYPE)', # librtcadapter
      '-L<(im_deps_abs)/lib', '-l360SCVP',
    ],
    'conditions': [
      [ 'OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',        # -fno-exceptions
            'MACOSX_DEPLOYMENT_TARGET':  '10.7',       # from MAC OS 10.7
            'OTHER_CFLAGS': ['-g -O$(OPTIMIZATION_LEVEL) -stdlib=libc++']
          },
        },
        { # OS!="mac"
          'cflags!' : ['-fno-exceptions'],
          'cflags_cc':  ['-Wall', '-O3', '-g' , '-std=c++11'],
          'cflags_cc!': ['-fno-exceptions']
        }
      ],
    ]
  },
  {
    'target_name': 'librtcadapter',
    'type': 'shared_library',
    'sources': [
      'VideoSendAdapter.cc',
      '<(owt_root)/source/core/rtc_adapter/RtcAdapter.cc',
      '<(owt_root)/source/core/rtc_adapter/VideoReceiveAdapter.cc',
      '<(owt_root)/source/core/rtc_adapter/AudioSendAdapter.cc',
      '<(owt_root)/source/core/rtc_adapter/thread/StaticTaskQueueFactory.cc',
      '<(owt_root)/source/core/owt_base/SsrcGenerator.cc',
      '<(owt_root)/source/core/owt_base/AudioUtilitiesNew.cpp',
      '<(owt_root)/source/core/owt_base/TaskRunnerPool.cpp',
    ],
    'cflags_cc': [
      '-DWEBRTC_POSIX',
      '-DWEBRTC_LINUX',
      '-DLINUX',
      '-DNOLINUXIF',
      '-DNO_REG_RPC=1',
      '-DHAVE_VFPRINTF=1',
      '-DRETSIGTYPE=void',
      '-DNEW_STDIO',
      '-DHAVE_STRDUP=1',
      '-DHAVE_STRLCPY=1',
      '-DHAVE_LIBM=1',
      '-DHAVE_SYS_TIME_H=1',
      '-DTIME_WITH_SYS_TIME_H=1',
      '-DOWT_ENABLE_H265',
      '-D_LIBCPP_ABI_UNSTABLE',
      '-DNDEBUG',
      '-D_ENABLE_HEVC_TILES_MERGER_',
    ],
    'include_dirs': [
      './',
      '<(im_deps_abs)/include',
      '<(owt_root)/node_modules/nan',
      '<(owt_root)/source/core/common',
      '<(owt_root)/source/core/owt_base',
      '<(owt_root)/source/core/rtc_adapter',
      '<(owt_root)/third_party/webrtc-m79/src', # webrtc include files
      '<(owt_root)/third_party/webrtc-m79/src/third_party/abseil-cpp', # abseil-cpp include files used by webrtc
      '<(owt_root)/build/libdeps/build/include',
      '<!@(pkg-config glib-2.0 --cflags-only-I | sed s/-I//g)',
    ],
    'libraries': [
      '-L<(owt_root_abs)/build/libdeps/build/lib',
      '-lsrtp2',
      '-lssl',
      '-ldl',
      '-lcrypto',
      '-llog4cxx',
      '-lboost_thread',
      '-lboost_system',
      '-lnice',
      '-L<(owt_root_abs)/third_party/webrtc-m79', '-lwebrtc',
    ],
    'conditions': [
      [ 'OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',        # -fno-exceptions
            'MACOSX_DEPLOYMENT_TARGET':  '10.7',       # from MAC OS 10.7
            'OTHER_CFLAGS': ['-g -O$(OPTIMIZATION_LEVEL) -stdlib=libc++']
          },
      }, { # OS!="mac"
        'cflags!' : ['-fno-exceptions'],
        'cflags' : ['-D__STDC_CONSTANT_MACROS'],
        'cflags_cc' : [
          '-Wall', '-O3', '-g' , '-std=gnu++14', '-fexceptions',
          '-nostdinc++',
          '-isystem<(owt_root_abs)/third_party/webrtc-m79/src/buildtools/third_party/libc++/trunk/include',
          '-isystem<(owt_root_abs)/third_party/webrtc-m79/src/buildtools/third_party/libc++abi/trunk/include'
        ]
      }]
    ]
  }]
}

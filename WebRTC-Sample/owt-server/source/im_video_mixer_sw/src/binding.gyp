{
  'variables': {
    'owt_root': '../../../third_party/owt-server',
    'owt_root_abs': '<!(pwd)/<(owt_root)',
    'im_deps': '../../../build/libdeps/build',
    'im_deps_abs': '<!(pwd)/<(im_deps)',
  },
  'targets': [{
    'target_name': 'IMVideoMixer-sw',
    'sources': [
      '<(owt_root)/source/agent/video/videoMixer/addon.cc',
      '<(owt_root)/source/agent/video/videoMixer/VideoMixerWrapper.cc',
      '<(owt_root)/source/core/owt_base/I420BufferManager.cpp',
      '<(owt_root)/source/core/owt_base/MediaFramePipeline.cpp',
      '<(owt_root)/source/core/owt_base/FrameConverter.cpp',
      '<(owt_root)/source/core/owt_base/VCMFrameDecoder.cpp',
      '<(owt_root)/source/core/owt_base/VCMFrameEncoder.cpp',
      '<(owt_root)/source/core/owt_base/FFmpegDrawText.cpp',
      '<(owt_root)/source/core/common/JobTimer.cpp',
      'IMVideoMixer.cpp',
      'IMVideoCompositor.cpp',
      '../../common/IMSVTHEVCEncoder.cpp',
      '../../common/IMSVTHEVCEncoderBase.cpp',
      '../../common/IMSVTHEVCMCTSEncoder.cpp',
      '../../common/IMFFmpegFrameDecoder.cpp',
    ],
    'cflags_cc': [
      '-Wall',
      '-O$(OPTIMIZATION_LEVEL)',
      '-g',
      '-std=c++11',
      '-DWEBRTC_POSIX',
      '-DENABLE_SVT_HEVC_ENCODER',
    ],
    'cflags_cc!': [
      '-fno-exceptions',
    ],
    'include_dirs': [
      '.',
      '../../common',
      '<(im_deps)/include',
      '<(im_deps)/include/svt-hevc',
      '<(owt_root)/source/agent/video/videoMixer',
      '<(owt_root)/source/core/common',
      '<(owt_root)/source/core/owt_base',
      '<(owt_root)/third_party/webrtc/src',
      '<(owt_root)/third_party/webrtc/src/third_party/libyuv/include',
      '<(owt_root)/build/libdeps/build/include',
    ],
    'libraries': [
      '-L<(im_deps_abs)/lib', '-lSvtHevcEnc',
      '-L<(im_deps_abs)/lib', '-lyaml-cpp',
      '-lboost_thread',
      '-llog4cxx',
      '-L<(owt_root_abs)/third_party/webrtc', '-lwebrtc',
      '-L<(owt_root_abs)/third_party/openh264', '-lopenh264',
      '<!@(pkg-config --libs libavutil)',
      '<!@(pkg-config --libs libavcodec)',
      '<!@(pkg-config --libs libavformat)',
      '<!@(pkg-config --libs libavfilter)',
    ],
  }]
}

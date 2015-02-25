# Copyright 2011 The LibYuv Project Authors. All rights reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS. All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'variables': {
    'libyuv_disable_jpeg%': 0,
    'libyuv_enable_svn%': 0,
  },
  'targets': [
    {
      'target_name': 'libyuv_unittest',
      'type': 'executable',
      'dependencies': [
        'libyuv.gyp:libyuv',
        # The tests are based on gtest
        'testing/gtest.gyp:gtest',
        'testing/gtest.gyp:gtest_main',
      ],
      'defines': [
        # Enable the following 3 macros to turn off assembly for specified CPU.
        # 'LIBYUV_DISABLE_X86',
        # 'LIBYUV_DISABLE_NEON',
        # 'LIBYUV_DISABLE_MIPS',
        # Enable the following macro to build libyuv as a shared library (dll).
        # 'LIBYUV_USING_SHARED_LIBRARY',
      ],
      'sources': [
        # headers
        'unit_test/unit_test.h',

        # sources
        'unit_test/basictypes_test.cc',
        'unit_test/compare_test.cc',
        'unit_test/color_test.cc',
        'unit_test/convert_test.cc',
        'unit_test/cpu_test.cc',
        'unit_test/math_test.cc',
        'unit_test/planar_test.cc',
        'unit_test/rotate_argb_test.cc',
        'unit_test/rotate_test.cc',
        'unit_test/scale_argb_test.cc',
        'unit_test/scale_test.cc',
        'unit_test/unit_test.cc',
        'unit_test/video_common_test.cc',
        'unit_test/version_test.cc',
      ],
      'conditions': [
        [ 'libyuv_enable_svn == 1', {
          'defines': [
            'LIBYUV_SVNREVISION="<!(svnversion -n)"',
          ],
        }],
        ['OS=="linux"', {
          'cflags': [
            '-fexceptions',
          ],
        }],
        [ 'OS == "ios" and target_subarch == 64', {
          'defines': [
            'LIBYUV_DISABLE_NEON'
          ],
        }],
        [ 'OS == "ios"', {
          'xcode_settings': {
            'DEBUGGING_SYMBOLS': 'YES',
            'DEBUG_INFORMATION_FORMAT' : 'dwarf-with-dsym',
          },
        }],
        [ 'OS != "ios" and libyuv_disable_jpeg != 1', {
          'defines': [
            'HAVE_JPEG',
          ],
        }],
        # TODO(YangZhang): These lines can be removed when high accuracy
        # YUV to RGB to Neon is ported.
        [ '(target_arch == "armv7" or target_arch == "armv7s" \
          or (target_arch == "arm" and arm_version >= 7) \
          or target_arch == "arm64") \
          and (arm_neon == 1 or arm_neon_optional == 1)', {
          'defines': [
            'LIBYUV_NEON'
          ],
       }],
      ], # conditions
    },

    {
      'target_name': 'compare',
      'type': 'executable',
      'dependencies': [
        'libyuv.gyp:libyuv',
      ],
      'sources': [
        # sources
        'util/compare.cc',
      ],
      'conditions': [
        ['OS=="linux"', {
          'cflags': [
            '-fexceptions',
          ],
        }],
      ], # conditions
    },
    {
      'target_name': 'convert',
      'type': 'executable',
      'dependencies': [
        'libyuv.gyp:libyuv',
      ],
      'sources': [
        # sources
        'util/convert.cc',
      ],
      'conditions': [
        ['OS=="linux"', {
          'cflags': [
            '-fexceptions',
          ],
        }],
      ], # conditions
    },
    # TODO(fbarchard): Enable SSE2 and OpenMP for better performance.
    {
      'target_name': 'psnr',
      'type': 'executable',
      'sources': [
        # sources
        'util/psnr_main.cc',
        'util/psnr.cc',
        'util/ssim.cc',
      ],
      'dependencies': [
        'libyuv.gyp:libyuv',
      ],
      'conditions': [
        [ 'OS == "ios" and target_subarch == 64', {
          'defines': [
            'LIBYUV_DISABLE_NEON'
          ],
        }],

        [ 'OS != "ios" and libyuv_disable_jpeg != 1', {
          'defines': [
            'HAVE_JPEG',
          ],
        }],
      ], # conditions
    },

    {
      'target_name': 'cpuid',
      'type': 'executable',
      'sources': [
        # sources
        'util/cpuid.c',
      ],
      'dependencies': [
        'libyuv.gyp:libyuv',
      ],
    },
  ], # targets
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:

#!/usr/bin/evn python
# encoding: utf-8
# Copyright (C) 2019 Michael Fisher <mfisher@kushview.net>

import sys, os, platform
from subprocess import call
from waflib.extras import autowaf as autowaf
from waflib.extras import juce as juce

APPNAME = 'jlv2'
VERSION = '0.1.0'
top = '.'
out = 'build'

def options (opts):
    autowaf.set_options (opts)
    opts.load ('compiler_c compiler_cxx autowaf juce')

def silence_warnings (conf):
    # TODO: update LV2 module to use latest LV2 / LILV / SUIL
    conf.env.append_unique ('CXXFLAGS', ['-Wno-deprecated-declarations'])

def configure (conf):
    conf.load ('compiler_c compiler_cxx autowaf')
    silence_warnings (conf)

    conf.env.DEBUG = conf.options.debug

    # Write out the version header
    conf.define ("JLV2_VERSION_STRING", VERSION)
    conf.write_config_header ('jlv2/version.h', 'JLV2_VERSION_H')
    
    for jmod in [ 'juce_audio_processors', 'juce_data_structures', 
                  'juce_audio_devices', 'juce_audio_utils',
                  'juce_gui_extra' ]:
        pkgname = '%s_debug-5' % jmod if conf.options.debug else '%s-5' % jmod
        conf.check_cfg (package=pkgname, uselib_store=jmod.upper(), 
                        args=['--libs', '--cflags'], mandatory=True)

    conf.check_cfg (package='lv2', uselib_store='LV2',  
                    args=['--libs', '--cflags'], mandatory=True)
    conf.check_cfg (package='lilv-0', uselib_store='LILV', 
                    args=['--libs', '--cflags'], mandatory=True)
    conf.check_cfg (package='suil-0', uselib_store='SUIL', 
                    args=['--libs', '--cflags'], mandatory=True)
    
    conf.define('JUCE_MODULE_AVAILABLE_jlv2_host', True)
    conf.write_config_header ('jlv2/config.h', 'JLV2_MODULES_CONFIG_H')
    
    conf.load ('juce')
    conf.check_cxx_version ('c++14', True)
    conf.env.append_unique ('CFLAGS', ['-fvisibility=hidden'])
    conf.env.append_unique ('CXXFLAGS', ['-fvisibility=hidden'])

    print
    juce.display_header ("JLV2")
    juce.display_msg (conf, 'Version',  VERSION)
    juce.display_msg (conf, 'Debug',    conf.env.DEBUG)

    if juce.is_mac():
        print
        juce.display_header ('Mac Options')
        juce.display_msg (conf, 'OSX Arch', conf.env.ARCH)
        juce.display_msg (conf, 'OSX Min Version', conf.options.mac_version_min if len(conf.options.mac_version_min) else 'default')
        juce.display_msg (conf, 'OSX SDK', conf.options.mac_sdk if len(conf.options.mac_sdk) else 'default')
    
    print
    juce.display_header ('Compiler')
    juce.display_msg (conf, 'CFLAGS',   conf.env.CFLAGS)
    juce.display_msg (conf, 'CXXFLAGS', conf.env.CXXFLAGS)
    juce.display_msg (conf, 'LDFLAGS',  conf.env.LINKFLAGS)

def library_slug (bld):
    return 'jlv2_debug-0' if bld.env.DEBUG else 'jlv2-0'

def get_include_path (bld, subpath=''):
    ip = '%s/jlv2-%s' % (bld.env.INCLUDEDIR, '0')
    ip = os.path.join (ip, subpath) if len(subpath) > 0 else ip
    return ip

def install_misc_header (bld, header, subpath=''):
    destination = get_include_path (bld, subpath)
    bld.install_files (destination, header)

def maybe_install_headers (bld):
    for header in [ 'build/jlv2/jlv2.h', 'build/jlv2/config.h', 'build/jlv2/version.h' ]:
        install_misc_header (bld, header, 'jlv2')
    bld.install_files (get_include_path (bld), 
                       bld.path.ant_glob ("modules/**/*.*"),
                       relative_trick=True, 
                       cwd=bld.path.find_dir ('modules'))
    
def build (bld):
    bld (
        features = 'subst',
        source = [ 'jlv2.h.in' ],
        target = 'jlv2/jlv2.h',
        install_path = None,
        name = 'JLV2_HEADER'
    )
    
    bld.add_group()
    
    library = bld (
        features    = 'cxx cxxshlib',
        source      = [ 'src/include_jlv2_host.cpp' ],
        includes    = [ 'build', 'modules' ],
        name        = 'JLV2',
        target      = 'lib/%s' % library_slug (bld),
        use         = [ 'JLV2_HEADER', 'JUCE_AUDIO_PROCESSORS', 
                        'JUCE_DATA_STRUCTURES', 'JUCE_GUI_EXTRA',
                        'LILV', 'SUIL' ],
        vnum        = VERSION
    )

    pcobj = bld (
        features      = 'subst',
        source        = 'jlv2.pc.in',
        target        = '%s.pc' % library_slug (bld),
        install_path  = bld.env.LIBDIR + '/pkgconfig',
        env           = bld.env.derive(),
        MAJOR_VERSION = '0',
        PREFIX        = bld.env.PREFIX,
        INCLUDEDIR    = bld.env.INCLUDEDIR,
        LIBDIR        = bld.env.LIBDIR,
        CFLAGSS       = [],
        DEPLIBS       = '-l%s' % library_slug (bld),
        REQUIRED      = '',
        VERSION       = VERSION
    )

    pcobj.env.CXXFLAGS = pcobj.env.CFLAGS = [] #
    pcobj.REQUIRED += 'juce_audio_processors_debug-5 ' if bld.env.DEBUG else 'juce_audio_processors-5 '
    pcobj.REQUIRED += 'juce_data_structures_debug-5 ' if bld.env.DEBUG else 'juce_data_structures-5 '
    if bld.env.HAVE_SUIL: pcobj.REQUIRED += 'suil-0 '
    if bld.env.HAVE_LILV: pcobj.REQUIRED += 'lilv-0 '
    
    lv2show = bld.program (
        source          = [ 'tools/lv2show.cpp' ],
        includes        = [ 'modules' ],
        target          = 'bin/lv2show',
        use             = [ 'JLV2', 'JUCE_AUDIO_UTILS', 'JUCE_AUDIO_DEVICES' ],
        install_path    = None
    )

    maybe_install_headers (bld)

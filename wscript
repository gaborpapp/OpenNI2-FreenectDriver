#!/usr/bin/env python
# encoding: utf-8

import platform


APPNAME = 'FreenectDriver'
VERSION = '1.1'

top = '.'
out = 'build'


def options(opt):
	opt.load('compiler_cxx')
	
def configure(conf):
	conf.env.CXXFLAGS = ['-std=c++0x', '-O2']
	conf.load('compiler_cxx')
	conf.check_cxx(lib='freenect', uselib_store='freenect')
	
	if platform.system() == 'Darwin':
		conf.env.CXX = ['clang++'] # can remove if OSX has >= gcc-4.6

def build(bld):
	bld.shlib(
		target = APPNAME,
		name = APPNAME,
		vnum = VERSION,
		install_path = None,
		includes = ['extern/OpenNI-Linux-x64-2.2/Include', '/usr/include/libfreenect', '/usr/local/include/libfreenect'],
		source = bld.path.ant_glob('src/*.cpp'),
		
		use = 'freenect',
	)

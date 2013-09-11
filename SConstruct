TARGET = 'FreenectDriver'

import os

OPENNI2_INCLUDE = os.environ['OPENNI2_INCLUDE']

SOURCES = ['ColorStream.cpp', 'DepthStream.cpp', 'InfraRedStream.cpp', 'DeviceDriver.cpp']
SOURCES = ['src/' + s for s in SOURCES]
INCLUDES = [OPENNI2_INCLUDE, 'src', '/usr/local/include',
		'/usr/local/include/libfreenect', '/opt/local/include']
CCFLAGS = '-std=c++0x -arch i386 -arch x86_64 -mmacosx-version-min=10.7'
LINKFLAGS = '-arch i386 -arch x86_64'
LIBS = [File('/usr/local/lib/libfreenect.a'), File('/opt/local/lib/libusb-1.0.a'), 'objc']
LIBPATH = ['/usr/lib', '/usr/local/lib']

env = Environment()
env['CXX'] = 'clang++'
env.Append(CPPPATH = INCLUDES)
env.Append(CCFLAGS = Split(CCFLAGS))
env.Append(LINKFLAGS = Split(LINKFLAGS))
env.Append(LIBS = LIBS)
env.Append(LIBPATH = LIBPATH)
if env['PLATFORM'] == 'darwin':
	env.Append(FRAMEWORKS = ['IOKit', 'CoreFoundation'])

env.SharedLibrary(TARGET, SOURCES)


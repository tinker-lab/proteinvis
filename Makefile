#
# Copyright Regents of the University of Minnesota, 2014.  This software is released under the following license: http://opensource.org/licenses/lgpl-3.0.html.
# Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
#
# Code author(s):
# 		Dan Orban (dtorban)
#

#.SILENT:

ifneq ($(WINDIR),)
  ARCH=WIN32
  ifeq ($(shell uname -o), Cygwin)
    WINCURDIR = $(shell cygpath -wm $(CURDIR))
  else
    WINCURDIR = $(CURDIR)
  endif
else
  UNAME=$(shell uname)
  ifeq ($(UNAME), Linux)
    ARCH=linux
    NPROCS = $(shell nproc)
  else ifeq ($(UNAME), Darwin)
    ARCH=OSX
  else
    $(error The Makefile does not recognize the architecture: $(UNAME))
  endif
endif

DIRNAME=$(shell basename `pwd`)

all: gen

gen:
	mkdir -p ./build
   ifeq ($(ARCH), linux)
	mkdir -p ./build/Release
	mkdir -p ./build/Debug
	mkdir -p ../$(DIRNAME)_eclipse
	cd ./build/Release; cmake -DCMAKE_BUILD_TYPE=Release ../../
	cd ./build/Debug; cmake -DCMAKE_BUILD_TYPE=Debug ../../
	cd ../$(DIRNAME)_eclipse; cmake -DCMAKE_BUILD_TYPE=Debug $(CURDIR) -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_VERSION=4.3
    else ifeq ($(ARCH), WIN32)
	cd ./build; cmake ../ -G "Visual Studio 11 Win64"
    else ifeq ($(ARCH), OSX)
	cd ./build; cmake ../ -G Xcode
    endif

use_boost:
	mkdir -p ./build
   ifeq ($(ARCH), linux)
	mkdir -p ./build/Release
	mkdir -p ./build/Debug
	mkdir -p ../$(DIRNAME)_eclipse
	cd ./build/Release; cmake -DUSE_BOOST=ON -DCMAKE_BUILD_TYPE=Release ../../
	cd ./build/Debug; cmake -DUSE_BOOST=ON -DCMAKE_BUILD_TYPE=Debug ../../
	cd ../$(DIRNAME)_eclipse; cmake -DUSE_BOOST=ON -DCMAKE_BUILD_TYPE=Debug $(CURDIR) -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_VERSION=4.3
    else ifeq ($(ARCH), WIN32)
	cd ./build; cmake ../ -DUSE_BOOST=ON -G "Visual Studio 10 Win64"
    else ifeq ($(ARCH), OSX)
	cd ./build; cmake ../ -DUSE_BOOST=ON -G Xcode
    endif

debug:
    ifeq ($(ARCH), linux)
	cd ./build/Debug; make -j$(NPROC)
    else
	@echo "Open the project file to build the project on this architecture."
    endif

release opt:
    ifeq ($(ARCH), linux)
	cd ./build/Release; make -j$(NPROC)
    else
	@echo "Open the project file to build the project on this architecture."
    endif

install:
    ifeq ($(ARCH), linux)
	cd ./build/Debug; make install
	cd ./build/Release; make install
    else
	@echo "Open the project file to run make install on this architecture."
    endif

clean:
    ifeq ($(ARCH), linux)
	cd ./build/Debug; make clean
	cd ./build/Release; make clean
    else
	@echo "Open the project file to run make clean on this architecture."
    endif

clobber:
	rm -rf ./build
	rm -rf ./install
	rm -rf ../$(DIRNAME)_eclipse

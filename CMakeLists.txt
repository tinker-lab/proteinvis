#
# Copyright Regents of the University of Minnesota, 2014.  This software is released under the following license: http://opensource.org/licenses/lgpl-3.0.html.
# Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
#
# Code author(s):
# 		Dan Orban (dtorban)
#
 
cmake_minimum_required (VERSION 2.8.2) 
project (ProteinVis)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{MinVR_DIR}/cmake/modules" "$ENV{FreeImage_DIR}")

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../install" CACHE PATH "default install path" FORCE )
endif()

#This way will auto detect all source files in the directory,
#while this will work, the preferred technique is to list out
#sourcefiles individually.  This will allow cmake to automatically
#rerun if a source file is added, which it will not do if you don't
#list the explicit source files.
#aux_source_directory(${CMAKE_SOURCE_DIR} SOURCEFILES})
#This is how you specific source file individually
set (SOURCEFILES 
		app/source/main.cpp
		app/source/App.cpp
		app/source/AbstractHCI.cpp
		app/source/GPUMesh.cpp
		app/source/GLSLProgram.cpp
		app/source/CFrameMgr.cpp
		app/source/Texture.cpp
		app/source/TextureMgr.cpp
		app/source/TouchData.cpp
		app/source/Axis.cpp
		app/source/OrigAnchoredHCI.cpp
		app/source/Feedback.cpp
		app/source/Tetrahedron.cpp
		app/source/NewAnchoredExperimentHCI.cpp
		app/source/CurrentHCIMgr.cpp
		app/source/AO.cpp
		app/source/AOgpu.cpp
		app/source/AtomColor.cpp
		app/source/CgUtil.cpp
		app/source/Common.cpp
		app/source/HardSettings.cpp
		app/source/Mol.cpp
		app/source/MyCanvas.cpp
		app/source/Sampling.cpp
		app/source/ShadowMap.cpp)

set (HEADERFILES 
		app/include/App.h
		app/include/GPUMesh.h
		app/include/GLSLProgram.h
		app/include/AbstractHCI.h
		app/include/CFrameMgr.h
		app/include/Texture.h
		app/include/TextureMgr.h
		app/include/TouchData.h
		app/include/OneEuroFilter.h
		app/include/Axis.h
		app/include/OrigAnchoredHCI.h
		app/include/Feedback.h
		app/include/Tetrahedron.h
		app/include/NewAnchoredExperimentHCI.h
		app/include/fontstash.h
		app/include/glfontstash.h
		app/include/stb_truetype.h
		app/include/ByteData.h
		app/include/ByteStream.h
		app/include/CurrentHCIMgr.h
		app/include/AO.h
		app/include/AOgpu.h
		app/include/AtomColor.h
		app/include/CgUtil.h
		app/include/Common.h
		app/include/CubeMapSamp.h
		app/include/gen_normal.h
		app/include/HardSettings.h
		app/include/Mol.h
		app/include/MyCanvas.h
		app/include/OctaMapSamp.h
		app/include/ShadowMap.h)


source_group("Header Files" FILES ${HEADERFILES})

if(USE_BOOST)
	add_definitions(-DUSE_BOOST)
endif()

############################################################
#Start of library detection
#Any libraries outside of your source should have a corresponding
#find module in the $G/src/cmake/Modules directory and the 
#find_package method should be used here.
############################################################

set(MinVR_DIR $ENV{MinVR_DIR})
find_package(MinVR COMPONENTS MVRCore AppKit_GLFW REQUIRED)
include_directories(${MinVR_INCLUDE_DIRS})

find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})

############################################################
#End external library searching
############################################################

set (CMAKE_VERBOSE_MAKEFILE TRUE)

# Used for compiling in subdirectories of the project
include_directories (
  .
  ..
  ${ProteinVis_SOURCE_DIR}/app/include
  ./app/include
  ./vcglib
)

# If you want to add any link directories you can add them here
link_directories (
  ${ProteinVis_BINARY_DIR}
)

############################################################
#OS Specific flags should go here
############################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # Linux specific code
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 

	set(Lib_Config "none")
	set(MinVR_LIBS)
	FOREACH (LIB ${MinVR_LIBRARIES})
	  if("${LIB}" STREQUAL "debug" OR "${LIB}" STREQUAL "optimized")
	    set(Lib_Config ${LIB})
	  else()
	    list(APPEND MinVR_LIBS ${LIB})
	    if("${Lib_Config}" STREQUAL "debug" OR "${Lib_Config}" STREQUAL "optimized")
	      list(APPEND MinVR_LIBS ${Lib_Config})
	    endif()
	    set(Lib_Config none)
	  endif()
	ENDFOREACH(LIB)
	LIST(REVERSE MinVR_LIBS)

	set(MinVR_LIBRARIES)

	FOREACH (LIB ${MinVR_LIBS})
	  list(APPEND MinVR_LIBRARIES ${LIB})
	ENDFOREACH(LIB)	

	find_package(Threads)
	find_package(X11)
	set(LIBS_ALL ${LIBS_ALL} ${CMAKE_THREAD_LIBS_INIT} rt Xrandr Xxf86vm Xi m dl ${X11_LIBRARIES})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  # Windows specific code

endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (MSVC)
	# Windows specific code
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

endif (MSVC)


############################################################
# Add POSTFIX info
############################################################

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")
set(CMAKE_MINSIZEREL_POSTFIX "s")

#set the build postfix extension according to the current configuration
if (CMAKE_BUILD_TYPE MATCHES "Release")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
elseif (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
elseif (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
elseif (CMAKE_BUILD_TYPE MATCHES "Debug")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
else()
  set(CMAKE_BUILD_POSTFIX "")
endif()

############################################################
#Finally add all the targets here
############################################################

#First an app of executable target
add_executable(app ${HEADERFILES} ${SOURCEFILES})
#All of libs should be added here
#If its a debug-only lib specify it as "debug <libtoadd>"
#If its a opt-only lib specify it as "optimized <libtoadd>"
#<libtoadd> should either be a full path to the lib, or the OS
#indepdent way it would appear in the command line (i.e. -l<libtoadd>)
#If the second is way is used you need to sure the directory the lib is
#in will be found
target_link_libraries (app ${MinVR_LIBRARIES} ${LIBS_DEBUG} ${LIBS_OPT} ${LIBS_ALL} ${FREEIMAGE_OPT_LIBRARIES} ${FREEIMAGE_DEBUG_LIBRARIES})


#Second an example of a library target
#It should default to making a static library
#add_library(examplevr ${HEADERFILES} ${SOURCEFILES})

#You can also add subprojects that are in subdirectories
#These subdirectories must contain their own CMakeLists.txt
#add_subdirectory (mysubproject)

#install(DIRECTORY app/include/ DESTINATION include)

#install( TARGETS AnchoredGestures app
#         LIBRARY DESTINATION lib
#         ARCHIVE DESTINATION lib
#         RUNTIME DESTINATION bin)

#
# Copyright (c) 2024 Robson Martins
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.4.1)
project(android-midi-synth)

# Where the fluidsynth library is located.
set(fluidsynth_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fluidsynth)

# fluidsynth lib binaries
add_library(libc++_shared SHARED IMPORTED)
set_target_properties(libc++_shared PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libc++_shared.so)

add_library(libFLAC SHARED IMPORTED)
set_target_properties(libFLAC PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libFLAC.so)

add_library(libfluidsynth SHARED IMPORTED)
set_target_properties(libfluidsynth PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libfluidsynth.so)

add_library(libfluidsynth-assetloader SHARED IMPORTED)
set_target_properties(libfluidsynth-assetloader PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libfluidsynth-assetloader.so)

add_library(libgio-2.0 SHARED IMPORTED)
set_target_properties(libgio-2.0 PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libgio-2.0.so)

add_library(libglib-2.0 SHARED IMPORTED)
set_target_properties(libglib-2.0 PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libglib-2.0.so)

add_library(libgmodule-2.0 SHARED IMPORTED)
set_target_properties(libgmodule-2.0 PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libgmodule-2.0.so)

add_library(libgobject-2.0 SHARED IMPORTED)
set_target_properties(libgobject-2.0 PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libgobject-2.0.so)

add_library(libgthread-2.0 SHARED IMPORTED)
set_target_properties(libgthread-2.0 PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libgthread-2.0.so)

add_library(libinstpatch-1.0 SHARED IMPORTED)
set_target_properties(libinstpatch-1.0 PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libinstpatch-1.0.so)

add_library(liboboe SHARED IMPORTED)
set_target_properties(liboboe PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/liboboe.so)

add_library(libogg SHARED IMPORTED)
set_target_properties(libogg PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libogg.so)

add_library(libopus SHARED IMPORTED)
set_target_properties(libopus PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libopus.so)

add_library(libpcre SHARED IMPORTED)
set_target_properties(libpcre PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libpcre.so)

add_library(libpcreposix SHARED IMPORTED)
set_target_properties(libpcreposix PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libpcreposix.so)
		
add_library(libsndfile SHARED IMPORTED)
set_target_properties(libsndfile PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libsndfile.so)

add_library(libvorbis SHARED IMPORTED)
set_target_properties(libvorbis PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libvorbis.so)

add_library(libvorbisenc SHARED IMPORTED)
set_target_properties(libvorbisenc PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libvorbisenc.so)

add_library(libvorbisfile SHARED IMPORTED)
set_target_properties(libvorbisfile PROPERTIES IMPORTED_LOCATION ${fluidsynth_DIR}/lib/${ANDROID_ABI}/libvorbisfile.so)

find_package(OpenMP REQUIRED)

# Native Library that will be called directly from JAVA
add_library(synth-lib SHARED
		MidiManager.cpp
		SynthManager.cpp
)

# Include fluidsynth header directory
target_include_directories(synth-lib PRIVATE ${fluidsynth_DIR}/include)

# Link everything (native lib should be the first element in the list)
target_link_libraries(
		synth-lib
		libc++_shared
        libFLAC
        libfluidsynth-assetloader
        libgio-2.0
        libglib-2.0
        libgmodule-2.0
        libgobject-2.0
        libgthread-2.0
        libinstpatch-1.0
        liboboe
        libogg
        libopus
        libpcre
        libpcreposix
        libsndfile
        libvorbis
        libvorbisenc
        libvorbisfile
        libfluidsynth
        OpenMP::OpenMP_CXX
		amidi
)

#-------------------------------------------------------------------------------
# Copyright 2012 Google Inc. All Rights Reserved.
# Author: blakely@google.com (Tim Blakely)
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#-------------------------------------------------------------------------------
cmake_minimum_required( VERSION 2.8 )

##############################
#
# Project defines
#
##############################

project( BigBrain )

set( BigBrain_VER_MAJOR "0" )
set( BigBrain_VER_MINOR "2" )

##############################
#
# Required packages
#
##############################

find_package (Threads REQUIRED) 
find_package (CURL REQUIRED) 

##############################
#
# Directory defines
#
##############################

if(NOT DEFINED BigBrain_ROOT_DIR)
  set ( BigBrain_ROOT_DIR ${BigBrain_SOURCE_DIR}/.. )
endif(NOT DEFINED BigBrain_ROOT_DIR)

set( BigBrain_SRC_DIR ${BigBrain_ROOT_DIR}/src )
set( BigBrain_BIN_DIR ${BigBrain_ROOT_DIR}/bin )
set( BigBrain_CMAKE_DIR ${BigBrain_ROOT_DIR}/build/cmake )

set( BigBrain_SRC_PROTO_DIR ${BigBrain_SRC_DIR}/protos )
set( BigBrain_SRC_NETWORK_DIR ${BigBrain_SRC_DIR}/network )
set( BigBrain_SRC_TOPOLOGY_DIR ${BigBrain_SRC_DIR}/topology )
set( BigBrain_SRC_CONTROLLER_DIR ${BigBrain_SRC_DIR}/controller )

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BigBrain_ROOT_DIR}/obj )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BigBrain_BIN_DIR} )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${BigBrain_CMAKE_DIR} )

set( EXT_HEADERS "${BigBrain_ROOT_DIR}/extlib/include" )

link_directories( "${BigBrain_ROOT_DIR}/extlib/lib/" )

##############################
#
# Module dependencies
#
##############################

include( ${BigBrain_CMAKE_DIR}/cython.cmake )
include( ${BigBrain_CMAKE_DIR}/compiler.cmake )
include( ${BigBrain_CMAKE_DIR}/macros.cmake )
include( ${BigBrain_CMAKE_DIR}/proto.cmake )
include( ${BigBrain_CMAKE_DIR}/python.cmake )
include( ${BigBrain_CMAKE_DIR}/boost.cmake )
include( ${BigBrain_CMAKE_DIR}/zlib.cmake )

##############################
#
# BigBrain build 
#
##############################

include_directories( ${BigBrain_SRC_DIR} ${EXT_HEADERS} )

add_subdirectory( ${BigBrain_SRC_PROTO_DIR} ${BigBrain_ROOT_DIR}/obj/proto )
add_subdirectory( ${BigBrain_SRC_NETWORK_DIR} ${BigBrain_ROOT_DIR}/obj/network )
add_subdirectory( ${BigBrain_SRC_TOPOLOGY_DIR} ${BigBrain_ROOT_DIR}/obj/topology )
add_subdirectory( ${BigBrain_SRC_CONTROLLER_DIR} ${BigBrain_ROOT_DIR}/obj/controller )

add_executable( simulator ${BigBrain_SRC_DIR}/main.cc )

target_link_libraries( simulator Network Topology Controller ${EXT_LIBS} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${PYTHON_LIBRARIES} )

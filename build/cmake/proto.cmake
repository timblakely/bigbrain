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
# We need to add these manually, since cmake doesn't really support protobufs

# Modified from FindProtobuf.cmake, which is:  
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
# Copyright 2008 Esben Mose Hansen, Ange Optimization ApS

# Protobuffer helper functions
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})


function(PROTOBUF_GENERATE SRCS HDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE() called without any proto files")
    return()
  endif(NOT ARGN)

  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    
    list(APPEND ${SRCS} "${BigBrain_SRC_PROTO_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${SRCS} "${BigBrain_SRC_PROTO_DIR}/${FIL_WE}_pb2.py")
    list(APPEND ${HDRS} "${BigBrain_SRC_PROTO_DIR}/${FIL_WE}.pb.h")

    add_custom_command(
      OUTPUT "${BigBrain_SRC_PROTO_DIR}/${FIL_WE}.pb.cc"
             "${BigBrain_SRC_PROTO_DIR}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --cpp_out  ${BigBrain_SRC_PROTO_DIR} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL}
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --python_out  ${BigBrain_SRC_PROTO_DIR} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running Python/C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
  
endfunction()

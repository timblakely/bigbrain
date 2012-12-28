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
message("-- Generating protos")
# Setup protobuf generation
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

file(GLOB proto_files "*.proto")

foreach(FILE ${proto_files}) 
    message("---- Adding rule for proto: " ${FILE})
    PROTOBUF_GENERATE(PROTO_SRCS PROTO_HDRS ${FILE})
endforeach()

add_library(bigbrain_protos ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(bigbrain_protos ${PROTOBUF_LIBRARIES})

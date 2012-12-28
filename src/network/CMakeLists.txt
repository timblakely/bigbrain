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
add_subdirectory(objects)

set(NETWORK_HEADERS
    "network.h"
    "parameter_collection.h"
    "bio_factory.h"
)

set(NETWORK_SRCS
    "network.cc"
    "parameter_collection.cc"
    "bio_factory.cc"
)

include_directories(${BigBrain_SRC_NETWORK_DIR})

add_library(Network ${NETWORK_HEADERS} ${NETWORK_SRCS})
target_link_libraries(Network bigbrain_protos bigbrain_objects_cells bigbrain_objects_channels bigbrain_objects_connections)




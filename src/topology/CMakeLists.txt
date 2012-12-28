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
add_subdirectory(arrangement)
add_subdirectory(connector)

add_library(Topology topology.cc topology.h arrangement.cc arrangement.h connector.cc connector.h)

target_link_libraries(Topology bigbrain_TOPOLOGY_ARRANGEMENT bigbrain_TOPOLOGY_CONNECTOR) 

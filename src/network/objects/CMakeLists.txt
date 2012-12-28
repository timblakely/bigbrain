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
add_library(bigbrain_object_framework channel.cc channel.h network_unit.cc network_unit.h neuron.cc neuron.h synapse.cc synapse.h)
add_dependencies(bigbrain_object_framework bigbrain_protos)

add_subdirectory(cells)
add_subdirectory(channels)
add_subdirectory(connections)

 

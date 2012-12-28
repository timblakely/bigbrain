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
ADD_CHANNEL(i_k_na)
ADD_CHANNEL(i_na_p)
ADD_CHANNEL(i_h)

include_directories(${BigBrain_SRC_PROTOS})

add_library(bigbrain_objects_channels ${CHANNEL_SRCS} ${CHANNEL_HDRS})
target_link_libraries(bigbrain_objects_channels bigbrain_object_framework)

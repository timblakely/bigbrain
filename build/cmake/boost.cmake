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

set(Boost_ADDITIONAL_VERSIONS "1.49" "1.49.0" "1.48" "1.48.0")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package( Boost REQUIRED COMPONENTS regex iostreams program_options)

if(Boost_FOUND)
    message("-- Found boost: " ${Boost_INCLUDE_DIRS}) 
endif()

set(EXT_HEADERS ${EXT_HEADERS} ${Boost_INCLUDE_DIRS})

message("-- Found boost libs: " ${Boost_LIBRARIES}) 

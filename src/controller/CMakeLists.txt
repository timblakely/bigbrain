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


# BB API Cython code

compile_pyx(apilayer generated_sources ${BigBrain_SRC_CONTROLLER_DIR}/apilayer.pyx)
set_source_files_properties(${generated_sources} PROPERTIES GENERATED TRUE)
find_package( PythonLibs REQUIRED )
include_directories(${PYTHON_INCLUDE_DIRS})

SET_SOURCE_FILES_PROPERTIES( apilayer.h PROPERTIES LANGUAGE CXX )
SET_SOURCE_FILES_PROPERTIES( apilayer.c PROPERTIES LANGUAGE CXX )

file(GLOB COPY_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.py)
add_custom_target(copyfiles ALL
    COMMENT "Copying python files to bin directory"
    )
foreach(FILENAME ${COPY_FILES})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(DST "${BigBrain_BIN_DIR}/${FILENAME}")

    add_custom_command(
        TARGET copyfiles
        COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
    )
endforeach(FILENAME)

add_library(Controller simulator.cc simulator.h 
  ${generated_sources} bigbrain_api.h bigbrain_api.cc)

target_link_libraries(Controller ${EXT_LIBS} Network Topology)
target_link_libraries(Controller ${CURL_LIBRARIES})
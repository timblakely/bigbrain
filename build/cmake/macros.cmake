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
macro(BigBrain_ADD_NEURON_TYPE NAME SOURCES HEADERS)
   
    message("-- Creating neuron project: " ${NAME})
    SOURCE_GROUP( Source\\Project FILES ${SOURCES})
    SOURCE_GROUP( Headers\\Project FILES ${HEADERS})
    
    add_library(${NAME} STATIC ${SOURCES} ${HEADERS})

endmacro(BigBrain_ADD_NEURON_TYPE)

macro(set_output_directory) 


endmacro(set_output_directory)

macro(ADD_CELL NAME)
    message("---- Adding cell: " ${NAME})
    set(CELL_HDRS ${CELL_HDRS} ${NAME}.cc)
    set(CELL_SRCS ${CELL_SRCS} ${NAME}.h)
endmacro()

macro(ADD_CHANNEL NAME)
    message("---- Adding channel: " ${NAME})
    set(CHANNEL_HDRS ${CHANNEL_HDRS} ${NAME}.cc)
    set(CHANNEL_SRCS ${CHANNEL_SRCS} ${NAME}.h)
endmacro()

macro(ADD_CONNECTION NAME)
    message("---- Adding connection: " ${NAME})
    set(CONNECTION_HDRS ${CONNECTION_HDRS} ${NAME}.cc)
    set(CONNECTION_SRCS ${CONNECTION_SRCS} ${NAME}.h)
endmacro()

macro(ADD_ARRANGEMENT NAME)
    message("---- Adding topology arrangement: " ${NAME})
    set(ARRANGEMENT_HDRS ${ARRANGEMENT_HDRS} ${NAME}.cc)
    set(ARRANGEMENT_SRCS ${ARRANGEMENT_SRCS} ${NAME}.h)
endmacro()

macro(ADD_CONNECTOR NAME)
    message("---- Adding topology connector: " ${NAME})
    set(CONNECTOR_HDRS ${CONNECTOR_HDRS} ${NAME}.cc)
    set(CONNECTOR_SRCS ${CONNECTOR_SRCS} ${NAME}.h)
endmacro()

macro(ADD_AGENT NAME)
    message("---- Adding agent: " ${NAME})
    set(AGENT_HDRS ${AGENT_HDRS} ${NAME}.cc)
    set(AGENT_SRCS ${AGENT_SRCS} ${NAME}.h)
endmacro()

if (CMAKE_COMPILER_IS_GNUCC)
    #Require > 4.6
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
    list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
  
    message("-- Detected GCC compiler " ${GCC_MAJOR}.${GCC_MINOR}.${GCC_PATCH})
    if(${GCC_MAJOR} STRLESS 4 OR (${GCC_MAJOR} STREQUAL 4 AND ${GCC_MINOR} STRLESS 6))
        message(FATAL_ERROR "**GCC is too old! Found ver: " ${GCC_MAJOR}"."${GCC_MINOR}"."${GCC_PATCH}", requires >= 4.6")
    elseif(${GCC_MAJOR} STREQUAL 4 AND ${GCC_MINOR} STREQUAL 6)
        # GCC 4.6 requires c++0x 
        add_definitions("-std=c++0x")
    else()
        # GCC >= 4.7 requires c++11 
        add_definitions("-std=c++11")
    endif() 
endif()
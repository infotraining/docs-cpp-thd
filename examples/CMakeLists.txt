cmake_minimum_required(VERSION 3.20)
project(cpp_thd_examples CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

option(EXAMPLES_SANITIZE_THREAD "Build with ThreadSanitizer" OFF)

file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/[0-9][0-9]_*.cpp)

foreach(source ${EXAMPLE_SOURCES})
    get_filename_component(name ${source} NAME_WE)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE Threads::Threads)

    if(MSVC)
        target_compile_options(${name} PRIVATE /W4)
    else()
        # -Wno-unused-result: 11_futures.cpp intentionally discards std::async results
        # -Wno-unused-*: demo snippets keep local variables only to illustrate the API
        target_compile_options(${name} PRIVATE -Wall -Wextra -Wno-unused-result
                               -Wno-unused-variable -Wno-unused-but-set-variable)
        if(EXAMPLES_SANITIZE_THREAD)
            target_compile_options(${name} PRIVATE -fsanitize=thread -g)
            target_link_options(${name} PRIVATE -fsanitize=thread)
        endif()
    endif()
endforeach()

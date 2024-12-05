cmake_minimum_required(VERSION 3.12)
project(2048_Game CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -flto
        -ffast-math
    )

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
elseif(MSVC)
    add_compile_options(
        /O2
        /GL
        /arch:AVX2
    )
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
endif()

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

add_executable(2048.out 
    main.cpp 
    game.cpp 
    utils.cpp
)

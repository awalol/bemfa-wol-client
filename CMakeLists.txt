cmake_minimum_required(VERSION 3.5)
project(wakeonlan_tcp)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "mipsel")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
    add_link_options("-s")

    # 静态编译测试，如果使用静态编译，使用 gnu 的工具链编译后也可以在 musl 上运行
    #    set(CMAKE_EXE_LINKER_FLAGS "-static")
    #    set(CMAKE_CXX_FLAGS  "-pthread")

    # 体积优化参数（效果不大，加上 upx 只少了 6kb）
    #    cmake_policy(SET CMP0069 NEW)
    #    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    #    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    #    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
    #    add_compile_options(-fno-rtti -fno-exceptions)
endif ()

add_executable(wakeonlan_tcp main.cpp)

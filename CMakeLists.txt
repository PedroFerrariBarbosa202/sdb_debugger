cmake_minimum_required(VERSION 3.19)

project("sdb" LANGUAGES CXX ASM)

include(CTest)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libedit REQUIRED IMPORTED_TARGET libedit)

add_subdirectory("domain/src")
add_subdirectory("application")
add_subdirectory("presentation")

if(BUILD_TESTING)
    find_package(Catch2 CONFIG REQUIRED)
    add_subdirectory("test")
endif()


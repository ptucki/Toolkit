cmake_minimum_required(VERSION 3.13...3.29)

project(Toolkit VERSION 1.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 20)

if (MSVC)
  add_compile_options(/utf-8 /Zc:char8_t)
endif()

include(GNUInstallDirs)

add_subdirectory(src)

install(DIRECTORY include/Toolkit DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
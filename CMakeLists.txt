cmake_minimum_required(VERSION 3.13...3.29)

project(Toolkit VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)

add_subdirectory(src)

message("CMAKE_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY include/Toolkit DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h")

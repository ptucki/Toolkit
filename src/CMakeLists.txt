set(TOOLKIT_SOURCE_FILES
  Json/json.cpp
  Json/json.h
  
  Json/json_parser.cpp
  Json/json_parser.h

  String/string_ex.cpp
  String/string_ex.h

  Filesystem/filesystem_ex.cpp
  Filesystem/filesystem_ex.h
)

add_library(${PROJECT_NAME} STATIC ${TOOLKIT_SOURCE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories(${PROJECT_NAME} PRIVATE
  ./
  Json/
  String/
  Filesystem/
)

# -------------------------------------------------------------------
# Extract Toolkit header files
# -------------------------------------------------------------------

foreach(TOOLKIT_FILE IN LISTS TOOLKIT_SOURCE_FILES)
  get_filename_component(BARENAME ${TOOLKIT_FILE} NAME)

  cmake_path(GET TOOLKIT_FILE EXTENSION LAST_ONLY FILE_EXTENSION)

  if ("${FILE_EXTENSION}" STREQUAL ".h")
  # cmake_path(REMOVE_FILENAME IMGUI_FILE OUTPUT_VARIABLE IMGUI_FILE_PATH)
  # file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${IMGUI_FILE}" DESTINATION "${CMAKE_SOURCE_DIR}/src/${IMGUI_FILE_PATH}")
    configure_file(${TOOLKIT_FILE} ${CMAKE_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/Modules/${BARENAME} COPYONLY)
  endif()
endforeach()

# -------------------------------------------------------------------
# Copy headers to include directory
# -------------------------------------------------------------------

foreach(TOOLKIT_FILE IN LISTS TOOLKIT_SOURCE_FILES)
  get_filename_component(BARENAME ${TOOLKIT_FILE} NAME)

  cmake_path(GET TOOLKIT_FILE EXTENSION LAST_ONLY FILE_EXTENSION)

  if ("${FILE_EXTENSION}" STREQUAL ".h")
  # cmake_path(REMOVE_FILENAME IMGUI_FILE OUTPUT_VARIABLE IMGUI_FILE_PATH)
  # file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${IMGUI_FILE}" DESTINATION "${CMAKE_SOURCE_DIR}/src/${IMGUI_FILE_PATH}")
    configure_file(${TOOLKIT_FILE} ${CMAKE_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/Modules/${BARENAME} COPYONLY)
  endif()
endforeach()

# -------------------------------------------------------------------
# Generate one header
# -------------------------------------------------------------------

set(TOOLKIT_HEADER_DEFINES "")
set(ONE_HEADER_FILE_PATH "${CMAKE_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/Toolkit.h")

foreach(TOOLKIT_FILE IN LISTS TOOLKIT_SOURCE_FILES)
  get_filename_component(BARENAME ${TOOLKIT_FILE} NAME)

  cmake_path(GET TOOLKIT_FILE EXTENSION LAST_ONLY FILE_EXTENSION)

  if ("${FILE_EXTENSION}" STREQUAL ".h")
    string(APPEND TOOLKIT_HEADER_DEFINES "#include \"Modules/${BARENAME}\"\n")
  endif()
endforeach()

configure_file("Toolkit.h.in" "${CMAKE_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_NAME}.h")

# -------------------------------------------------------------------
# Install targets
# -------------------------------------------------------------------


install(TARGETS ${PROJECT_NAME})

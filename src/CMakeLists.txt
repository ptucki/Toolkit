add_library(${PROJECT_NAME} STATIC
  Json/json.cpp
  Json/json.h
  Json/json_parser.cpp
  Json/json_parser.h
  )

include_directories(
  ./
  Json/
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

install(TARGETS ${PROJECT_NAME})

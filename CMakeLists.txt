cmake_minimum_required(VERSION 3.10)
project(sandbox VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (UNIX)
   add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/lib)
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

if (UNIX)
   find_package(raylib REQUIRED)
   target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
else()
   add_library(raylib SHARED IMPORTED)
   set_target_properties(raylib PROPERTIES
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/raylib.dll
      IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/raylibdll.lib
      INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/lib)
   target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
endif()

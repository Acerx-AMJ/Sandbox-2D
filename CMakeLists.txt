cmake_minimum_required(VERSION 3.10)
project(sandbox VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall)

include(FetchContent)
FetchContent_Declare(srulib GIT_REPOSITORY https://github.com/Acerx-AMJ/SRU-Library.git GIT_TAG main GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(srulib)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/lib)
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

find_package(raylib REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib srulib)

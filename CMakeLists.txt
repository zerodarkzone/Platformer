cmake_minimum_required(VERSION 3.26)

SET(PROJECT_NAME Physics_Test)
project(${PROJECT_NAME})

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wreorder -DCRO_DEBUG_")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O3 -flto -march=native -DNDEBUG")

# We're using c++20
SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)

SET (OpenGL_GL_PREFERENCE "GLVND")

find_package(CROGINE REQUIRED)
find_package(TMXLITE REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Box2D REQUIRED)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CROGINE_LIBRARIES crogine-d)
  set(TMXLITE_LIBRARIES tmxlite-d)
else()
  set(CROGINE_LIBRARIES crogine)
  set(TMXLITE_LIBRARIES tmxlite)
endif ()

include_directories(
        ${CROGINE_INCLUDE_DIR}
        ${TMXLITE_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${BOX2D_INCLUDE_DIR}
        src)

SET(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include(${PROJECT_DIR}/CMakeLists.txt)

add_executable(${PROJECT_NAME} ${PROJECT_SRC})

target_link_libraries(${PROJECT_NAME}
        ${CROGINE_LIBRARIES}
        ${TMXLITE_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
        box2d)

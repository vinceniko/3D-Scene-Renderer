cmake_minimum_required(VERSION 3.16)
project(Assignment3)

set ( CMAKE_CXX_STANDARD 17 )

find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)

# Suppress warnings of the deprecation of glut functions on macOS.
if(APPLE)
 add_definitions(-Wno-deprecated-declarations)
endif()

### Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

### Compilation flags: adapt to your needs ###
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj") ### Enable parallel compilation
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} )
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR} )
endif()

### Add src to the include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

### Add OpenGL
set(INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
set(LIBRARIES ${OPENGL_LIBRARIES})

### Include glm for linear algebra
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../ext/glm")

### Compile GLFW3 statically
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ext/glfw" "glfw")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../ext/glfw/include")
set(LIBRARIES "glfw" ${GLFW_LIBRARIES})

### On windows, you also need glew
if((UNIX AND NOT APPLE) OR WIN32)
  set(GLEW_INSTALL OFF CACHE BOOL " " FORCE)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ext/glew" "glew")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../ext/glew/include")
  list(APPEND LIBRARIES "glew")
endif()

if(APPLE)
  list(APPEND LIBRARIES "-framework OpenGL")
endif()

### Compile all the cpp files in src
file(GLOB HELPERS
  "${CMAKE_CURRENT_SOURCE_DIR}/extra/Helpers.*"
)

### Compile all the cpp files in lib
file(GLOB LIB
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/*"
)

add_library(Lib ${LIB})

### Compile all the cpp files in src
file(GLOB SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*"
)

add_executable(${PROJECT_NAME}_bin ${SOURCES})
target_link_libraries(${PROJECT_NAME}_bin ${LIBRARIES} ${OPENGL_LIBRARIES} Lib)
target_include_directories(${PROJECT_NAME}_bin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib")

if (CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(${PROJECT_NAME}_bin PUBLIC -DDEBUG)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extra)
add_executable(main_blending_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_blending.cpp ${HELPERS})
target_link_libraries(main_blending_bin ${LIBRARIES} ${OPENGL_LIBRARIES})

add_executable(main_border_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_border.cpp ${HELPERS})
target_link_libraries(main_border_bin ${LIBRARIES} ${OPENGL_LIBRARIES})

add_executable(main_depth_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_depth.cpp ${HELPERS})
target_link_libraries(main_depth_bin ${LIBRARIES} ${OPENGL_LIBRARIES})

add_executable(main_positions_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_positions.cpp ${HELPERS})
target_link_libraries(main_positions_bin ${LIBRARIES} ${OPENGL_LIBRARIES})

add_executable(main_properties_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_properties.cpp ${HELPERS})
target_link_libraries(main_properties_bin ${LIBRARIES} ${OPENGL_LIBRARIES})

add_executable(main_view_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_view.cpp ${HELPERS})
target_link_libraries(main_view_bin ${LIBRARIES} ${OPENGL_LIBRARIES})
endif ()

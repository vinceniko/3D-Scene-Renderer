cmake_minimum_required(VERSION 3.16)
project(3DSceneEditor)

set ( CMAKE_CXX_STANDARD 17 )

# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
endif()

set(EXT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext")

find_package(OpenGL REQUIRED)

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

### Add OpenGL
include_directories(${OPENGL_INCLUDE_DIR})
set(DEPENDENCIES ${OPENGL_LIBRARIES})

### Include glm for linear algebra
include_directories("${EXT_DIR}/glm")

### Compile GLFW3 statically
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
add_subdirectory("${EXT_DIR}/glfw" "glfw")
include_directories("${EXT_DIR}/glfw/include")
list(APPEND DEPENDENCIES "glfw" ${GLFW_LIBRARIES})

### On windows, you also need glew
if((UNIX AND NOT APPLE) OR WIN32)
  set(GLEW_INSTALL OFF CACHE BOOL " " FORCE)
  add_subdirectory("${EXT_DIR}/glew" "glew")
  include_directories("${EXT_DIR}/glew/include")
  list(APPEND DEPENDENCIES "glew")
endif()

if(APPLE)
  list(APPEND DEPENDENCIES "-framework OpenGL")
endif()

### add stb
### image loading
set(STB "${EXT_DIR}/stb/stb_image.h")

### Compile all the cpp files in lib
file(GLOB LIB
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/*"
)

### Compile all the cpp files in src
file(GLOB SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*"
)

add_executable(${PROJECT_NAME}_bin)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(${PROJECT_NAME}_bin PUBLIC -DDEBUG)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

if (TIMER)
  target_compile_definitions(${PROJECT_NAME}_bin PUBLIC -DTIMER)
else()
  list(REMOVE_ITEM LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/timer.h")
  list(REMOVE_ITEM LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/timer.cpp")
endif()

target_sources(${PROJECT_NAME}_bin PUBLIC ${SOURCES} ${LIB})

target_include_directories(${PROJECT_NAME}_bin PUBLIC "${EXT_DIR}/stb" "${CMAKE_CURRENT_SOURCE_DIR}/lib" "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME}_bin ${DEPENDENCIES})

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extra)
  ### Compile all the cpp files in src
  file(GLOB HELPERS
    "${CMAKE_CURRENT_SOURCE_DIR}/extra/Helpers.*"
  )
  
  add_executable(main_blending_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_blending.cpp ${HELPERS})
  target_link_libraries(main_blending_bin ${DEPENDENCIES})

  add_executable(main_border_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_border.cpp ${HELPERS})
  target_link_libraries(main_border_bin ${DEPENDENCIES})

  add_executable(main_depth_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_depth.cpp ${HELPERS})
  target_link_libraries(main_depth_bin ${DEPENDENCIES})

  add_executable(main_positions_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_positions.cpp ${HELPERS})
  target_link_libraries(main_positions_bin ${DEPENDENCIES})

  add_executable(main_properties_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_properties.cpp ${HELPERS})
  target_link_libraries(main_properties_bin ${DEPENDENCIES})

  add_executable(main_view_bin ${CMAKE_CURRENT_SOURCE_DIR}/extra/main_view.cpp ${HELPERS})
  target_link_libraries(main_view_bin ${DEPENDENCIES})
endif ()

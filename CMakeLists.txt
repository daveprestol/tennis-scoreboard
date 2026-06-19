cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core Network)
  foreach(imported_target IN ITEMS Qt6::Gui Qt6::Widgets WrapOpenGL::WrapOpenGL)
    if(TARGET ${imported_target})
      get_target_property(imported_libraries ${imported_target} INTERFACE_LINK_LIBRARIES)
      if(imported_libraries)
        list(REMOVE_ITEM imported_libraries "-framework AGL" "/System/Library/Frameworks/AGL.framework")
        set_target_properties(${imported_target} PROPERTIES INTERFACE_LINK_LIBRARIES "${imported_libraries}")
      endif()
    endif()
  endforeach()
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/plugin-main.cpp
    src/core/MatchState.cpp
    src/core/TennisRules.cpp
    src/overlay/OverlayHttpServer.cpp
    src/ui/ScoreboardDock.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src)

file(GLOB_RECURSE PLUGIN_RESOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")
foreach(resource_file IN LISTS PLUGIN_RESOURCE_FILES)
  cmake_path(
    RELATIVE_PATH
    resource_file
    BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    OUTPUT_VARIABLE resource_relative_path
  )
  cmake_path(GET resource_relative_path PARENT_PATH resource_parent_path)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${resource_file}")
  set_property(SOURCE "${resource_file}" PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${resource_parent_path}")
  source_group("Resources/${resource_parent_path}" FILES "${resource_file}")
endforeach()

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

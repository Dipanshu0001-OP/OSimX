cmake_minimum_required(VERSION 3.16)

project(OSimX VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

file(GLOB_RECURSE OSIMX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/database/*.cpp"
)
file(GLOB_RECURSE OSIMX_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/app/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/database/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/*.h"
)

add_executable(osimx
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${OSIMX_SOURCES}
    ${OSIMX_HEADERS}
    resources.qrc
)

target_include_directories(osimx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(osimx PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

if(WIN32)
    set_target_properties(osimx PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

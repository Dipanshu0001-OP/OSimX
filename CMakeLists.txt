cmake_minimum_required(VERSION 3.16)
project(OSimX VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(OSIMX_BUILD_APP "Build the Qt desktop application" ON)
option(OSIMX_BUILD_TESTS "Build dependency-free algorithm tests" OFF)

if(OSIMX_BUILD_APP)
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
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        ${OSIMX_SOURCES}
        ${OSIMX_HEADERS}
        resources.qrc
    )
    target_include_directories(osimx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(osimx PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)
    if(WIN32)
        set_target_properties(osimx PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()

if(OSIMX_BUILD_TESTS)
    enable_testing()
    add_executable(osimx_backend_tests
        tests/backend_tests.cpp
        modules/cpu_scheduling/backend/CPUScheduler.cpp
        modules/cpu_scheduling/backend/FCFS.cpp
        modules/cpu_scheduling/backend/SJF.cpp
        modules/cpu_scheduling/backend/SRTF.cpp
        modules/cpu_scheduling/backend/RoundRobin.cpp
        modules/cpu_scheduling/backend/PriorityScheduling.cpp
        modules/disk_scheduling/backend/DiskScheduler.cpp
        modules/memory_management/backend/MemoryAllocator.cpp
        modules/memory_management/backend/FirstFit.cpp
        modules/memory_management/backend/BestFit.cpp
        modules/memory_management/backend/WorstFit.cpp
        modules/memory_management/backend/PageReplacer.cpp
        modules/memory_management/backend/FIFO.cpp
        modules/memory_management/backend/LRU.cpp
        modules/memory_management/backend/Optimal.cpp
    )
    target_include_directories(osimx_backend_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME osimx_backend_tests COMMAND osimx_backend_tests)
    add_executable(osimx_filesystem_tests
        tests/filesystem_tests.cpp
        modules/file_system/backend/FileSystem.cpp
    )
    target_include_directories(osimx_filesystem_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME osimx_filesystem_tests COMMAND osimx_filesystem_tests)
endif()

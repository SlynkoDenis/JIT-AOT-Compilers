set(SOURCES
    default/DefaultArch.cpp
    LinearScanRegAlloc.cpp
    RegAllocChecker.cpp
    )

add_library(code_generation STATIC ${SOURCES})
enable_project_warnings(code_generation)

target_sources(code_generation PUBLIC
    default/DefaultArch.h
    LinearScanRegAlloc.h
    RegAllocChecker.h
    RegMap.h
    )

target_include_directories(code_generation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/analysis
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ir
    ${CMAKE_SOURCE_DIR}/utils
    )

add_dependencies(code_generation analysis)

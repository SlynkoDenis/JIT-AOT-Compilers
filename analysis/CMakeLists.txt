set(SOURCES
    Traversals.cpp
    )

add_library(analysis STATIC ${SOURCES})

target_sources(ir PUBLIC
        DomTree.h
        Traversals.h
        )

include_directories(${CMAKE_SOURCE_DIR}/utils)
include_directories(${CMAKE_SOURCE_DIR}/ir)

target_include_directories(ir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    DomTree.cpp
    DSU.cpp
    Traversals.cpp
    )

add_library(analysis STATIC ${SOURCES})

target_sources(ir PUBLIC
        DomTree.h
        DSU.h
        Traversals.h
        )

include_directories(${CMAKE_SOURCE_DIR}/utils)
include_directories(${CMAKE_SOURCE_DIR}/ir)

target_include_directories(analysis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    debug.cpp
    arena/ArenaAllocator.cpp
    )

add_library(utils STATIC ${SOURCES})

target_sources(utils PUBLIC
    debug.h
    macros.h
    arena/ArenaAllocator.h
    )

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

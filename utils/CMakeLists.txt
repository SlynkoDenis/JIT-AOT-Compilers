set(SOURCES
    debug.cpp
    arena/ArenaAllocator.cpp
    dumper/EventDumper.cpp
    )

add_library(utils STATIC ${SOURCES})

target_sources(utils PUBLIC
    debug.h
    helpers.h
    macros.h
    arena/ArenaAllocator.h
    dumper/DummyDumper.h
    dumper/EventDumper.h
    dumper/FileDumper.h
    )

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

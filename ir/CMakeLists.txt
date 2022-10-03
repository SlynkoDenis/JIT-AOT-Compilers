set(SOURCES
    BasicBlock.cpp
    Graph.cpp
    )

add_library(ir STATIC ${SOURCES})

target_sources(ir PUBLIC
        BasicBlock.h
        Concepts.h
        Graph.h
        Instruction.h
        InstructionBuilder.h
        IRBuilder.h
        Types.h
        )

include_directories(${CMAKE_SOURCE_DIR}/utils)

target_include_directories(ir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

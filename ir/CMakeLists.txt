set(SOURCES
    BasicBlock.cpp
    Compiler.cpp
    Graph.cpp
    GraphCopyHelper.cpp
    InstructionBase.cpp
    InstructionsCopy.cpp
    )

add_library(ir STATIC ${SOURCES})

target_sources(ir PUBLIC
    BasicBlock.h
    BasicBlockIterator.h
    Compiler.h
    CompilerBase.h
    Concepts.h
    Graph.h
    GraphCopyHelper.h
    Input.h
    Instruction.h
    InstructionBase.h
    InstructionBuilder.h
    Loop.h
    marker/marker.h
    Types.h
    Users.h
    )

target_include_directories(ir PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/utils
    )

add_dependencies(ir utils)

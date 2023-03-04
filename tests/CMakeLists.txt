set(BINARY tests)

set(SOURCES
    BasicBlockTest.cpp
    CompilerTestBase.h
    DCETest.cpp
    DomTreeTest.cpp
    EmptyBlocksRemovalTest.cpp
    GraphTest.cpp
    InliningTest.cpp
    InstructionsTest.cpp
    LoopAnalysisTest.cpp
    main.cpp
    PeepholesTest.cpp
    TestGraphSamples.h
    TestGraphSamples.cpp
    TraversalsTest.cpp
    )

add_executable(${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC analysis ir gtest optimization utils)

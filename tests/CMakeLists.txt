set(BINARY tests)

set(SOURCES
    BasicBlockTest.cpp
    DomTreeTest.cpp
    GraphTest.cpp
    InstructionsTest.cpp
    LoopAnalysisTest.cpp
    main.cpp
    TestGraphSamples.h
    TestGraphSamples.cpp
    TraversalsTest.cpp
    )

add_executable(${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC analysis ir gtest utils)

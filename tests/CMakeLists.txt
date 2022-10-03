set(BINARY tests)

set(SOURCES InstructionsTest.cpp main.cpp)

add_executable(${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC ir gtest utils)

PROJECT(kernel)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(LINKER_RULES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/link.ld)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -T ${LINKER_RULES_FILE}")

FILE(GLOB_RECURSE SRC_FILES "src/*.c")
FILE(GLOB_RECURSE ASM_FILES "src/*.s")

INCLUDE_DIRECTORIES(include)

SET_SOURCE_FILES_PROPERTIES(${ASM_FILES} PROPERTIES LANGUAGE ASM_NASM)
SET_SOURCE_FILES_PROPERTIES(${SHARED_ASM_FILES} PROPERTIES LANGUAGE ASM_NASM)

ADD_EXECUTABLE(${PROJECT_NAME} ${SHARED_ASM_FILES} ${ASM_FILES} ${SHARED_SRC_FILES} ${SRC_FILES})
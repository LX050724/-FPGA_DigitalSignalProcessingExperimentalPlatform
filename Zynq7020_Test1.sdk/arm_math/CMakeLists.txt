file(GLOB_RECURSE SOURCES src/*.c src/*.h)

add_library(arm_math ${SOURCES})
target_include_directories(arm_math PUBLIC src/Include)
target_include_directories(arm_math PRIVATE src/PrivateInclude)

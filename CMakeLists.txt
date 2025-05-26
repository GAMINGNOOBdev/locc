cmake_minimum_required(VERSION 3.10)

string(TIMESTAMP NOW "%Y %m %d")
string(REPLACE " " ";" DATE_LIST ${NOW})
list(GET DATE_LIST 0 YEAR)
list(GET DATE_LIST 1 MONTH)
list(GET DATE_LIST 2 DAY)

math(EXPR YEAR ${YEAR})
math(EXPR MONTH ${MONTH})
math(EXPR DAY ${DAY})

project(locc VERSION ${YEAR}.${MONTH}.${DAY} DESCRIPTION "line of code counter" HOMEPAGE_URL "https://gamingnoobdev.github.io")

add_compile_definitions(__YEAR__=${YEAR})
add_compile_definitions(__MONTH__=${MONTH})
add_compile_definitions(__DAY__=${DAY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (UNIX)
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

    add_compile_options(
        "-Wall" "-Wextra" "-Wpedantic" "-fexceptions" "-Wl,--export-dynamic"
    )
endif()

include_directories(inc)

set(sources
    src/argument_parser.c
    src/string_util.c
    src/file_util.c
    src/logging.c
    src/main.c
    )

add_executable(${PROJECT_NAME} ${sources})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if( MSVC )
    set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME} )
endif()


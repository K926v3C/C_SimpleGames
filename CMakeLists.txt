cmake_minimum_required(VERSION 3.0.0)
project(C_SimpleGames VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
add_compile_options(/std:c17 /nologo /source-charset:UTF-8 /execution-charset:GB18030 /Zi /W4)

file(GLOB SourceFiles src/*.c)
foreach(File IN LISTS SourceFiles)
    get_filename_component(FileName ${File} NAME_WE)
    add_executable(${FileName} ${SourceFiles})
endforeach()
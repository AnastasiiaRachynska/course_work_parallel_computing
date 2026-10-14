cmake_minimum_required(VERSION 3.20)
project(course_work_parallel_computing CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(course_work_parallel_computing src/main.cpp
        src/ApiHandler.cpp
        src/WebSocketSession.cpp
        src/WebSocketSession.h
        src/SearchSystem.cpp
        src/SearchSystem.h
        src/Ollama.cpp
        src/Ollama.h
        src/InvertedIndex.cpp
        src/InvertedIndex.h
        src/ThreadPool.cpp
        src/ThreadPool.h)

target_include_directories(course_work_parallel_computing PRIVATE src)
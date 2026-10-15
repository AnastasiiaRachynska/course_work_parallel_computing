cmake_minimum_required(VERSION 3.20)
project(course_work_parallel_computing CXX)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(course_work_parallel_computing
        src/main.cpp
        src/ApiHandler.cpp
        src/ApiHandler.h
        src/WebSocketSession.cpp
        src/WebSocketSession.h
        src/SearchSystem.cpp
        src/SearchSystem.h
        src/Ollama.cpp
        src/Ollama.h
        src/InvertedIndex.cpp
        src/InvertedIndex.h
        src/ThreadPool.cpp
        src/ThreadPool.h
)

target_link_libraries(course_work_parallel_computing PRIVATE nlohmann_json::nlohmann_json)
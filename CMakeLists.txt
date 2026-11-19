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
FetchContent_Declare(
        cpp_httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.66.0
        GIT_SHALLOW TRUE
)
FetchContent_Declare(
        ixwebsocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG v12.1.0
        GIT_SHALLOW TRUE
)
set(USE_ZLIB OFF CACHE BOOL "Disable IXWebSocket compression")
FetchContent_MakeAvailable(nlohmann_json cpp_httplib ixwebsocket)

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

target_link_libraries(course_work_parallel_computing PRIVATE nlohmann_json::nlohmann_json httplib::httplib ixwebsocket::ixwebsocket)

add_executable(search src/client.cpp)
target_link_libraries(search PRIVATE nlohmann_json::nlohmann_json ixwebsocket::ixwebsocket)

add_executable(try_index
    tests/try-index.cpp
    src/SearchSystem.cpp
    src/InvertedIndex.cpp
    src/Ollama.cpp
    src/ThreadPool.cpp
)
target_link_libraries(try_index PRIVATE nlohmann_json::nlohmann_json httplib::httplib ixwebsocket::ixwebsocket)

add_executable(time_threads
    tests/time-threads.cpp
    src/SearchSystem.cpp
    src/InvertedIndex.cpp
    src/Ollama.cpp
    src/ThreadPool.cpp
)

target_link_libraries(time_threads PRIVATE nlohmann_json::nlohmann_json httplib::httplib ixwebsocket::ixwebsocket)

add_executable(try_ollama tests/try-ollama.cpp src/Ollama.cpp)
target_link_libraries(try_ollama PRIVATE
    nlohmann_json::nlohmann_json
    httplib::httplib
)
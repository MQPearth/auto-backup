cmake_minimum_required(VERSION 3.5)

project(auto-backup CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

set(SYSTEM_LIBS stdc++ pthread)

include_directories("${CMAKE_SOURCE_DIR}/thirdparty/include")

add_executable(${PROJECT_NAME} main.cc)

target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/thirdparty/lib/cos/libcossdk-shared.so
        ${CMAKE_SOURCE_DIR}/thirdparty/lib/Poco/libPocoFoundation.so
        ${CMAKE_SOURCE_DIR}/thirdparty/lib/Poco/libPocoJSON.so
        ${SYSTEM_LIBS})

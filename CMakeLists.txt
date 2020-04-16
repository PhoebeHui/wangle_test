cmake_minimum_required(VERSION 3.12)
project(Lion)
find_package(wangle  REQUIRED)
file(GLOB LION_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB LION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(Lion
${LION_HEADERS}
${LION_SOURCES}
)
target_link_libraries(Lion PRIVATE  wangle::wangle)
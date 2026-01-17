cmake_minimum_required(VERSION 3.16)
set (PROJECT_NAME ahb_adaptor)
project(${PROJECT_NAME} CXX)

set (CMAKE_PREFIX_PATH /opt/systemc)
set (SystemC_CXX_STANDARD 17)
set (SystemC_CXX_STANDARD_REQUIRED 11)
find_package(SystemCLanguage CONFIG REQUIRED)

set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD} CACHE STRING
    "C++ standard to build all targets. Minimum version is C++17.")
set (CMAKE_CXX_STANDARD_REQUIRED ${SystemC_CXX_STANDARD_REQUIRED} CACHE BOOL
    "The with CMAKE_CXX_STANDARD selected C++ standard is a requirement.")

file(GLOB SOURCE_FILES "*.cpp")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} SystemC::systemc)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
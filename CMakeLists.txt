cmake_minimum_required(VERSION 3.15)
project(miniXML LANGUAGES CXX)

add_library(miniXML INTERFACE)

target_include_directories(
    miniXML
    INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(miniXML INTERFACE cxx_std_17)
#***********************************************************************
# Copyright 2022 Massachusets Institute of Technology
# SPDX short identifier: BSD-2-Clause
#
# File Name:      CMakeLists.txt
# Program:        Common Evaluation Platform (CEP)
# Description:    CMake file
# Notes:          
#************************************************************************
cmake_minimum_required(VERSION 3.8)
project(gpiotest C)
set(CMAKE_C_STANDARD 11)

add_executable(gpiotest gpiotest.c)

target_include_directories(gpiotest PRIVATE)

target_link_libraries(gpiotest PRIVATE "gpiod")

set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Standard install prefix" FORCE)

install(TARGETS gpiotest RUNTIME DESTINATION bin)
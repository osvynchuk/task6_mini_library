cmake_minimum_required(VERSION 3.5)

project(task6_mini_library LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(task6_mini_library my_algo.h main.cpp)

##################################
# Download and install GoogleTest

include(ExternalProject)
ExternalProject_Add(gtest
  URL https://github.com/google/googletest/archive/master.zip
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(gtest source_dir binary_dir)
message("Gtest source_dir: " ${source_dir})
message("Gtest binary_dir: " ${binary_dir})

################
# Define a test
add_executable(task6_mini_library_TEST algo_lib_test.cpp)

######################################
# Configure the test to use GoogleTest
#
add_dependencies(task6_mini_library_TEST gtest)
include_directories(${source_dir}/googletest/include)
target_link_libraries(task6_mini_library_TEST ${binary_dir}/lib/libgtest.a)
target_link_libraries(task6_mini_library_TEST ${binary_dir}/lib/libgtest_main.a)

##################################
# Just make the test runnable with
#   $ make test

enable_testing()
add_test(NAME    task6_mini_library_TEST
         COMMAND task6_mini_library_TEST)

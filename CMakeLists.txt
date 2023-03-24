project(mympilib C CXX)
cmake_minimum_required(VERSION 3.23)

cmake_policy(SET CMP0074 NEW)

Find_package(MPI REQUIRED)
find_package(Boost REQUIRED)

add_library(mpinterposition SHARED interposition.cc)
target_link_libraries(mpinterposition PUBLIC MPI::MPI_C Boost::boost)

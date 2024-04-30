cmake_minimum_required(VERSION 3.26)
project(net_4_4_1_postgresql)

set(CMAKE_CXX_STANDARD 20)
set(PostgreSQL_ADDITIONAL_VERSIONS "16")

add_subdirectory("C:/Programs/dev/libpqxx-7.9.0" libpqxx-build)

add_executable(net_4_4_1_postgresql main.cpp
                                    db_users.h db_users.cpp)
target_compile_features(net_4_4_1_postgresql PRIVATE cxx_std_20)
target_link_libraries(net_4_4_1_postgresql pqxx)


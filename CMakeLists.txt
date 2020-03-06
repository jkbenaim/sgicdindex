cmake_minimum_required(VERSION 3.15)
project(sgicdserv C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check -Og -ggdb")

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(ulfius REQUIRED IMPORTED_TARGET libulfius)
pkg_search_module(yder REQUIRED IMPORTED_TARGET libyder)
pkg_search_module(systemd REQUIRED IMPORTED_TARGET libsystemd)

add_executable(sgicdserv
        escape.c
	escape.h
	sa.c
	sa.h
	sgicdserv.c)

target_link_libraries(sgicdserv PUBLIC
        sqlite3
	PkgConfig::ulfius
	PkgConfig::yder
	PkgConfig::systemd
	)
cmake_minimum_required(VERSION 2.8)

project (script-pumpr)

set (CMAKE_CXX_FLAGS "-std=c++1y ${CMAKE_CXX_FLAGS}")

include_directories(extern/enet/include)
add_subdirectory(extern/enet)

include_directories(extern/fmt)
list (APPEND fmt_src
	extern/fmt/fmt/format.cc
	)

include_directories(extern/network-host)
include_directories(extern/rdestl)
include_directories(extern/logger)

find_package(Boost COMPONENTS system filesystem REQUIRED)

add_executable(script-pump script-pump.cpp ${fmt_src})

target_link_libraries(script-pump ${Boost_LIBRARIES} enet)


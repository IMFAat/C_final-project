cmake_minimum_required(VERSION 3.0)
project(GTKTutorial)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(GTK3_INCLUDE_DIRS "C:/msys64/mingw64/include/")
set(GTK3_LIBRARY_DIRS "C:/msys64/mingw64/lib")
set(ENV{PKG_CONFIG_PATH} "C:/msys64/mingw64/lib/pkgconfig")
include_directories("C:\\sqlite")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})
add_definitions(${GTK4_CFLAGS_OTHER})
add_definitions(-DROOT_PATH="${CMAKE_BINARY_DIR}")
add_custom_target(copy_ui_file ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/src
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src
)

add_executable(TutorialApp main.c
                Screens/MainScreen/MainScreen.c
                Screens/MainScreen/MainScreen.h
                sqlite/daily/daily.c
                sqlite/daily/daily.h
                sqlite/weekly/weekly.c
                sqlite/weekly/weekly.h
)
SET(CMAKE_EXE_LINKER_FLAGS "-lsqlite3 -LC:\\sqlite")
SET(CMAKE_SHARED_LINKER_FLAGS "-lsqlite3 -LC:\\sqlite" )
#set_target_properties(TutorialApp PROPERTIES WIN32_EXECUTABLE true)
target_link_libraries(TutorialApp ${GTK4_LIBRARIES})
# C-program
CMakeList.txt
```aiignore
add_definitions(-DROOT_PATH="${CMAKE_BINARY_DIR}")
```
```aiignore
add_custom_target(copy_ui_file ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/src
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src
)
```

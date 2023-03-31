cmake_minimum_required(VERSION 3.24)
project(gtk3-opengl C)

# set(CMAKE_C_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE on)

add_compile_options(-std=c99 -pthread)
add_link_options(-lm -pthread)

find_package(PkgConfig REQUIRED)

# pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GTK3 REQUIRED gtk4)
pkg_check_modules(GL REQUIRED gl)
include_directories(
        ${GTK3_INCLUDE_DIRS}
        ${GL_INCLUDE_DIRS}
        )

link_libraries(
        ${GTK3_LIBRARIES}
        ${GL_LIBRARIES}
        )

file(GLOB GTK3_OPENGL_SRC *.c)

add_executable(${PROJECT_NAME} ${GTK3_OPENGL_SRC})

target_compile_definitions(${PROJECT_NAME} PUBLIC GL_GLEXT_PROTOTYPES)

file(COPY shaders DESTINATION ./)
file(COPY textures DESTINATION ./)

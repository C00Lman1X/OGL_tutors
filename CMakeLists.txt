cmake_minimum_required(VERSION 3.5.1)

project(GLFW_TMP)
add_executable(GLFW_TMP main.cpp GLAD/src/glad.c)

include_directories(${PROJECT_SOURCE_DIR}/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/GLAD/include)


if(WIN32)
    message("WIN32!")
    find_library(GLFW_LIB_DEBUG glfw3 HINTS ${PROJECT_SOURCE_DIR}/glfw/build/src/Debug)
    find_library(GLFW_LIB_RELEASE glfw3 HINTS ${PROJECT_SOURCE_DIR}/glfw/build/src/Release)
    target_link_libraries(GLFW_TMP debug ${GLFW_LIB_DEBUG})
    target_link_libraries(GLFW_TMP optimized ${GLFW_LIB_RELEASE})
    target_link_libraries(GLFW_TMP opengl32)
else()
    message("Unix!")
    find_library(GLFW_LIB glfw3 HINTS ${PROJECT_SOURCE_DIR}/glfw/build/src)
    find_library(OGL_LIB GL /usr/lib/x86_64-linux-gnu)
    target_link_libraries(GLFW_TMP ${GLFW_LIB} ${OGL_LIB} dl X11 Xrandr Xi Xxf86vm Xinerama Xcursor pthread)
endif()

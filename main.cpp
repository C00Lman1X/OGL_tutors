#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* wnd, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char ** argv)
{
    GLFWwindow* wnd;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);

    wnd = glfwCreateWindow(640, 480, "Yo, GLFW!", NULL, NULL);
    if (!wnd)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(wnd);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 640, 480);
    glfwSetFramebufferSizeCallback(wnd, framebuffer_size_callback);

    while(!glfwWindowShouldClose(wnd))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(wnd);

        glfwPollEvents();

    }

    glfwTerminate();
    return 0;
}
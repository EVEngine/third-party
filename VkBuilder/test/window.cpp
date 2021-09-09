#include "window.hpp"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vulkan/vulkan.h>

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
}

GLFW_Window::GLFW_Window() {}

GLFW_Window::~GLFW_Window() {}

void *GLFW_Window::getWindow() { return window; }

void GLFW_Window::createWindow()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW\n");

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(1366, 768, "Test Window", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window\n");
    }

    // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

extern void onRender();

void GLFW_Window::mainLoop()
{
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // poll IO events and handle them (keys pressed/released, mouse moved etc.)
        processInput();
        onRender();
    }
    cleanUp();
}


void GLFW_Window::processInput()
{
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void GLFW_Window::cleanUp()
{
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwDestroyWindow(window);
    glfwTerminate();
}

extern void *create_surface_glfw(void *instance, void *window)
{
    VkSurfaceKHR surface = nullptr;
    VkResult err = glfwCreateWindowSurface((VkInstance)instance, (GLFWwindow *)window, NULL, &surface);
    if (err)
    {
        const char *error_msg;
        int ret = glfwGetError(&error_msg);
        throw std::runtime_error(error_msg);
    }
    return (void *)surface;
}
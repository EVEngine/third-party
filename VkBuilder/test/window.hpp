#pragma once

class GLFW_Window {
public:
    GLFW_Window();
    virtual ~GLFW_Window();
    virtual void createWindow();
    virtual void mainLoop();
    virtual void* getWindow();

protected:
    struct GLFWwindow* window;
    
    virtual void cleanUp();
    virtual void processInput();
};

extern void* create_surface_glfw (void* instance, void* window);
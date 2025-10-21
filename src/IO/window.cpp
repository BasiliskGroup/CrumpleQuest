#include "IO/window.h"

/**
 * @brief Callback for window resize. 
 *        Sets to the framebuffer size for mac compatibility. 
 * 
 * @param window 
 * @param width 
 * @param height 
 */
inline void windowResize(GLFWwindow* window, int width, int height) {
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
}

/**
 * @brief Enables X11 window if the user is on Wayland. 
 *        Fixes the borderless window error on some linux devices. 
 * 
 */
void enableX11() {
    const char* sessionType = getenv("XDG_SESSION_TYPE");    
    if (sessionType && std::string(sessionType) == "wayland") {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    }
}

/**
 * @brief Sets the window hint for ther GL version.
 *        Must be set before the window is created.  
 * 
 * @param major Major version
 * @param minor Sub-Version within the major version
 */
void setGLVersion(unsigned int major, unsigned int minor) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

/**
 * @brief Checks that the window was sucessfully created
 * 
 * @param window Pointer to the GLFWwindow to check
 * @return true 
 * @return false 
 */
bool confirmGLFW(GLFWwindow* window) {
    if (window != NULL) { return true; }
    
    // Default GLFW fail message
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
}

/**
 * @brief Checks that the GLAD was sucessfully initialized
 * 
 * @return true 
 * @return false 
 */
bool confirmGLAD() {
    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return true; }

    // Default GLAD fail message
    std::cout << "Failed to initialize GLAD" << std::endl;
    return false;
}

/**
 * @brief Construct a new Window object with GLFW.
 * 
 * @param width The initial width of the window in pixels
 * @param height The initial height of the window in pixels
 * @param title 
 */
Window::Window(int width, int height, const char* title) {
    // Initialize GLFW with OpenGL
    enableX11();
    glfwInit();
    setGLVersion(3, 3);

    // Create the window and confirm proper initialization
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!confirmGLFW(window)) { return; }
    glfwMakeContextCurrent(window);
    if (!confirmGLAD()) { return; }

    // Draws closer objects in front of futher objects
    glEnable(GL_DEPTH_TEST);  
    // Does not draw faces that are not facing the camera
    // glEnable(GL_CULL_FACE);
    // Antialiasing 
    glEnable(GL_MULTISAMPLE);

    // Set the resize callback
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, windowResize);

    // Fix mac
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
}

/**
 * @brief Checks if the window close flag has been set.
 * 
 * @return true 
 * @return false 
 */
bool Window::isRunning() {
    glfwPollEvents();
    return !glfwWindowShouldClose(window);
}

/**
 * @brief Swaps the window buffers, showing all draws in the last frame.
 * 
 */
void Window::render() {
    glfwSwapBuffers(window);
}

/**
 * @brief Clears the window with the given color. 
 * 
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @param a Alpha component
 */
void Window::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief Destroy the Window object and terminates GLFW
 * 
 */
Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

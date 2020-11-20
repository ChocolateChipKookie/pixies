//
// Created by kookie on 27/03/2020.
//

#include "Context.h"
#include "util/graphics_error.h"

/////////////////////////////////////////////////////////////////
//  Constructors, destructors and default callback functions   //
/////////////////////////////////////////////////////////////////

kki::Context::Context(int width,
                      int height,
                      const std::string &name,
                      const std::function<void()> &window_hints,
                      const std::function<void()> &window_options) {
    // Init GLFW window
    glfwInit();
    // Set window hints
    window_hints();

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        throw graphics_error{ "Failed to create GLFW window" };
    }

    // Set window
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        throw graphics_error{ "Failed to create GLFW window" };
    }

    // Set window options
    window_options();
    this->context_ = window;
}

void kki::Context::default_hints() {
    // Needed window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
}

void kki::Context::default_options() {
}

void kki::Context::apply_options(const std::function<void()> &window_options) {
    this->bind();
    window_options();
}

void kki::Context::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void kki::Context::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

kki::Context::Context(int width, int height, const std::string &name, bool depth, bool alpha) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

    if (window == nullptr)
    {
        glfwTerminate();
        throw graphics_error{ "Failed to create GLFW window" };
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        throw graphics_error{ "Failed to create GLFW window" };
    }

    glDisable(GL_LIGHTING);
    if(depth){
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    if (alpha)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    this->context_ = window;
}

kki::Context::~Context()
{
    glfwDestroyWindow(context_);
}

//
//  Handling functions
//

bool kki::Context::should_close() const
{
    return glfwWindowShouldClose(context_);
}

void kki::Context::set_should_close(bool value) const
{
    glfwSetWindowShouldClose(context_, value);
}

void kki::Context::swap_buffers() const
{
    glfwSwapBuffers(context_);
}

void kki::Context::poll_events()
{
    glfwPollEvents();
}

void kki::Context::wait_events(double seconds)
{
    if(seconds < 0.){
        glfwWaitEvents();
    }
    else{
        glfwWaitEventsTimeout(seconds);
    }
}

void kki::Context::set_viewport(int width, int height) const {
    glViewport(0, 0, width, height);
}

void kki::Context::reset_viewport() const
{
    int width, height;
    glfwGetWindowSize(context_, &width, &height);
    set_viewport(width, height);
}

void kki::Context::resize(int width, int height) const
{
    glfwSetWindowSize(context_, width, height);
}

unsigned kki::Context::get_width() const
{
    int width, height;
    glfwGetWindowSize(context_, &width, &height);
    return width;
}

unsigned kki::Context::get_height() const
{
    int width, height;
    glfwGetWindowSize(context_, &width, &height);
    return height;
}

void kki::Context::set_process_input(void(*function)(GLFWwindow*, int, int, int, int)) const
{
    glfwSetKeyCallback(context_, function);
}

void kki::Context::set_resize_callback(void(*function)(GLFWwindow*, int, int)) const {
    glfwSetFramebufferSizeCallback(context_, function);
}

void kki::Context::bind()
{
    glfwMakeContextCurrent(context_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void kki::Context::set_clear_colour(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void kki::Context::set_clear_colour(const glm::vec4& colour) {
    glClearColor(colour.r, colour.g, colour.b, colour.a);
}

void kki::Context::set_clear_colour(const glm::vec3& colour) {
    glClearColor(colour.r, colour.g, colour.b, 1.f);
}





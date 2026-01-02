#include "../headers/inputManager.h"
#include "../headers/engine.h"

void InputManager::keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key > 348) {
        return;
    }

    if (action == GLFW_PRESS)
        Engine::pressed[key] = true;
    else if (action == GLFW_RELEASE)
        Engine::pressed[key] = false;
}

void InputManager::initInputMode(bool sticky, bool lockKeys, bool disableCursor, bool hiddenCursor,
    bool mouseSticky, GLFWwindow* window) {

    Engine::pressed.resize(349, false);

    if (sticky) {
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    }

    if (lockKeys) {
        glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    }

    if (disableCursor) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (hiddenCursor) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    if (mouseSticky) {
        glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    }

    glfwSetKeyCallback(Engine::window, InputManager::keyCallBack);
    glfwSetCursorEnterCallback(Engine::window, InputManager::mouseEnterCallBack);
    glfwSetMouseButtonCallback(Engine::window, InputManager::mouseButtonCallBack);
    glfwSetScrollCallback(Engine::window, InputManager::mouseScrollCallBack);
    glfwSetCursorPosCallback(Engine::window, InputManager::cursorPosCallBack);
}

void InputManager::resetCursor(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void InputManager::mouseEnterCallBack(GLFWwindow* window, int entered) {
    if (entered)
    {
        // The cursor entered the content area of the window
    }
    else
    {
        // The cursor left the content area of the window
    }
}

void InputManager::mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods) {

    if (button > 348) {
        return;
    }

    if (action == GLFW_PRESS) {
        Engine::pressed[button] = true;
    }
    else if (action == GLFW_RELEASE) {
        Engine::pressed[button] = false;

        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            Engine::rightMouseFirstPress = true;
        }
    }
}

void InputManager::mouseScrollCallBack(GLFWwindow* window, double xoffset, double yoffset) {

    if (yoffset != 0) {
        Engine::camera.setAcceleration(Engine::camera.acceleration += (0.05f) * (float)yoffset);
    }

    if (xoffset != 0) {
        Engine::camera.setFOV(Engine::camera.fov += 2.0f * (float)xoffset);
    }
}

void InputManager::cursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {

    if (!Engine::rightMouseFirstPress) {
        Engine::camera.yaw += ((float)xpos - (float)Engine::lastKnownMousePos.first) * Engine::camera.sensitivity;
        Engine::camera.pitch += ((float)Engine::lastKnownMousePos.second - (float)ypos) * Engine::camera.sensitivity;

        Engine::lastKnownMousePos.first = xpos;
        Engine::lastKnownMousePos.second = ypos;

        Engine::camera.pitch = glm::clamp(Engine::camera.pitch, -89.0f, 89.0f);
    }
    else {

        Engine::lastKnownMousePos.first = xpos;
        Engine::lastKnownMousePos.second = ypos;
    }
}

void InputManager::consumeInput(GLFWwindow* window) {

    bool reduceCameraSpeed = true;

    for (int i = 0; i < (int)Engine::pressed.size(); i++) {

        if (!Engine::pressed[i]) continue;

        switch (i) {
        case GLFW_KEY_W:
            Engine::camera.velocity += Engine::camera.front * Engine::camera.acceleration;
            reduceCameraSpeed = false;
            break;
        case GLFW_KEY_S:
            Engine::camera.velocity -= Engine::camera.front * Engine::camera.acceleration;
            reduceCameraSpeed = false;
            break;
        case GLFW_KEY_A:
            Engine::camera.velocity -= Engine::camera.right * Engine::camera.acceleration;
            reduceCameraSpeed = false;
            break;
        case GLFW_KEY_D:
            Engine::camera.velocity += Engine::camera.right * Engine::camera.acceleration;
            reduceCameraSpeed = false;
            break;
        case GLFW_KEY_R:
            Engine::camera.velocity += Engine::camera.up * Engine::camera.acceleration;
            reduceCameraSpeed = false;
            break;
        case GLFW_KEY_F:
            Engine::camera.velocity -= Engine::camera.up * Engine::camera.acceleration;
            reduceCameraSpeed = false;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (Engine::rightMouseFirstPress) {
                Engine::rightMouseFirstPress = false;
            }
            break;
        case GLFW_KEY_SPACE:
            Engine::displayGUI = !Engine::displayGUI;
            Engine::pressed[i] = false;
        }
    }

    Engine::camera.position += Engine::camera.velocity;

    if (reduceCameraSpeed) {
        Engine::camera.velocity -= Engine::camera.velocity * 0.7f;
    }

    float speed = glm::length(Engine::camera.velocity);
    if (speed > Engine::camera.maxSpeed)
    {
        Engine::camera.velocity = (Engine::camera.velocity / speed) * Engine::camera.maxSpeed;
    }
};
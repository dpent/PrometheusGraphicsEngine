#include "../headers/inputManager.h"
#include "../headers/engine.h"


using namespace Prometheus;

namespace Prometheus
{
    
    void InputManager::keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods){

        glm::vec3 cameraPosChange = glm::vec3(0.0f);
        //float rollChange = 0.0f;

        if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            cameraPosChange += Engine::camera.front * Engine::camera.velocity;
        }

        if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            cameraPosChange -= Engine::camera.front * Engine::camera.velocity;
        }

        if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            cameraPosChange -= Engine::camera.right * Engine::camera.velocity;
        }

        if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            cameraPosChange += Engine::camera.right * Engine::camera.velocity;
        }

        if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            cameraPosChange += Engine::camera.up * Engine::camera.velocity;
        }

        if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            cameraPosChange -= Engine::camera.up * Engine::camera.velocity;
        }
        
        /*if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            rollChange += 1.0f;
        }

        if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            rollChange -= 1.0f;
        }*/

        Engine::camera.position+=cameraPosChange;
        //Engine::camera.roll+=rollChange;

        //if(rollChange!=0){
            //Engine::updateCameraVectors = true;
        //}
    }   

    void InputManager::initInputMode(bool sticky, bool lockKeys, bool disableCursor, bool hiddenCursor,
        bool mouseSticky, GLFWwindow* window){

        if(sticky){
            glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
        }

        if(lockKeys){
            glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
        }

        if(disableCursor){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        if(hiddenCursor){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }

        if(mouseSticky){
            glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
        }
    }

    void InputManager::resetCursor(GLFWwindow* window){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void InputManager::mouseEnterCallBack(GLFWwindow* window, int entered){
        if (entered)
        {
            // The cursor entered the content area of the window
        }
        else
        {
            // The cursor left the content area of the window
        }
    }

    void InputManager::mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods){
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){

            if(Engine::rightMouseFirstPress){

                Engine::rightMouseFirstPress=false;
            }
            
        }

        if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
            Engine::rightMouseFirstPress=true;
        }
    }

    void InputManager::mouseScrollCallBack(GLFWwindow* window, double xoffset, double yoffset){
        
        if(yoffset!=0){
            Engine::camera.setVelocity(Engine::camera.velocity+=(0.1)*yoffset);
        }
    }

    void InputManager::cursorPosCallBack(GLFWwindow* window, double xpos, double ypos){

        if(!Engine::rightMouseFirstPress){
            Engine::camera.yaw += (xpos - Engine::lastKnownMousePos.first) * Engine::camera.sensitivity;
            Engine::camera.pitch += (Engine::lastKnownMousePos.second - ypos) * Engine::camera.sensitivity;

            Engine::lastKnownMousePos.first = xpos;
            Engine::lastKnownMousePos.second = ypos;

            if (Engine::camera.pitch > 89.0f)
                Engine::camera.pitch = 89.0f;
            if (Engine::camera.pitch < -89.0f)
                Engine::camera.pitch = -89.0f;

            
            Engine::updateCameraVectors = true;
        }else{

            Engine::lastKnownMousePos.first = xpos;
            Engine::lastKnownMousePos.second = ypos;
        }
    }
}

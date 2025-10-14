#include "../headers/inputManager.h"
#include "../headers/engine.h"
#include <glm/common.hpp>


using namespace Prometheus;

namespace Prometheus
{
    
    void InputManager::keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods){

        if(key>348){
            return ;
        }

        if (action == GLFW_PRESS)
            Engine::pressed[key] = true;
        else if (action == GLFW_RELEASE)
            Engine::pressed[key] = false;

        //glm::vec3 cameraPosChange = glm::vec3(0.0f);
        //float rollChange = 0.0f;

        /*if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)){
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
        }*/
        
        /*if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            rollChange += 1.0f;
        }

        if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            rollChange -= 1.0f;
        }*/

        //Engine::camera.position+=cameraPosChange;
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

        if(button > 348){
            return;
        }

        if (action == GLFW_PRESS){
            Engine::pressed[button] = true;
        }else if (action == GLFW_RELEASE){
            Engine::pressed[button] = false;

            if(button == GLFW_MOUSE_BUTTON_RIGHT){
                Engine::rightMouseFirstPress=true;
            }
        }   
        /*if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){

            if(Engine::rightMouseFirstPress){

                Engine::rightMouseFirstPress=false;
            }
            
        }

        if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
            Engine::rightMouseFirstPress=true;
        }*/
    }

    void InputManager::mouseScrollCallBack(GLFWwindow* window, double xoffset, double yoffset){
        
        if(yoffset!=0){
            Engine::camera.setVelocity(Engine::camera.velocity+=(0.05f)*yoffset);
        }
    }

    void InputManager::cursorPosCallBack(GLFWwindow* window, double xpos, double ypos){

        if(!Engine::rightMouseFirstPress){
            Engine::camera.yaw += (xpos - Engine::lastKnownMousePos.first) * Engine::camera.sensitivity;
            Engine::camera.pitch += (Engine::lastKnownMousePos.second - ypos) * Engine::camera.sensitivity;

            Engine::lastKnownMousePos.first = xpos;
            Engine::lastKnownMousePos.second = ypos;
    
            glm::clamp(Engine::camera.pitch,-89.0f,89.0f);

            Engine::updateCameraVectors = true;

        }else{

            Engine::lastKnownMousePos.first = xpos;
            Engine::lastKnownMousePos.second = ypos;
        }
    }

    void InputManager::consumeInput(GLFWwindow* window){

        Engine::cameraChangePos.x = 0.0f;
        Engine::cameraChangePos.y = 0.0f;
        Engine::cameraChangePos.z = 0.0f;

        for (int i = 0; i < (int)Engine::pressed.size(); i++) {

            if(!Engine::pressed[i]) continue;
            
            switch(i) {
                case GLFW_KEY_W:
                    Engine::cameraChangePos += Engine::camera.front * Engine::camera.velocity;
                    break;
                case GLFW_KEY_S:
                    Engine::cameraChangePos -= Engine::camera.front * Engine::camera.velocity;
                    break;
                case GLFW_KEY_A:
                    Engine::cameraChangePos -= Engine::camera.right * Engine::camera.velocity;
                    break;
                case GLFW_KEY_D:
                    Engine::cameraChangePos += Engine::camera.right * Engine::camera.velocity;
                    break;
                case GLFW_KEY_R:
                    Engine::cameraChangePos += Engine::camera.up * Engine::camera.velocity;
                    break;
                case GLFW_KEY_F:
                    Engine::cameraChangePos -= Engine::camera.up * Engine::camera.velocity;
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    if(Engine::rightMouseFirstPress){
                        Engine::rightMouseFirstPress=false;
                    }
                    break;
                
            }
        }

        Engine::camera.position += Engine::cameraChangePos;
    }
}

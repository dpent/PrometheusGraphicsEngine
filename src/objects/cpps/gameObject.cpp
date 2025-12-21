#include "../../engine/headers/engine.h"
#include <semaphore>
#include <vulkan/vulkan_core.h> 
#include <sstream>
#include "../../engine/headers/modelManager.h"
#include "../../physics/headers/collision.h"

using namespace Prometheus;

namespace Prometheus{
    uint64_t GameObject::autoIncrementId=0;

    GameObject::GameObject(std::string texturePath, std::string modelPath)
    {

        Engine::gameObjectMutex.lock();
        this->id = GameObject::autoIncrementId;
        GameObject::autoIncrementId++;
        Engine::gameObjectMutex.unlock();

        velocity = glm::vec3(0.0f);

        this->transform = Transform();
        this->texturePath = texturePath;
        this->meshPath = modelPath;

        this->info = new InstanceInfo();
    }

    GameObject::~GameObject(){
    }

    void GameObject::terminate(VkDevice& device){ //Used for object deletion
        Engine::textureMutex.lock();
        
        if (Engine::textureMap.count(texturePath)!=0) {
            Engine::textureMap[texturePath].count--;
            if(Engine::textureMap[texturePath].count==0){
                
                Engine::textureQueuedMutex.lock();
                Engine::texturesQueuedForDeletion[texturePath].push_back(std::move(Engine::textureMap[texturePath]));
                Engine::framesSinceTextureQueuedForDeletion[texturePath].push_back(0);
                Engine::textureQueuedMutex.unlock();
                
                Engine::textureMap.erase(texturePath);
            } 
        }
        Engine::textureMutex.unlock();

        Engine::gameObjectMutex.lock();

        if(prev != nullptr){
            if(next != nullptr){
                this->prev->next = this->next;
            }else{
                this->prev->next = nullptr;
            }
        }

        if(next != nullptr){
            if(prev != nullptr){
                this->next->prev = this->prev;
            }else{
                this->next->prev = nullptr;
            }
        }

        Engine::objectDQueue.size --;

        Engine::gameObjectMutex.unlock();
    }

    void GameObject::draw(VkCommandBuffer& commandBuffer, uint32_t instanceCount, uint32_t firstInstance){
        
        if(Engine::meshMap[this->meshPath].indexOffset==3200171710
            && Engine::meshMap[this->meshPath].indices.size()==0
            && Engine::meshMap[this->meshPath].vertexOffset==3200171710
            && Engine::meshMap[this->meshPath].vertices.size()==0)
        {
            return;
        }
        //std::cout<<Engine::meshMap[this->meshPath].toString()<<std::endl;
        vkCmdDrawIndexed(commandBuffer, Engine::meshMap[this->meshPath].indices.size(), 
            instanceCount, Engine::meshMap[this->meshPath].indexOffset, Engine::meshMap[this->meshPath].vertexOffset, firstInstance);
    }

    GameObject::GameObject(){

    }

    std::string GameObject::toString() {
        std::ostringstream oss;
        oss << "GameObject { "
            << "id=" << id << ", "
            << "autoIncrementId=" << autoIncrementId << ", "
            << "texturePath=\"" << (texturePath=="" ? texturePath : "null") << "\", "
            << "meshPath=\"" << meshPath << "\" }";
        return oss.str();
    }

    void GameObject::animateCircularMotion(float centerX, float centerY, float centerZ, float radius, float speed, float offset){
        float time   = glfwGetTime(); // or your own frame timer
        time+=offset;

        float x = centerX + radius * cos(time * speed);
        float y = centerY + radius * sin(time * speed);             // keep same height
        float z = centerZ + radius * sin(time * speed);

        velocity = glm::vec3(x,y,z) - transform.position;
    }

    void GameObject::createObjectThreaded(VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkQueue& graphicsQueue, GameObject* obj
    ){
        CreateObjectJob* j = new CreateObjectJob();
        j->data.emplace_back(std::in_place_type<GameObject*>, obj);
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j->data.emplace_back(std::in_place_type<VkPhysicalDevice*>, &physicalDevice);
        j->data.emplace_back(std::in_place_type<VkQueue*>, &graphicsQueue);

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();

        Engine::workInQueueSemaphore.release();
    }

    void GameObject::deleteObjectThreaded(VkDevice &device, GameObject* object){

        DeleteObjectJob* j = new DeleteObjectJob();
        j->data.emplace_back(std::in_place_type<GameObject*>, object);
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();
        
        Engine::workInQueueSemaphore.release();
    }

    void GameObject::start(){
        scale(glm::vec3(15.0f));
    }

    void GameObject::update(){
        rotate(glm::angleAxis(0.1f,glm::vec3(0.0f,1.0f,0.0f)));
        //animateCircularMotion(0.0f,0.0f,0.0f,15.0f, id%(Engine::objectDQueue.size) * 0.1f + 0.1f,0.0f);
        move();
        //checkCollisions();
    }

    void GameObject::updateInstanceInfo(uint64_t textureIndex){
        info->modelMatrix = transform.getModelMatrix();
        info->textureIndex = textureIndex;
    }

    void GameObject::scale(glm::vec3 scale){

        this->transform.scale = scale;

        for(int i=0; i<8; i++){
            hitboxPoints[i] = transform.rotation * (mesh->hitboxPoints[i] * transform.scale);
        }

        center = transform.rotation * (getCenter() * transform.scale);
        radius = glm::length(hitboxPoints[1]);
    }

    void GameObject::rotate(glm::quat rotation){
        transform.rotation *= rotation;

        for(int i=0; i<8; i++){
            hitboxPoints[i] = rotation * hitboxPoints[i];
        }

        center = getCenter();
    }

    void GameObject::drawOBB(glm::vec3 color){ //Magenta default
        Engine::debugMutex.lock();
        Debug::drawLine(transform.position + hitboxPoints[1], 
            transform.position + hitboxPoints[3], color); //TOP PART
        Debug::drawLine(transform.position + hitboxPoints[1], 
            transform.position + hitboxPoints[0], color);
        Debug::drawLine(transform.position + hitboxPoints[2], 
            transform.position + hitboxPoints[3], color);
        Debug::drawLine(transform.position + hitboxPoints[2], 
            transform.position + hitboxPoints[0], color);

        Debug::drawLine(transform.position + hitboxPoints[1], 
            transform.position + hitboxPoints[5], color); //PERPENDICULAR PARTS
        Debug::drawLine(transform.position + hitboxPoints[3], 
            transform.position + hitboxPoints[7], color);
        Debug::drawLine(transform.position + hitboxPoints[0], 
            transform.position + hitboxPoints[4], color);
        Debug::drawLine(transform.position + hitboxPoints[2], 
            transform.position + hitboxPoints[6], color);

        Debug::drawLine(transform.position + hitboxPoints[6], 
            transform.position + hitboxPoints[4], color); //BOTTOM PART
        Debug::drawLine(transform.position + hitboxPoints[6], 
            transform.position + hitboxPoints[7], color);
        Debug::drawLine(
            transform.position + hitboxPoints[5], 
            transform.position + hitboxPoints[4], color);
        Debug::drawLine(transform.position + hitboxPoints[5], 
            transform.position + hitboxPoints[7], color);

        Engine::debugMutex.unlock();
    }

    glm::vec3 GameObject::hsvToRgb(float h, float s, float v)
    {
        float c = v * s;
        float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
        float m = v - c;

        float r, g, b;

        if (0 <= h && h < 60)      { r = c; g = x; b = 0; }
        else if (60 <= h && h < 120)  { r = x; g = c; b = 0; }
        else if (120 <= h && h < 180) { r = 0; g = c; b = x; }
        else if (180 <= h && h < 240) { r = 0; g = x; b = c; }
        else if (240 <= h && h < 300) { r = x; g = 0; b = c; }
        else                          { r = c; g = 0; b = x; }

        return glm::vec3(r + m, g + m, b + m);
    }

    glm::vec3 GameObject::getColorBasedOnTime(){
        float t = glfwGetTime(); // or your own time variable
        float hue = fmodf(t * 60.0f, 360.0f); // 60° per second
        return GameObject::hsvToRgb(hue, 1.0f, 1.0f);
    }

    glm::vec3 GameObject::updatePulse(float time)
    {

        float s = 15.0f + sin(time) * 4.0f;

        return glm::vec3(s);
    }

    bool GameObject::sphereTest(glm::vec3 center,glm::vec3 radius){

        float distSqr = glm::length2(center - this->center);
        float radiusSumSqr = glm::length2(hitboxPoints[1]) + glm::length2(radius);
        
        if(distSqr > radiusSumSqr){
            return false;
        }

        return true;
    }

    bool GameObject::checkCollisions(){

        moved = false;

        for(auto& cell : cells){
            for(auto& object : cell->objects){
                if(object->id == id){
                    continue;
                }

                if(sphereTest(object->getCenter(), object->hitboxPoints[1])){
                    if(Atlas::Collision::checkOBBtoOBB(object->transform.getBasicAxes(), 
                        this->transform.getBasicAxes(), 
                        object->getWorldHitpoints(), 
                        getWorldHitpoints()
                    )){
                        
                        object->drawOBB(COLOR_RED);
                        drawOBB(COLOR_RED);
                        return true;
                    }
                }
            }
        }

        return false;
    }

    std::array<glm::vec3, 8> GameObject::getWorldHitpoints(){

        std::array<glm::vec3, 8> worldPos = std::array<glm::vec3, 8>();

        for(int i=0; i<8; i++){
            worldPos[i] = hitboxPoints[i] + transform.position;
        }

        return worldPos;
    }

    glm::vec3 GameObject::getCenter(){
        return  (transform.rotation * ((mesh->center) * transform.scale)) + transform.position;
    }

    void GameObject::move(){

        if(velocity == glm::vec3(0.0f)){
            return;
        }
        transform.position += velocity;

        for(auto& cell: cells){
            cell->objectMutex.lock();
            cell->objects.erase(this);
            cell->objectMutex.unlock();
        }

        cells.clear();

        center = getCenter();

        moved = true;

        Engine::insertToHash(this);
    }

    void GameObject::instantiate(VkCommandPool& commandPool, VkDevice& device, VkPhysicalDevice& physicalDevice, 
        VkQueue& graphicsQueue, int req_comp) {

        Engine::textureMutex.lock();
        if (Engine::textureMap.count(texturePath) != 0) {

            Engine::textureMap[texturePath].count++;

        }
        else {
            Engine::textureMap.insert(std::make_pair(texturePath,
                Texture(texturePath, req_comp, device, physicalDevice, graphicsQueue, commandPool)));
        }

        Engine::textureMutex.unlock();

        Engine::meshMutex.lock();

        if (Engine::meshMap.count(this->meshPath) == 0 && Engine::meshesLoading.count(this->meshPath) == 0) {

            Engine::meshesLoading.insert(this->meshPath);

            std::binary_semaphore* meshLoadSemaphore = new std::binary_semaphore{ 0 }; //In case i use them sometime

            ModelManager::loadModel(this->meshPath, *meshLoadSemaphore); //Also inserts the mesh into meshMap
            glm::mat4 inverse = glm::inverse(transform.getModelMatrix());

            for (int i = 0; i < 8; i++) {
                hitboxPoints[i] = glm::vec3(inverse * glm::vec4(Engine::meshMap[this->meshPath].hitboxPoints[i], 1.0f));
            }
            Engine::recreateVertexIndexBuffer = true;

            delete meshLoadSemaphore;
        }

        if (Engine::meshesLoading.count(this->meshPath) == 0) {
            mesh = &(Engine::meshMap[this->meshPath]);

        }
        else {

            while (Engine::meshesLoading.count(this->meshPath) != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if (Engine::meshesLoading.count(this->meshPath) == 0) {
                    mesh = &(Engine::meshMap[this->meshPath]);
                    break;
                }
            }

        }

        Engine::meshMutex.unlock();
        center = getCenter();
        radius = glm::length(hitboxPoints[1]);
        Engine::insertToHash(this);

        Engine::gameObjectMutex.lock();

        //Engine::gameObjectMap.insert({this->id,this});
        Engine::objectsByMesh[this->meshPath][this->id] = this;
        Engine::objectDQueue.push(this);

        Engine::gameObjectMutex.unlock();

        this->start();
    }
}


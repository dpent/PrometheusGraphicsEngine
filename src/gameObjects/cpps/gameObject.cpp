#include "../headers/gameObject.h"
#include "../../core/headers/engine.h"
#include "../../threads/headers/job.h"

uint64_t GameObject::incrementID = 0;

GameObject::GameObject(){
	this->transform = new Transform();
}

GameObject::GameObject(Mesh* mesh, Material* material) {
	this->mesh = mesh;
	Engine::meshMutex.lock();
	this->mesh->instances++;
	Engine::meshMutex.unlock();

	this->material = material;

	this->transform = new Transform();

	Engine::objectCreateMutex.lock();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);

	Engine::objectCreateMutex.unlock();

}

GameObject::GameObject(Mesh* mesh, std::string textureFilename) {
	this->mesh = mesh;
	Engine::meshMutex.lock();
	this->mesh->instances++;
	Engine::meshMutex.unlock();

	this->material = new Material(textureFilename, 0.0f, 1.0f, Engine::command);

	this->transform = new Transform();

	Engine::objectCreateMutex.lock();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);

	Engine::objectCreateMutex.unlock();

}

GameObject::GameObject(std::string modelFilename, Material* material) {

	this->mesh = new Mesh(modelFilename);
	this->mesh->instances++;

	this->material = material;

	this->transform = new Transform();

	Engine::objectCreateMutex.lock();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);

	Engine::objectCreateMutex.unlock();

}

GameObject::GameObject(std::string modelFilename, std::string textureFilename) {

	this->mesh = new Mesh(modelFilename);
	this->mesh->instances++;

	this->material = new Material(textureFilename, 0.0f, 1.0f, Engine::command);

	this->transform = new Transform();

	Engine::objectCreateMutex.lock();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);

	Engine::objectCreateMutex.unlock();

	Engine::remakeInstanceDataSSBO = true;
}

void GameObject::update() {
	rotate(glm::angleAxis(0.01f, glm::vec3(0.0f, 1.0f, 0.0f)));
}

void GameObject::rotate(glm::quat rotation) {
	transform->rotation *= rotation;
}

void GameObject::scale(glm::vec3 scale) {

	this->transform->scale = scale;
}

void GameObject::initialise(InitInfo& info, CommandPool& command) {
	if (info.modelPointer == nullptr) {
		this->mesh = new Mesh(info.modelFilename);
		this->mesh->instances++;
	}
	else {
		this->mesh = info.modelPointer;
		Engine::meshMutex.lock();
		this->mesh->instances++;
		Engine::meshMutex.unlock();
	}

	if (info.materialPointer == nullptr) {
		this->material = new Material(info.textureFilename, 0.0f, 1.0f, command);
	}
	else {
		this->material = info.materialPointer;
	}
	
	//this->transform = new Transform(); //Already created by default const/tor

	Engine::objectCreateMutex.lock();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);

	Engine::objectCreateMutex.unlock();

	Engine::remakeInstanceDataSSBO = true;

}

void GameObject::createInitiasationJob(GameObject* obj, InitInfo* info) {

	InitialiseObjectJob* job = new InitialiseObjectJob(obj, *info);

	Engine::jobQueueMutex.lock();
	Engine::jobQueue.push(job);
	Engine::jobQueueMutex.unlock();

	Engine::jobInQueueSem.release();
}

InstanceInfo::InstanceInfo(GameObject* owner) {
	modelMatrix = glm::mat4(1.0f);
	materialIndex = 0;
	this->owner = owner;
}

std::string InstanceInfo::toString() {
	std::ostringstream oss;

	oss << "Material index: " << materialIndex << "\n"
		<< "Model matrix: \n" << printMatrix(modelMatrix);
	return oss.str();
}

InitInfo::InitInfo(){}

InitInfo::InitInfo(std::string modelFilename, Mesh* modelPointer, std::string textureFilename, Material* materialPointer) {
	this->modelFilename = modelFilename;
	this->modelPointer = modelPointer;
	this->textureFilename = textureFilename;
	this->materialPointer = materialPointer;
}
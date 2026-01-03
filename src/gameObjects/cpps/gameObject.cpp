#include "../headers/gameObject.h"
#include "../../core/headers/engine.h"

uint64_t GameObject::incrementID = 0;

GameObject::GameObject(){}

GameObject::GameObject(Mesh* mesh, Material* material) {
	this->mesh = mesh;
	this->mesh->instances++;

	this->material = material;

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);
}

GameObject::GameObject(Mesh* mesh, std::string textureFilename) {
	this->mesh = mesh;
	this->mesh->instances++;

	this->material = new Material(textureFilename, 0.0f, 1.0f);

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);
}

GameObject::GameObject(std::string modelFilename, Material* material) {
	this->mesh = new Mesh(modelFilename);
	this->mesh->instances++;

	this->material = material;

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);
}

GameObject::GameObject(std::string modelFilename, std::string textureFilename) {
	this->mesh = new Mesh(modelFilename);
	this->mesh->instances++;

	this->material = new Material(textureFilename, 0.0f, 1.0f);

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;

	Engine::instanceData.push_back(InstanceInfo(this));
	this->instanceIndex = static_cast<uint32_t>(Engine::instanceData.size() - 1);
	Engine::gameObjects.push_back(this);
}

void GameObject::update() {
	//rotate(glm::angleAxis(0.1f, glm::vec3(0.0f, 1.0f, 0.0f)));
}

void GameObject::rotate(glm::quat rotation) {
	transform->rotation *= rotation;
}

void GameObject::scale(glm::vec3 scale) {

	this->transform->scale = scale;
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
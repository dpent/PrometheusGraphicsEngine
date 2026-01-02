#include "../headers/gameObject.h"

uint64_t GameObject::incrementID = 0;

GameObject::GameObject(){}

GameObject::GameObject(Mesh* mesh, Material* material) {
	this->mesh = mesh;
	this->mesh->instances++;

	this->material = material;

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;
}

GameObject::GameObject(Mesh* mesh, std::string textureFilename) {
	this->mesh = mesh;
	this->mesh->instances++;

	this->material = new Material(textureFilename, 0.0f, 1.0f);

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;
}

GameObject::GameObject(std::string modelFilename, Material* material) {
	this->mesh = new Mesh(modelFilename);
	this->mesh->instances++;

	this->material = material;

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;
}

GameObject::GameObject(std::string modelFilename, std::string textureFilename) {
	this->mesh = new Mesh(modelFilename);
	this->mesh->instances++;

	this->material = new Material(textureFilename, 0.0f, 1.0f);

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;
}
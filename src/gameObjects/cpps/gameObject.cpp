#include "../headers/gameObject.h"

uint64_t GameObject::incrementID = 0;

GameObject::GameObject(){}

GameObject::GameObject(Mesh* mesh) {
	this->mesh = mesh;
	this->mesh->instances++;

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;
}

GameObject::GameObject(std::string modelFilename) {
	this->mesh = new Mesh(modelFilename);
	this->mesh->instances++;

	this->transform = new Transform();

	this->id = GameObject::incrementID;

	GameObject::incrementID++;
}
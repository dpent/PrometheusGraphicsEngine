#include "../headers/gameObject.h"


GameObject::GameObject(){}

GameObject::GameObject(Mesh* mesh) {
	this->mesh = mesh;
}

GameObject::GameObject(std::string modelFilename) {
	this->mesh = new Mesh(modelFilename);
}
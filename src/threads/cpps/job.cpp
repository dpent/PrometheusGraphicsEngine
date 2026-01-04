#include "../headers/job.h"

Job::Job() {
}

Job::~Job() {
}

void Job::execute() {
	std::cout << "This is a base job. It only prints this to the console." << std::endl;
}

std::string Job::toString() {

	return "Base Job. No attributes.";
}

std::string Job::humanReadableName() {
	
	return "Job";
}


PrepareForJoinJob::PrepareForJoinJob() {
}

PrepareForJoinJob::~PrepareForJoinJob() {
}

void PrepareForJoinJob::execute() {
}

std::string PrepareForJoinJob::toString() {

	return "PrepareForJoinJob. No attributes.";
}

std::string PrepareForJoinJob::humanReadableName() {

	return "PrepareForJoinJob";
}

InitialiseObjectJob::InitialiseObjectJob(GameObject* object, InitInfo info) {
	this->object = object;
	this->info = info;
}

InitialiseObjectJob::~InitialiseObjectJob(){}

void InitialiseObjectJob::execute() {
	object->initialise(info);
}

std::string InitialiseObjectJob::toString() {

	std::ostringstream oss;
	oss << "InitialiseObjectJob."
		<< "\nGameObject* -> " << object
		<< "\nInitInfo -> " << "Model file: " << info.modelFilename << "\n"
		<< "			Mesh*: " << info.modelPointer << "\n"
		<< "			Texture file: " << info.textureFilename << "\n"
		<< "			Material*: " << info.materialPointer;
	return oss.str();
}

std::string InitialiseObjectJob::humanReadableName() {
	return "InitialiseObjectJob";
}

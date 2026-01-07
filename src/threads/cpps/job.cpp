#include "../headers/job.h"

Job::Job() {
}

Job::~Job() {
}

void Job::execute(CommandPool& command, Buffer& stagingBuffer) {
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

void PrepareForJoinJob::execute(CommandPool& command, Buffer& stagingBuffer) {
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

void InitialiseObjectJob::execute(CommandPool& command, Buffer& stagingBuffer) {
	object->initialise(info, command, stagingBuffer);
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

DeleteObjectJob::DeleteObjectJob(GameObject* object) {
	this->object = object;
}

DeleteObjectJob::~DeleteObjectJob() {}

void DeleteObjectJob::execute(CommandPool& command, Buffer& stagingBuffer) {
	delete object;
}

std::string DeleteObjectJob::toString() {

	std::ostringstream oss;
	oss << "DeleteObjectJob.";
	return oss.str();
}

std::string DeleteObjectJob::humanReadableName() {
	return "DeleteObjectJob";
}

UpdateGarbageJob::UpdateGarbageJob() {
}

UpdateGarbageJob::~UpdateGarbageJob() {}

void UpdateGarbageJob::execute(CommandPool& command, Buffer& stagingBuffer) {
	Engine::garbage.update();
}

std::string UpdateGarbageJob::toString() {

	return "UpdateGarbageJob. No attributes.";
}

std::string UpdateGarbageJob::humanReadableName() {
	return "UpdateGarbageJob";
}

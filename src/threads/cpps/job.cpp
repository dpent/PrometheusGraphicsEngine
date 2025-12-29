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
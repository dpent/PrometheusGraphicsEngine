#pragma once
#include "../../core/headers/Prometheus.h"
#include "../../gameObjects/headers/gameObject.h"

struct Job {
public:
	Job();
	~Job();

	virtual void execute();
	virtual std::string toString();
	virtual std::string humanReadableName();
};

struct PrepareForJoinJob : Job {
public:
	PrepareForJoinJob();
	~PrepareForJoinJob();

	void execute() override;
	std::string toString() override;
	std::string humanReadableName() override;
};

struct InitialiseObjectJob : Job {

	GameObject* object;
	InitInfo info;

public:
	InitialiseObjectJob(GameObject* object, InitInfo info);
	~InitialiseObjectJob();

	void execute() override;
	std::string toString() override;
	std::string humanReadableName() override;
};
#pragma once
#include "../../core/headers/Prometheus.h"
#include "../../gameObjects/headers/gameObject.h"
#include "../../core/headers/engine.h"

struct Job {
public:
	Job();
	~Job();

	virtual void execute(CommandPool& command);
	virtual std::string toString();
	virtual std::string humanReadableName();
};

struct PrepareForJoinJob : Job {
public:
	PrepareForJoinJob();
	~PrepareForJoinJob();

	void execute(CommandPool& command) override;
	std::string toString() override;
	std::string humanReadableName() override;
};

struct InitialiseObjectJob : Job {

	GameObject* object;
	InitInfo info;

public:
	InitialiseObjectJob(GameObject* object, InitInfo info);
	~InitialiseObjectJob();

	void execute(CommandPool& command) override;
	std::string toString() override;
	std::string humanReadableName() override;
};

struct DeleteObjectJob : Job {
	GameObject* object;

public:
	DeleteObjectJob(GameObject* object);
	~DeleteObjectJob();

	void execute(CommandPool& command) override;
	std::string toString() override;
	std::string humanReadableName() override;
};
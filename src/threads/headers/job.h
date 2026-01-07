#pragma once
#include "../../core/headers/Prometheus.h"
#include "../../gameObjects/headers/gameObject.h"
#include "../../core/headers/engine.h"

struct Job {
public:
	Job();
	~Job();

	virtual void execute(CommandPool& command, Buffer& stagingBuffer);
	virtual std::string toString();
	virtual std::string humanReadableName();
};

struct PrepareForJoinJob : Job {
public:
	PrepareForJoinJob();
	~PrepareForJoinJob();

	void execute(CommandPool& command, Buffer& stagingBuffer) override;
	std::string toString() override;
	std::string humanReadableName() override;
};

struct InitialiseObjectJob : Job {

	GameObject* object;
	InitInfo info;

public:
	InitialiseObjectJob(GameObject* object, InitInfo info);
	~InitialiseObjectJob();

	void execute(CommandPool& command, Buffer& stagingBuffer) override;
	std::string toString() override;
	std::string humanReadableName() override;
};

struct DeleteObjectJob : Job {
	GameObject* object;

public:
	DeleteObjectJob(GameObject* object);
	~DeleteObjectJob();

	void execute(CommandPool& command, Buffer& stagingBuffer) override;
	std::string toString() override;
	std::string humanReadableName() override;
};

struct UpdateGarbageJob : Job {

public:
	UpdateGarbageJob();
	~UpdateGarbageJob();

	void execute(CommandPool& command, Buffer& stagingBuffer) override;
	std::string toString() override;
	std::string humanReadableName() override;
};
#pragma once
#include "../../core/headers/Prometheus.h"

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
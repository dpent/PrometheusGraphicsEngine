#include "../headers/workerThread.h"
#include "../../core/headers/engine.h"
#include "../headers/job.h"

WorkerThread::WorkerThread() {

	this->thread = std::thread(&WorkerThread::workerLoop, this);
	this->id = thread.get_id();

	this->command.initialize();
	BufferManager::createStagingBuffer(8192, this->stagingBuffer);

	this->thread.detach();
}

void WorkerThread::detach() {
    this->thread.detach();
}

void WorkerThread::workerLoop() {

	while (alive) {
		Engine::jobInQueueSem.acquire();

		Engine::jobQueueMutex.lock();
		
		Job* j = std::move(Engine::jobQueue.front());

		Engine::jobQueue.pop();

		Engine::jobQueueMutex.unlock();

		doWork(j);
	}
}

void WorkerThread::doWork(Job* job) {
	job->execute(command, stagingBuffer);
}
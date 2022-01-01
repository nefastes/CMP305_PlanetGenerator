#ifndef FARM_H
#define FARM_H
#include <queue>
#include <mutex>
#include <thread>
#include <assert.h>
#include "task.h"

/** A collection of tasks that should be performed in parallel. */
class Farm {
public:

	Farm();
	~Farm();

	/** Add a task to the farm.
		The task will be deleted once it has been run. */
	void add_task(Task* task);

	/** Run all the tasks in the farm.
		This method only returns once all the tasks in the farm
		have been completed. */
	void run();
	const bool& isRunning() { return running_; }
	void clean();

	void toggleNumberThreads();
	const unsigned& getNumberThreads() { return nCPUs_; }
	const float getProgressPercentage();

private:
	bool running_;
	std::queue<Task*> task_queue_;
	std::mutex queue_mutex_;
	unsigned nCPUs_;
	std::vector<std::thread*> threads_;
	float percentage_per_task_;
};

#endif
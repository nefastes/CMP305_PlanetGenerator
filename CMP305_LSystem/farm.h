#ifndef FARM_H
#define FARM_H
#include <queue>
#include <mutex>
#include <thread>
#include "task.h"

/** A collection of tasks that should be performed in parallel. */
class Farm {
public:

	/** Add a task to the farm.
		The task will be deleted once it has been run. */
	void add_task(Task* task);

	/** Run all the tasks in the farm.
		This method only returns once all the tasks in the farm
		have been completed. */
	void run();

	void toggleNumberThreads();
	const unsigned& getNumberThreads() { return nCPUs_; }
	const float& getProgressPercentage() { return progress_percentage_; }

private:
	std::queue<Task*> task_queue_;
	std::mutex queue_mutex_;
	unsigned nCPUs_ = std::thread::hardware_concurrency();	//Change this to anything you want
	std::vector<std::thread*> threads_;
	float progress_percentage_;
};

#endif
#include "farm.h"

// FIXME - You may need to add #includes here (e.g. <thread>)

Farm::Farm() : running_(false), nCPUs_(std::thread::hardware_concurrency()), percentage_per_task_(0.f)
{
}

Farm::~Farm()
{
}

void Farm::add_task(Task* task)
{
	task_queue_.push(task);
}

void Farm::run()
{
	//First make sure the farm can't be run while already running
	assert(running_ == false);
	//Run the farm
	running_ = true;
	percentage_per_task_ = 1.f / static_cast<float>(task_queue_.size() + 1);
	for (unsigned i = 0; i < nCPUs_; ++i) threads_.push_back(new std::thread(
		[&] {
			while (1)
			{
				//A lock is necessary only for this !!
				queue_mutex_.lock();
				if (task_queue_.empty())
				{
					//Return if no more task is available (the thread may have entered while a task was available, and then tried to access the queue after another thread cleared it)
					running_ = false;
					queue_mutex_.unlock();
					return;
				}
				Task* task = task_queue_.front();
				task_queue_.pop();
				queue_mutex_.unlock();

				//Running the task on its own does not require locking !
				task->run();
				delete task, task = nullptr;
			}
		}
	));
}

void Farm::clean()
{
	if (threads_.empty()) return;
	for (unsigned i = 0; i < nCPUs_; ++i) threads_[i]->join(), delete threads_[i];
	threads_.clear();
}

void Farm::toggleNumberThreads()
{
	//Function to toggle the number of threads between 1 and hardware_concurrency() (mine is 4)
	nCPUs_ = (nCPUs_ == 1u) * std::thread::hardware_concurrency() + (nCPUs_ != 1u);
}

const float Farm::getProgressPercentage()
{
	return 1.f - percentage_per_task_ * static_cast<float>(task_queue_.size() + 1);
}

#include "farm.h"

// FIXME - You may need to add #includes here (e.g. <thread>)

void Farm::add_task(Task* task)
{
	task_queue_.push(task);
}

void Farm::run()
{
	float progress_percentage_step = 100.f / static_cast<float>(task_queue_.size());
	for (unsigned i = 0; i < nCPUs_; ++i) threads_.push_back(new std::thread(
		[&] {
			while (1)
			{
				//A lock is necessary only for this !!
				queue_mutex_.lock();
				if (task_queue_.empty())
				{
					//Return if no more task is available (the thread may have entered while a task was available, and then tried to access the queue after another thread cleared it)
					queue_mutex_.unlock();
					return;
				}
				Task* task = task_queue_.front();
				task_queue_.pop();
				queue_mutex_.unlock();

				//Running the task on its own does not require locking !
				task->run();
				delete task, task = nullptr;
				progress_percentage_ += progress_percentage_step;
			}
		}
	));
	for (unsigned i = 0; i < nCPUs_; ++i) threads_[i]->join(), delete threads_[i];
	threads_.clear();
}

void Farm::toggleNumberThreads()
{
	//Function to toggle the number of threads between 1 and hardware_concurrency() (mine is 4)
	nCPUs_ = (nCPUs_ == 1u) * std::thread::hardware_concurrency() + (nCPUs_ != 1u);
}
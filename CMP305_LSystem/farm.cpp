#include "farm.h"

// FIXME - You may need to add #includes here (e.g. <thread>)

void Farm::add_task(Task* task)
{
	task_queue_.push(task);
}

void Farm::run()
{
	for (unsigned i = 0; i < nCPUs; ++i) threads.push_back(new std::thread(
		[&] {
			while (1)
			{
				//Following commented block is BAD !! It's only making you use 1 thread !!
				//std::unique_lock<std::mutex> lock(queue_mutex_);
				//if (!task_queue_.empty())
				//{
				//	task_queue_.front()->run();
				//	//delete task_queue_.front();	//pop actually deletes, calls the destructor
				//	task_queue_.pop();
				//}
				//else return;

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
			}
		}
	));
	for (unsigned i = 0; i < nCPUs; ++i) threads[i]->join(), delete threads[i];
	threads.clear();
}

void Farm::toggleNumberThreads()
{
	//Function to toggle the number of threads between 1 and hardware_concurrency() (mine is 4)
	nCPUs = (nCPUs == 1u) * std::thread::hardware_concurrency() + (nCPUs != 1u);
}
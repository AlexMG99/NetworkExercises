#include "ModuleTaskManager.h"
//#include <Windows.h>

// Alex Morales Garcia and Alejandro París Gómez


void ModuleTaskManager::threadMain()
{
	while (true)
	{
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks
		
		//Check if it works
		//Sleep(1000);
		{
			std::unique_lock<std::mutex> lock(mtx);
			while (scheduledTasks.empty())
			{
				event.wait(lock);

				if (exitFlag)
					return;
			}

			scheduledTasks.front()->execute();

			finishedTasks.push(scheduledTasks.front());
			scheduledTasks.pop(); 
		}
	}
}

bool ModuleTaskManager::init()
{
	// TODO 1: Create threads (they have to execute threadMain())
	for (int i = 0; i < MAX_THREADS; ++i)
	{
		threads[i] = std::thread(&ModuleTaskManager::threadMain, this);
	}

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)
	while (!finishedTasks.empty())
	{
		std::unique_lock<std::mutex> lock(mtx);
		finishedTasks.front()->owner->onTaskFinished(finishedTasks.front());
		finishedTasks.pop();
	}
	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them
	exitFlag = true;
	event.notify_all();

	for (int i = 0; i < MAX_THREADS; ++i)
	{
		threads[i].join();
	}

	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	task->owner = owner;

	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	{
		std::unique_lock<std::mutex> lock(mtx);
		scheduledTasks.push(task);

		event.notify_one();
	}
}

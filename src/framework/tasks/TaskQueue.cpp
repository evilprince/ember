/*
 Copyright (C) 2009 Erik Hjortsberg

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "TaskQueue.h"
#include "ITask.h"
#include "ITaskExecutionListener.h"
#include "TaskExecutor.h"
#include "framework/LoggingInstance.h"

namespace Ember
{

namespace Tasks
{

TaskQueue::TaskQueue(unsigned int numberOfExecutors)
{
	S_LOG_VERBOSE("Creating task queue with " << numberOfExecutors << " executors.");
	for (unsigned int i = 0; i < numberOfExecutors; ++i) {
		TaskExecutor* executor = new TaskExecutor(*this);
		mExecutors.push_back(executor);
	}
}

TaskQueue::~TaskQueue()
{
	for (TaskExecutorStore::iterator I = mExecutors.begin(); I != mExecutors.end(); ++I) {
		delete *I;
	}
	{
		boost::mutex::scoped_lock l(mUnprocessedQueueMutex);
		while (mUnprocessedTaskUnits.size()) {
			TaskUnit taskUnit = mUnprocessedTaskUnits.front();
			mUnprocessedTaskUnits.pop();
			delete taskUnit.first;
		}
	}
	{
		boost::mutex::scoped_lock l(mProcessedQueueMutex);
		while (mProcessedTaskUnits.size()) {
			TaskUnit taskUnit = mProcessedTaskUnits.front();
			mProcessedTaskUnits.pop();
			delete taskUnit.first;
		}
	}
}

void TaskQueue::enqueueTask(ITask* task, ITaskExecutionListener* listener)
{
	{
		boost::mutex::scoped_lock l(mUnprocessedQueueMutex);

		mUnprocessedTaskUnits.push(TaskUnit(task, listener));
	}
	mUnprocessedQueueCond.notify_one();

}

TaskQueue::TaskUnit TaskQueue::fetchNextTask()
{

	boost::unique_lock<boost::mutex> lock(mUnprocessedQueueMutex);

	while (!mUnprocessedTaskUnits.size()) {
		mUnprocessedQueueCond.wait(lock);
	}
	TaskUnit taskUnit = mUnprocessedTaskUnits.front();
	mUnprocessedTaskUnits.pop();
	return taskUnit;
}

void TaskQueue::addProcessedTask(TaskQueue::TaskUnit taskUnit)
{
	boost::mutex::scoped_lock l(mProcessedQueueMutex);
	mProcessedTaskUnits.push(taskUnit);
}

void TaskQueue::pollProcessedTasks()
{
	TaskUnitQueue processedCopy;
	{
		boost::mutex::scoped_lock l(mProcessedQueueMutex);
		processedCopy = mProcessedTaskUnits;
	}
	while (processedCopy.size()) {

		TaskUnit taskUnit = processedCopy.front();
		processedCopy.pop();
		try {
			taskUnit.first->executeTaskInMainThread();
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Error when executing task in main thread." << ex);
		} catch (...) {
			S_LOG_FAILURE("Unknown error when executing task in main thread.");
		}
		try {
			delete taskUnit.first;
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Error when deleting task in main thread." << ex);
		} catch (...) {
			S_LOG_FAILURE("Unknown error when deleting task in main thread.");
		}

	}
}

}

}
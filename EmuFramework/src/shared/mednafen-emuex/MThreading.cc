/*  This file is part of EmuFramework.

	EmuFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	EmuFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <mednafen/types.h>
#include <mednafen/MThreading.h>
#include <imagine/util/utility.h>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Mednafen::MThreading
{

struct Thread : public std::thread
{
	using thread::thread;
};
struct Mutex : public std::mutex {};
struct Cond : public std::condition_variable {};

Thread* Thread_Create(int (*fn)(void *), void *data, const char* debug_name)
{
	return new Thread{fn, data};
}

void Thread_Wait(Thread* thread, int* status)
{
	thread->join();
	delete thread;
}

uint64 Thread_SetAffinity(Thread* thread, const uint64 mask)
{
	return 0;
}

Mutex* Mutex_Create(void)
{
	return new Mutex{};
}

void Mutex_Destroy(Mutex* mutex) noexcept
{
	delete mutex;
}

bool Mutex_Lock(Mutex* mutex) noexcept
{
	//logMsg("lock %p", mutex);
	assumeExpr(mutex);
	mutex->lock();
	return true;
}

bool Mutex_Unlock(Mutex* mutex) noexcept
{
	//logMsg("unlock %p", mutex);
	assumeExpr(mutex);
	mutex->unlock();
	return true;
}

Cond* Cond_Create(void)
{
	return new Cond{};
}

void Cond_Destroy(Cond* cond) noexcept
{
	delete cond;
}

bool Cond_Signal(Cond* cond) noexcept
{
	assumeExpr(cond);
	cond->notify_one();
	return true;
}

bool Cond_Wait(Cond* cond, Mutex* mutex) noexcept
{
	//logMsg("waiting %p on mutex %p", cond, mutex);
	assumeExpr(cond);
	assumeExpr(mutex);
	std::unique_lock<std::mutex> lock{*mutex, std::adopt_lock};
	cond->wait(lock);
	lock.release();
	return true;
}

}

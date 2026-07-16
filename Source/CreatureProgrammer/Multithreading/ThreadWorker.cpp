#include "ThreadWorker.h"

int32 FThreadWorker::ThreadNameIndex = 0;

FThreadWorker::FThreadWorker()
{
	Thread = FRunnableThread::Create(this, *FString::Printf(L"Creature Thread Worker %d", ThreadNameIndex), 0, TPri_Highest);
	ThreadNameIndex++;
}

FThreadWorker::~FThreadWorker()
{
	if (Thread != nullptr)
	{
		Thread->Kill();
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
}

bool FThreadWorker::Init()
{
	DoInit();
	bRun = true;
	return true;
}

uint32 FThreadWorker::Run()
{
	while (bRun)
	{
		WorkAvailiabilty = DoWork();
		while (!ShouldWake())
		{
			const float Time = (WorkAvailiabilty == EThreadWorkAvailiabilty::WorkUnavailable ? SleepTime : 0.0f);
			// 0 passed in here causes a thread context switch, which makes more sense if work is available soon.
			// otherwise, sleep for some time >= 1ms.
			FPlatformProcess::SleepNoStats(Time);
		}
	}
	return 0;
}

void FThreadWorker::Stop()
{
	bRun = false;
}

void FThreadWorker::Exit()
{
	DoCleanup();
}

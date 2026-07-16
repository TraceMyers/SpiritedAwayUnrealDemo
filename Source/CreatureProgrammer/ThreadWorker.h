#pragma once

enum class EThreadWorkAvailiabilty
{
	WorkAvailable,
	WorkAvailableSoon,
	WorkUnavailable
};

// I always end up making thread workers in unreal engine projects the same way, and this is it. 
// subclass and implement DoWork() at minimum. It will be called in a loop automatically.
class FThreadWorker : public FRunnable
{
public:
	
	FThreadWorker();
	~FThreadWorker();
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	
	// unreal + static variables don't work incredibly well together. reset at game start.
	static void ResetThreadNameIndex() { ThreadNameIndex = 0; }
	
protected:
	
	// define in subclass for thread init.
	virtual void DoInit() {}
	
	virtual void DoCleanup() {}
	
	// define in subclass for doing one work unit.
	// return WorkAvailable if another job is available right away.
	// return WorkAvailableSoon if a job should be available in < 1ms
	// return WorkUnavailable if a job won't be available until after 1ms
	// this functions as a hint for sleep behavior.
	virtual EThreadWorkAvailiabilty DoWork() = 0;
	
protected:
	
	bool ShouldWake() const { return !bRun || WorkAvailiabilty == EThreadWorkAvailiabilty::WorkAvailable; }
	
	static constexpr float ONE_MILLISECOND = 1.0f / 1000.0f;
	static int32 ThreadNameIndex;
	
	FRunnableThread* Thread = nullptr;
	
	FThreadSafeBool bRun = true;
	EThreadWorkAvailiabilty WorkAvailiabilty = EThreadWorkAvailiabilty::WorkUnavailable;
	
	// if this is < 1ms, it does not sleep. On Windows anyway, it just calls SwitchToThread().
	float SleepTime = ONE_MILLISECOND * 1.0001f;

};
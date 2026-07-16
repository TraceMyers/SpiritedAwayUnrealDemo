#pragma once
#include "ThreadWorker.h"

class FCreatureThread : public FThreadWorker
{
	virtual EThreadWorkAvailiabilty DoWork() override;
};


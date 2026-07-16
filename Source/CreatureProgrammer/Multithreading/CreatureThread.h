#pragma once

#include "ThreadWorker.h"

class FCreatureThread : public FThreadWorker
{
protected:
	virtual EThreadWorkAvailiabilty DoWork() override;
};
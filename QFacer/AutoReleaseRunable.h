#pragma once

#include "Runable.h"

class AutoReleaseRunable :public Runable
{
public:
	AutoReleaseRunable();
	virtual ~AutoReleaseRunable();
};


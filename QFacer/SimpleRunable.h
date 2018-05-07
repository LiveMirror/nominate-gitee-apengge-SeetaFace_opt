#include "AutoReleaseRunable.h"

class SimpleRunable : public AutoReleaseRunable
{
public:

	virtual int start();

	SimpleRunable(RunableFun fun, void * pUser);

	virtual ~SimpleRunable();
	
	RunableFun fun;
	void * pUser;
};


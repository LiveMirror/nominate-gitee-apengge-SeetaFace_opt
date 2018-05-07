#include "SimpleRunable.h"


SimpleRunable::SimpleRunable(RunableFun fun, void * pUser)
{
	this->fun = fun;
	this->pUser = pUser;
}


SimpleRunable::~SimpleRunable()
{

}


int SimpleRunable::start()
{
	return fun(pUser);
}

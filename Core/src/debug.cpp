#include "debug.h"
#include <stdexcept>

ng::Debug::Debug()
{

}

ng::Debug::~Debug()
{

}

void ng::Debug::graphicsErrorCallback(int error, const char * description)
{
	throw std::runtime_error(description);
}

#include "debug.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>

ng::debug::Debug::Debug()
{
}

ng::debug::Debug::~Debug()
{
}

void ng::debug::exitFatal(std::string message, int32 exitCode)
{
	LOGI("Fatal error : %s", message.c_str());
	exit(exitCode);
}


#pragma once
#include "def.h"

#ifdef NDEBUG
#define LOGD(s)
#else
#define LOGD(s)															\
{																		\
	std::cout << "LOGD: " << s << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
}
#endif

#define USE_LOGI

#ifdef USE_LOGI
#define LOGI(s)															\
{																		\
	std::cout << "LOGI: " << s << std::endl;							\
}
#else
#define LOGI(s)
#endif

namespace ng {

	namespace debug {

		class Debug
		{
		public:
			Debug();
			~Debug();

		};

		void exitFatal(std::string message, int32 exitCode);

	}

}
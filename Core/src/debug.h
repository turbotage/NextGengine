#pragma once

namespace ng {
	class Debug
	{
	public:
		Debug();
		~Debug();

		int lastErrorOpenGL;

		static void graphicsErrorCallback(int error, const char* description);

	};
}


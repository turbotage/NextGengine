#pragma once

#include "../def.h"
#include <WSIWindow\WSIWindow.h>

namespace ng {
	namespace graphics {
		class Window : public WSIWindow
		{
		public:
			void OnMouseEvent(eAction action, int16 x, int16 y, uint8 btn);

			void OnKeyEvent(eAction action, uint8 keycode);

			void OnTextEvent(const char* str);

			void OnResizeEvent(uint16 width, uint16 height);
		};
	}
}



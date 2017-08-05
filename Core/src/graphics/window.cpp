#include "window.h"
#include <stdio.h>

void ng::graphics::Window::OnMouseEvent(eAction action, int16 x, int16 y, uint8 btn)
{
	const char* type[] = { "up  ", "down", "move" };
	printf("Mouse: %s %d x %d Btn:%d\n", type[action], x, y, btn);
}

void ng::graphics::Window::OnKeyEvent(eAction action, uint8 keycode)
{
	const char* type[] = { "up  ", "down" };
	printf("Key: %s keycode:%d\n", type[action], keycode);
}

void ng::graphics::Window::OnTextEvent(const char * str)
{
	printf("Text: %s\n", str);
}

void ng::graphics::Window::OnResizeEvent(uint16 width, uint16 height)
{
	printf("Window Resize: width=%4d height=%4d\n", width, height);
}

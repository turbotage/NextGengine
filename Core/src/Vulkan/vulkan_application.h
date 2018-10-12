#pragma once

#include "window.h"
#include "vulkan_base.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_framebuffer.h"
#include "vulkan_debug.h"
#include "../camera.h"

namespace ng {
	namespace vulkan {

		class VulkanApplication {
		private:

		public:

			Window window;

			VulkanBase base;

			VulkanDevice graphicsDevice;
			VulkanDevice computeDevice;

			VulkanSwapchain swapchain;

			VulkanFramebuffer framebuffer;

			struct Settings {
				/** @brief Activates validation layers (and message output) when set to true */
				bool validation = false;
				/** @brief Set to true if fullscreen mode has been requested via command line */
				bool fullscreen = false;
				/** @brief Set to true if v-sync will be forced for the swapchain */
				bool vsync = false;
				/** @brief Enable UI overlay */
				bool overlay = false;
				/** @brief The name the window sahould have */
				const char* appWindowName = "sick application";
				/** @brief Color screen should clear to */
				VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };
			} appSettings;

			struct Time {
				// Defines a frame rate independent timer value clamped from -1.0...1.0
				// For use in animations, rotations, etc.
				float timer = 0.0f;
				// Multiplier for speeding up (or slowing down) the global timer
				float timerSpeed = 0.25f;

				bool paused = false;
			} time;

			Camera camera;

			struct {
				VkImage image;
				VkDeviceMemory memory;
				VkImageView view;
			} depthStencil;

			struct {
				bool left = false;
				bool right = false;
				bool middle = false;
			} mouseButtons;




		public:

			VulkanApplication();

			void createPhysicalDevices();

			void init();

			void run();

			void cleanup();

			// Pure virtual render function (override in derived class)
			virtual void render() = 0;
			// Called when view change occurs
			// Can be overriden in derived class to e.g. update uniform buffers 
			// Containing view dependant matrices
			virtual void viewChanged();
			// Called after a key was pressed, used to do custom key handling
			virtual void keyPressed(uint32 key);
			// Called after the mouse cursor moved and before internal events (camera rotation and such)
			virtual void mouseMoved(double x, double y, bool &handled);
			// Called when the window has been resized, overide to recreate resources attached to frambuffer / swapchain
			virtual void windowResized();

			virtual void buildCommandBuffers() = 0;

		};

	}
}



#include <pch.h>

namespace ng {
	struct ModelPrimitive {
		uint32 firstIndex;
		uint32 indexCount;
		int32 materialIndex;
	};

	struct ModelMaterial {
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		std::string baseColorTexturePath;
		uint32 normalTexturePath;

		uint8 alphaMode = AlphaModeFlagBits::eOpaque;
		float alphaCutOff;
		bool doubleSided = false;

		vk::DescriptorSet descriptorSet;
		vk::Pipeline pipeline;
	};
}
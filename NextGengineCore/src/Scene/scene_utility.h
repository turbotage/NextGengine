
#include <pch.h>

namespace ng {

	enum class AlphaModeFlagBits {
		eOpaque,
		eMask,
		eBlend
	};

	



	class AABB3D {
	public:

		AABB3D(glm::vec3 min, glm::vec3 max);

		glm::vec3 getMin();
		glm::vec3 getMax();

		void setMin(glm::vec3 min);
		void setMax(glm::vec3 max);

	private:
		glm::vec3 m_Min;
		glm::vec3 m_Max;
	};




	class AABB2D {
	public:

		AABB2D(glm::vec2 min, glm::vec2 max);

		glm::vec2 getMin();
		glm::vec2 getMax();

		void setMin(glm::vec2 min);
		void setMax(glm::vec2 max);

	private:
		glm::vec2 m_Min;
		glm::vec2 m_Max;

	};

}
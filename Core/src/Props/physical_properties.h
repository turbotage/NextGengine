#pragma once

#include "vec3f.h"
#include "../def.h"
#include <vector>

namespace ng {
	namespace props {
		enum ePhysicalProperties {
			GRAVITY_ENABLED_BIT = 1,
			MOVEMENT_ENABLED_BIT,
			DESTRUCTION_ENABLED_BIT,
			CONSTANT_FORCE_ENABLED_BIT
		};

		struct MechanicalProperties {
			//FEM properties
			float elasticity;
			float density;
			float brittleness;
		};

		struct OpticalProperties {
			float absorbance;
			float reflectivity;
			float refractiveIndex;
			float color;
		};

		struct ThermalProperties {
			float boilingPoint;
			float meltingPoint;
			float conductivity;
			float expansion;
		};

		class PhysicalProperties
		{
			uint8 enabledPhysicalProps;
			MechanicalProperties* mechanicalProps;
			OpticalProperties* opticalProps;
			ThermalProperties* thermalProps;
		};
	}
}



#include "LGRGardenBuilding.h"

ALGRGardenBuilding::ALGRGardenBuilding()
{
	BuildingType = ELGRBuildingType::Garden;
	FootprintSize = FIntPoint(1, 1);
	PopulationCost = 0;
	MaxHealth = 75.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "GardenName", "花园");
	BuildingDescription = NSLOCTEXT(
		"LGRBuilding",
		"GardenDescription",
		"降低周围3×3格内的噪音；处于铁匠铺范围内时失效。");
}

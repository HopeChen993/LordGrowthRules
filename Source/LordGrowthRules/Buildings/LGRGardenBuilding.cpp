#include "LGRGardenBuilding.h"

ALGRGardenBuilding::ALGRGardenBuilding()
{
	BuildingType = ELGRBuildingType::Garden;
	FootprintSize = FIntPoint(1, 1);
	MaxHealth = 100.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "GardenName", "花园");
}

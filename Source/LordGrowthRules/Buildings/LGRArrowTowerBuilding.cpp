#include "LGRArrowTowerBuilding.h"

ALGRArrowTowerBuilding::ALGRArrowTowerBuilding()
{
	BuildingType = ELGRBuildingType::ArrowTower;
	FootprintSize = FIntPoint(1, 1);
	MaxHealth = 100.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "ArrowTowerName", "箭塔");
}

#include "LGRLordManorBuilding.h"

ALGRLordManorBuilding::ALGRLordManorBuilding()
{
	BuildingType = ELGRBuildingType::LordManor;
	FootprintSize = FIntPoint(2, 2);
	PopulationCost = 0;
	MaxHealth = 300.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "LordManorName", "领主屋");
}

#include "LGRLordManorBuilding.h"

ALGRLordManorBuilding::ALGRLordManorBuilding()
{
	BuildingType = ELGRBuildingType::LordManor;
	FootprintSize = FIntPoint(2, 2);
	PopulationCost = 20;
	MaxHealth = 500.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "LordManorName", "领主屋");
}

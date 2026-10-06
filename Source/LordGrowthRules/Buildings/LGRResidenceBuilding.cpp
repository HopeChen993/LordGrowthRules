#include "LGRResidenceBuilding.h"

ALGRResidenceBuilding::ALGRResidenceBuilding()
{
	BuildingType = ELGRBuildingType::Residence;
	FootprintSize = FIntPoint(1, 1);
	PopulationCost = 10;
	MaxHealth = 50.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "ResidenceName", "居民楼");
	BuildingDescription = NSLOCTEXT(
		"LGRBuilding",
		"ResidenceDescription",
		"每日产生人口；噪音会影响人口增长。");
}

void ALGRResidenceBuilding::SetLastSettlementResult(
	const int32 EffectiveNoise,
	const int32 PopulationGain)
{
	LastEffectiveNoise = FMath::Max(0, EffectiveNoise);
	LastPopulationGain = FMath::Max(0, PopulationGain);
}

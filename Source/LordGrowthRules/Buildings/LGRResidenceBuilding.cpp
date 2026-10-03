#include "LGRResidenceBuilding.h"

ALGRResidenceBuilding::ALGRResidenceBuilding()
{
	BuildingType = ELGRBuildingType::Residence;
	FootprintSize = FIntPoint(1, 1);
	MaxHealth = 100.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "ResidenceName", "居民楼");
}

void ALGRResidenceBuilding::SetLastSettlementResult(
	const int32 EffectiveNoise,
	const int32 PopulationGain)
{
	LastEffectiveNoise = FMath::Max(0, EffectiveNoise);
	LastPopulationGain = FMath::Max(0, PopulationGain);
}

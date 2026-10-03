#pragma once

#include "CoreMinimal.h"
#include "LGRBuildingBase.h"
#include "LGRResidenceBuilding.generated.h"

UCLASS(BlueprintType, Blueprintable)
class LORDGROWTHRULES_API ALGRResidenceBuilding : public ALGRBuildingBase
{
	GENERATED_BODY()

public:
	ALGRResidenceBuilding();

	UFUNCTION(BlueprintPure, Category = "LGR|Residence")
	int32 GetBasePopulationGain() const { return BasePopulationGain; }

	UFUNCTION(BlueprintPure, Category = "LGR|Residence")
	int32 GetNoiseTolerance() const { return NoiseTolerance; }

	UFUNCTION(BlueprintPure, Category = "LGR|Residence|Settlement")
	int32 GetLastEffectiveNoise() const { return LastEffectiveNoise; }

	UFUNCTION(BlueprintPure, Category = "LGR|Residence|Settlement")
	int32 GetLastPopulationGain() const { return LastPopulationGain; }

	void SetLastSettlementResult(int32 EffectiveNoise, int32 PopulationGain);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Residence", meta = (ClampMin = "0"))
	int32 BasePopulationGain = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Residence", meta = (ClampMin = "0"))
	int32 NoiseTolerance = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Residence|Settlement")
	int32 LastEffectiveNoise = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Residence|Settlement")
	int32 LastPopulationGain = 0;
};

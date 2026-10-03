#pragma once

#include "CoreMinimal.h"
#include "LGRBuildingBase.h"
#include "LGRGardenBuilding.generated.h"

UCLASS(BlueprintType, Blueprintable)
class LORDGROWTHRULES_API ALGRGardenBuilding : public ALGRBuildingBase
{
	GENERATED_BODY()

public:
	ALGRGardenBuilding();

	UFUNCTION(BlueprintPure, Category = "LGR|Garden")
	int32 GetEffectRadius() const { return EffectRadius; }

	UFUNCTION(BlueprintPure, Category = "LGR|Garden")
	int32 GetNoiseReduction() const { return NoiseReduction; }

	UFUNCTION(BlueprintPure, Category = "LGR|Garden")
	int32 GetResidencePopulationBonus() const { return ResidencePopulationBonus; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Garden", meta = (ClampMin = "0"))
	int32 EffectRadius = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Garden", meta = (ClampMin = "0"))
	int32 NoiseReduction = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Garden", meta = (ClampMin = "0"))
	int32 ResidencePopulationBonus = 1;
};

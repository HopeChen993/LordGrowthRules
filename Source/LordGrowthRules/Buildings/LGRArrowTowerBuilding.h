#pragma once

#include "CoreMinimal.h"
#include "LGRBuildingBase.h"
#include "LGRArrowTowerBuilding.generated.h"

UCLASS(BlueprintType, Blueprintable)
class LORDGROWTHRULES_API ALGRArrowTowerBuilding : public ALGRBuildingBase
{
	GENERATED_BODY()

public:
	ALGRArrowTowerBuilding();

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Noise")
	int32 GetNoiseStrength() const { return NoiseStrength; }

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Noise")
	int32 GetNoiseRadius() const { return NoiseRadius; }

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Noise")
	int32 GetNoiseFalloffPerCell() const { return NoiseFalloffPerCell; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Noise", meta = (ClampMin = "0"))
	int32 NoiseStrength = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Noise", meta = (ClampMin = "0"))
	int32 NoiseRadius = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Noise", meta = (ClampMin = "0"))
	int32 NoiseFalloffPerCell = 1;
};

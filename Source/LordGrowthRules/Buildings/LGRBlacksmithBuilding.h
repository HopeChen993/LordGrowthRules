#pragma once

#include "CoreMinimal.h"
#include "LGRBuildingBase.h"
#include "LGRBlacksmithBuilding.generated.h"

UCLASS(BlueprintType, Blueprintable)
class LORDGROWTHRULES_API ALGRBlacksmithBuilding : public ALGRBuildingBase
{
	GENERATED_BODY()

public:
	ALGRBlacksmithBuilding();

	UFUNCTION(BlueprintPure, Category = "LGR|Blacksmith")
	float GetEffectRadiusCells() const { return EffectRadiusCells; }

	UFUNCTION(BlueprintPure, Category = "LGR|Blacksmith")
	float GetAttackDamageMultiplier() const { return AttackDamageMultiplier; }

	UFUNCTION(BlueprintPure, Category = "LGR|Blacksmith")
	int32 GetNoiseStrength() const { return NoiseStrength; }

	UFUNCTION(BlueprintPure, Category = "LGR|Blacksmith")
	bool IsGridPositionInEffectRange(FIntPoint GridPosition) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Blacksmith", meta = (ClampMin = "0.0"))
	float EffectRadiusCells = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Blacksmith", meta = (ClampMin = "1.0"))
	float AttackDamageMultiplier = 1.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Blacksmith", meta = (ClampMin = "0"))
	int32 NoiseStrength = 7;
};

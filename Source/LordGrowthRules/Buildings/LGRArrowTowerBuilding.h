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

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Combat")
	float GetAttackDamage() const { return AttackDamage; }

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Combat")
	float GetEffectiveAttackDamage() const;

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Combat")
	float GetAttackInterval() const { return AttackInterval; }

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Combat")
	float GetAttackRadiusCells() const { return AttackRadiusCells; }

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Combat")
	bool IsBuffedByBlacksmith() const;

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Noise")
	int32 GetNoiseStrength() const { return NoiseStrength; }

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Noise")
	int32 GetNoiseRadius() const { return NoiseRadius; }

	UFUNCTION(BlueprintPure, Category = "LGR|Arrow Tower|Noise")
	int32 GetNoiseFalloffPerCell() const { return NoiseFalloffPerCell; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Combat", meta = (ClampMin = "0.0"))
	float AttackDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Combat", meta = (ClampMin = "0.01"))
	float AttackInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Combat", meta = (ClampMin = "0.0"))
	float AttackRadiusCells = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Noise", meta = (ClampMin = "0"))
	int32 NoiseStrength = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Noise", meta = (ClampMin = "0"))
	int32 NoiseRadius = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Arrow Tower|Noise", meta = (ClampMin = "0"))
	int32 NoiseFalloffPerCell = 0;
};

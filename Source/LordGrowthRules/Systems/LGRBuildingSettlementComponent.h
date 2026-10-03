#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGRBuildingSettlementComponent.generated.h"

class ALGRResidenceBuilding;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FLGRResidenceSettledSignature,
	ALGRResidenceBuilding*,
	Residence,
	int32,
	EffectiveNoise,
	int32,
	PopulationGain);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLGRDailyBuildingSettlementCompletedSignature,
	int32,
	TotalPopulationGain);

UCLASS(ClassGroup = (LGR), meta = (BlueprintSpawnableComponent))
class LORDGROWTHRULES_API ULGRBuildingSettlementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGRBuildingSettlementComponent();

	UFUNCTION(BlueprintCallable, Category = "LGR|Settlement")
	bool ResolveDailyBuildingRules();

	UPROPERTY(BlueprintAssignable, Category = "LGR|Settlement")
	FLGRResidenceSettledSignature OnResidenceSettled;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Settlement")
	FLGRDailyBuildingSettlementCompletedSignature OnDailyBuildingSettlementCompleted;

private:
	static int32 GetGridDistance(const FIntPoint& A, const FIntPoint& B);
};

#pragma once

#include "CoreMinimal.h"
#include "LGRGridTypes.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct FLGRGridCell
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Grid")
	FIntPoint Coordinates = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGR|Grid")
	bool bBuildable = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Grid")
	TObjectPtr<AActor> OccupyingActor = nullptr;

	bool IsFree() const
	{
		return bBuildable && !IsValid(OccupyingActor.Get());
	}
};

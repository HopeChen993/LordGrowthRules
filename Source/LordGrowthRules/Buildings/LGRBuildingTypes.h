#pragma once

#include "CoreMinimal.h"
#include "LGRBuildingTypes.generated.h"

UENUM(BlueprintType)
enum class ELGRBuildingType : uint8
{
	Residence UMETA(DisplayName = "Residence"),
	ArrowTower UMETA(DisplayName = "Arrow Tower"),
	Garden UMETA(DisplayName = "Garden"),
	LordManor UMETA(DisplayName = "Lord Manor"),
	Blacksmith UMETA(DisplayName = "Blacksmith")
};

UENUM(BlueprintType)
enum class ELGRPlacementFailureReason : uint8
{
	None UMETA(DisplayName = "None"),
	GridUnavailable UMETA(DisplayName = "Grid Unavailable"),
	NoCellSelected UMETA(DisplayName = "No Cell Selected"),
	NoBuildingSelected UMETA(DisplayName = "No Building Selected"),
	InvalidBuildingClass UMETA(DisplayName = "Invalid Building Class"),
	NotBuildingPhase UMETA(DisplayName = "Not Building Phase"),
	CellUnavailable UMETA(DisplayName = "Cell Unavailable"),
	InsufficientPopulation UMETA(DisplayName = "Insufficient Population"),
	SpawnFailed UMETA(DisplayName = "Spawn Failed"),
	NoBuildActionsRemaining UMETA(DisplayName = "No Build Actions Remaining")
};

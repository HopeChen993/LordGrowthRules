#include "LGRPlayerController.h"

#include "../Buildings/LGRBuildingBase.h"
#include "../Core/LGRGameModeBase.h"
#include "../Grid/LGRGridHighlight.h"
#include "../Grid/LGRGridManager.h"
#include "Kismet/GameplayStatics.h"

ALGRPlayerController::ALGRPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ALGRPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ConfigureCursorForGameplay();

	if (IsLocalController())
	{
		SelectedBuildingClass = nullptr;
		RefreshGridManager();
		EnsureGridHighlight();
	}
}

void ALGRPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GridHighlight))
	{
		GridHighlight->Destroy();
		GridHighlight = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ALGRPlayerController::PlayerTick(const float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController())
	{
		UpdateGridHover();
	}
}

bool ALGRPlayerController::GetMouseGroundHit(FHitResult& OutHit) const
{
	OutHit = FHitResult();
	return GetHitResultUnderCursor(GroundTraceChannel, bTraceComplexGround, OutHit);
}

void ALGRPlayerController::ConfigureCursorForGameplay()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

bool ALGRPlayerController::RefreshGridManager()
{
	GridManager = Cast<ALGRGridManager>(UGameplayStatics::GetActorOfClass(this, ALGRGridManager::StaticClass()));
	return IsValid(GridManager);
}

bool ALGRPlayerController::GetHoveredGridCell(FIntPoint& OutGridCoordinates, bool& bOutCanPlace) const
{
	OutGridCoordinates = HoveredGridCoordinates;
	bOutCanPlace = bHoveredCellCanPlace;
	return bHasHoveredGridCell;
}

bool ALGRPlayerController::SelectHoveredGridCell()
{
	if (!bHasHoveredGridCell || (!IsValid(GridManager) && !RefreshGridManager()))
	{
		ClearSelectedGridCell();
		return false;
	}

	if (!GridManager->IsCellFree(HoveredGridCoordinates))
	{
		ClearSelectedGridCell();
		return false;
	}

	SelectedGridCoordinates = HoveredGridCoordinates;
	bHasSelectedGridCell = true;
	SelectedBuildingClass = nullptr;
	UpdatePlacementHighlight();
	OnGridCellSelectionChanged.Broadcast(SelectedGridCoordinates, true);
	return true;
}

void ALGRPlayerController::ClearSelectedGridCell()
{
	if (!bHasSelectedGridCell)
	{
		return;
	}

	const FIntPoint PreviousSelection = SelectedGridCoordinates;
	bHasSelectedGridCell = false;
	SelectedBuildingClass = nullptr;
	OnGridCellSelectionChanged.Broadcast(PreviousSelection, false);
	UpdatePlacementHighlight();
}

bool ALGRPlayerController::GetSelectedGridCell(FIntPoint& OutGridCoordinates) const
{
	OutGridCoordinates = SelectedGridCoordinates;
	return bHasSelectedGridCell;
}

void ALGRPlayerController::SelectBuildingClass(const TSubclassOf<ALGRBuildingBase> BuildingClass)
{
	SelectedBuildingClass = BuildingClass;
	UpdatePlacementHighlight();
}

void ALGRPlayerController::ClearBuildingSelection()
{
	SelectedBuildingClass = nullptr;
	UpdatePlacementHighlight();
}

bool ALGRPlayerController::CanPlaceSelectedBuilding() const
{
	const bool bHasTargetCell = bHasSelectedGridCell || bHasHoveredGridCell;
	if (!bHasTargetCell || !IsValid(GridManager) || !SelectedBuildingClass)
	{
		return false;
	}

	const FIntPoint& TargetCoordinates = bHasSelectedGridCell
		? SelectedGridCoordinates
		: HoveredGridCoordinates;
	return GetPlacementFailureReasonAt(TargetCoordinates) == ELGRPlacementFailureReason::None;
}

bool ALGRPlayerController::CanDeploySelectedBuilding() const
{
	return bHasSelectedGridCell
		&& GetPlacementFailureReasonAt(SelectedGridCoordinates) == ELGRPlacementFailureReason::None;
}

int32 ALGRPlayerController::GetSelectedBuildingPopulationCost() const
{
	if (!SelectedBuildingClass)
	{
		return 0;
	}

	const ALGRBuildingBase* BuildingDefaults = SelectedBuildingClass->GetDefaultObject<ALGRBuildingBase>();
	return IsValid(BuildingDefaults) ? BuildingDefaults->GetPopulationCost() : 0;
}

void ALGRPlayerController::GetSelectedBuildingPlacementStatus(
	bool& bOutCanDeploy,
	ELGRPlacementFailureReason& OutFailureReason,
	FText& OutFailureText) const
{
	OutFailureReason = bHasSelectedGridCell
		? GetPlacementFailureReasonAt(SelectedGridCoordinates)
		: ELGRPlacementFailureReason::NoCellSelected;
	bOutCanDeploy = OutFailureReason == ELGRPlacementFailureReason::None;
	OutFailureText = GetPlacementFailureText(OutFailureReason);
}

bool ALGRPlayerController::TryDeploySelectedBuilding()
{
	return bHasSelectedGridCell && TryPlaceBuildingAt(SelectedGridCoordinates, true);
}

bool ALGRPlayerController::TryPlaceSelectedBuilding()
{
	if (bHasSelectedGridCell)
	{
		return TryPlaceBuildingAt(SelectedGridCoordinates, true);
	}

	return bHasHoveredGridCell && TryPlaceBuildingAt(HoveredGridCoordinates, false);
}

bool ALGRPlayerController::TryPlaceBuildingAt(
	const FIntPoint& GridCoordinates,
	const bool bClearSelectionOnSuccess)
{
	if (GetPlacementFailureReasonAt(GridCoordinates) != ELGRPlacementFailureReason::None
		|| !IsValid(GetWorld()))
	{
		return false;
	}

	ALGRGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ALGRGameModeBase>();
	const ALGRBuildingBase* BuildingDefaults = SelectedBuildingClass->GetDefaultObject<ALGRBuildingBase>();
	if (!IsValid(GameMode) || !IsValid(BuildingDefaults))
	{
		return false;
	}

	const FIntPoint FootprintSize = BuildingDefaults->GetFootprintSize();
	const int32 PopulationCost = BuildingDefaults->GetPopulationCost();
	if (!GameMode->TryAssignPopulation(PopulationCost))
	{
		return false;
	}

	const FVector BuildingSpawnLocation = GridManager->GridAreaToWorld(GridCoordinates, FootprintSize);
	const FRotator SpawnRotation = GridManager->GetActorRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALGRBuildingBase* NewBuilding = GetWorld()->SpawnActor<ALGRBuildingBase>(
		SelectedBuildingClass,
		BuildingSpawnLocation,
		SpawnRotation,
		SpawnParameters);

	if (!IsValid(NewBuilding))
	{
		GameMode->RefundAssignedPopulation(PopulationCost);
		return false;
	}

	if (!GridManager->OccupyArea(GridCoordinates, FootprintSize, NewBuilding))
	{
		NewBuilding->Destroy();
		GameMode->RefundAssignedPopulation(PopulationCost);
		return false;
	}

	NewBuilding->InitializePlacedBuilding(GridManager, GridCoordinates);
	OnBuildingPlaced.Broadcast(NewBuilding, GridCoordinates);
	if (bClearSelectionOnSuccess)
	{
		ClearSelectedGridCell();
	}
	UpdateGridHover();
	return true;
}

ELGRPlacementFailureReason ALGRPlayerController::GetPlacementFailureReasonAt(
	const FIntPoint& GridCoordinates) const
{
	if (!IsValid(GridManager))
	{
		return ELGRPlacementFailureReason::GridUnavailable;
	}

	if (!SelectedBuildingClass)
	{
		return ELGRPlacementFailureReason::NoBuildingSelected;
	}

	const ALGRBuildingBase* BuildingDefaults = SelectedBuildingClass->GetDefaultObject<ALGRBuildingBase>();
	if (!IsValid(BuildingDefaults))
	{
		return ELGRPlacementFailureReason::InvalidBuildingClass;
	}

	UWorld* World = GetWorld();
	const ALGRGameModeBase* GameMode = IsValid(World)
		? World->GetAuthGameMode<ALGRGameModeBase>()
		: nullptr;
	if (!IsValid(GameMode) || GameMode->GetCurrentPhase() != ELGRGamePhase::Building)
	{
		return ELGRPlacementFailureReason::NotBuildingPhase;
	}

	if (!GridManager->CanOccupyArea(GridCoordinates, BuildingDefaults->GetFootprintSize()))
	{
		return ELGRPlacementFailureReason::CellUnavailable;
	}

	if (!GameMode->CanAffordPopulation(BuildingDefaults->GetPopulationCost()))
	{
		return ELGRPlacementFailureReason::InsufficientPopulation;
	}

	return ELGRPlacementFailureReason::None;
}

FText ALGRPlayerController::GetPlacementFailureText(
	const ELGRPlacementFailureReason FailureReason) const
{
	switch (FailureReason)
	{
	case ELGRPlacementFailureReason::None:
		return FText::GetEmpty();
	case ELGRPlacementFailureReason::GridUnavailable:
		return NSLOCTEXT("LGRPlacement", "GridUnavailable", "未找到网格");
	case ELGRPlacementFailureReason::NoCellSelected:
		return NSLOCTEXT("LGRPlacement", "NoCellSelected", "请先选择一个空格");
	case ELGRPlacementFailureReason::NoBuildingSelected:
		return NSLOCTEXT("LGRPlacement", "NoBuildingSelected", "请选择建筑");
	case ELGRPlacementFailureReason::InvalidBuildingClass:
		return NSLOCTEXT("LGRPlacement", "InvalidBuildingClass", "建筑配置无效");
	case ELGRPlacementFailureReason::NotBuildingPhase:
		return NSLOCTEXT("LGRPlacement", "NotBuildingPhase", "当前不是建设阶段");
	case ELGRPlacementFailureReason::CellUnavailable:
		return NSLOCTEXT("LGRPlacement", "CellUnavailable", "所选区域无法建造");
	case ELGRPlacementFailureReason::InsufficientPopulation:
		return NSLOCTEXT("LGRPlacement", "InsufficientPopulation", "空闲人口不足");
	case ELGRPlacementFailureReason::SpawnFailed:
		return NSLOCTEXT("LGRPlacement", "SpawnFailed", "建筑生成失败");
	default:
		return NSLOCTEXT("LGRPlacement", "UnknownFailure", "无法建造");
	}
}

void ALGRPlayerController::EnsureGridHighlight()
{
	if (IsValid(GridHighlight) || !IsValid(GetWorld()))
	{
		return;
	}

	UClass* HighlightClass = GridHighlightClass
		? GridHighlightClass.Get()
		: ALGRGridHighlight::StaticClass();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GridHighlight = GetWorld()->SpawnActor<ALGRGridHighlight>(
		HighlightClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);

	if (IsValid(GridHighlight) && IsValid(GridManager))
	{
		GridHighlight->HideHighlight();
	}
}

void ALGRPlayerController::UpdateGridHover()
{
	if (!IsValid(GridManager) && !RefreshGridManager())
	{
		ClearGridHover();
		return;
	}

	EnsureGridHighlight();

	FHitResult GroundHit;
	FIntPoint NewGridCoordinates;
	if (!GetMouseGroundHit(GroundHit)
		|| !GridManager->TryWorldToGrid(GroundHit.ImpactPoint, NewGridCoordinates))
	{
		ClearGridHover();
		return;
	}

	const bool bNewCanPlace = GridManager->IsCellFree(NewGridCoordinates);
	const bool bCellChanged = !bHasHoveredGridCell
		|| HoveredGridCoordinates != NewGridCoordinates
		|| bHoveredCellCanPlace != bNewCanPlace;

	HoveredGridCoordinates = NewGridCoordinates;
	bHasHoveredGridCell = true;
	bHoveredCellCanPlace = bNewCanPlace;

	UpdatePlacementHighlight();

	if (bCellChanged)
	{
		OnHoveredGridCellChanged.Broadcast(HoveredGridCoordinates, bHoveredCellCanPlace);
	}
}

void ALGRPlayerController::ClearGridHover()
{
	bHasHoveredGridCell = false;
	bHoveredCellCanPlace = false;

	UpdatePlacementHighlight();
}

void ALGRPlayerController::UpdatePlacementHighlight()
{
	if (!IsValid(GridHighlight) || !IsValid(GridManager))
	{
		return;
	}

	const bool bUseSelectedCell = bHasSelectedGridCell;
	if (!bUseSelectedCell && !bHasHoveredGridCell)
	{
		GridHighlight->HideHighlight();
		return;
	}

	const FIntPoint Coordinates = bUseSelectedCell
		? SelectedGridCoordinates
		: HoveredGridCoordinates;
	FIntPoint FootprintSize(1, 1);
	if (bUseSelectedCell && SelectedBuildingClass)
	{
		if (const ALGRBuildingBase* BuildingDefaults = SelectedBuildingClass->GetDefaultObject<ALGRBuildingBase>())
		{
			FootprintSize = BuildingDefaults->GetFootprintSize();
		}
	}

	const bool bCanPlace = bUseSelectedCell && SelectedBuildingClass
		? GetPlacementFailureReasonAt(Coordinates) == ELGRPlacementFailureReason::None
		: GridManager->CanOccupyArea(Coordinates, FootprintSize);
	GridHighlight->SetFootprintSize(GridManager->GetCellSize(), FootprintSize);
	GridHighlight->ShowHighlight(
		GridManager->GridAreaToWorld(Coordinates, FootprintSize, GridHighlightZOffset),
		bCanPlace);
}

#include "LGRPlayerController.h"

#include "../Buildings/LGRArrowTowerBuilding.h"
#include "../Buildings/LGRBlacksmithBuilding.h"
#include "../Buildings/LGRBuildingBase.h"
#include "../Buildings/LGRGardenBuilding.h"
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
		EnsureRangeHighlights();
	}
}

void ALGRPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GridHighlight))
	{
		GridHighlight->Destroy();
		GridHighlight = nullptr;
	}
	if (IsValid(PrimaryRangeHighlight))
	{
		PrimaryRangeHighlight->Destroy();
		PrimaryRangeHighlight = nullptr;
	}
	if (IsValid(SecondaryRangeHighlight))
	{
		SecondaryRangeHighlight->Destroy();
		SecondaryRangeHighlight = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ALGRPlayerController::PlayerTick(const float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController())
	{
		if (SelectedPlacedBuilding && !IsValid(SelectedPlacedBuilding))
		{
			ClearPlacedBuildingSelection();
		}
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
	FHitResult CursorHit;
	if (GetMouseGroundHit(CursorHit))
	{
		if (ALGRBuildingBase* HitBuilding = Cast<ALGRBuildingBase>(CursorHit.GetActor()))
		{
			return SelectPlacedBuilding(HitBuilding);
		}
	}

	if (!bHasHoveredGridCell || (!IsValid(GridManager) && !RefreshGridManager()))
	{
		ClearPlacedBuildingSelection();
		ClearSelectedGridCell();
		return false;
	}

	if (!GridManager->IsCellFree(HoveredGridCoordinates))
	{
		ClearPlacedBuildingSelection();
		ClearSelectedGridCell();
		return false;
	}

	ClearPlacedBuildingSelection();
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

bool ALGRPlayerController::SelectPlacedBuilding(ALGRBuildingBase* Building)
{
	if (!IsValid(Building) || !Building->IsPlacedOnGrid() || Building->GetCurrentHealth() <= 0.0f)
	{
		ClearPlacedBuildingSelection();
		return false;
	}

	if (bHasSelectedGridCell)
	{
		const FIntPoint PreviousSelection = SelectedGridCoordinates;
		bHasSelectedGridCell = false;
		SelectedBuildingClass = nullptr;
		OnGridCellSelectionChanged.Broadcast(PreviousSelection, false);
	}

	SelectedPlacedBuilding = Building;
	UpdatePlacementHighlight();
	UpdateSelectedBuildingRanges();
	OnPlacedBuildingSelectionChanged.Broadcast(SelectedPlacedBuilding);
	return true;
}

void ALGRPlayerController::ClearPlacedBuildingSelection()
{
	if (!SelectedPlacedBuilding)
	{
		HideRangeHighlights();
		return;
	}

	SelectedPlacedBuilding = nullptr;
	HideRangeHighlights();
	OnPlacedBuildingSelectionChanged.Broadcast(nullptr);
	UpdatePlacementHighlight();
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
	if (!GameMode->TryConsumeBuildAction())
	{
		return false;
	}

	if (!GameMode->TryAssignPopulation(PopulationCost))
	{
		GameMode->RefundBuildAction();
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
		GameMode->RefundBuildAction();
		return false;
	}

	if (!GridManager->OccupyArea(GridCoordinates, FootprintSize, NewBuilding))
	{
		NewBuilding->Destroy();
		GameMode->RefundAssignedPopulation(PopulationCost);
		GameMode->RefundBuildAction();
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

	if (!GameMode->HasRemainingBuildActions())
	{
		return ELGRPlacementFailureReason::NoBuildActionsRemaining;
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
	case ELGRPlacementFailureReason::NoBuildActionsRemaining:
		return NSLOCTEXT("LGRPlacement", "NoBuildActionsRemaining", "今日建造次数已用完");
	default:
		return NSLOCTEXT("LGRPlacement", "UnknownFailure", "无法建造");
	}
}

void ALGRPlayerController::EnsureRangeHighlights()
{
	if (!IsValid(GetWorld()))
	{
		return;
	}

	auto SpawnRangeHighlight = [this]() -> ALGRGridHighlight*
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ALGRGridHighlight* Highlight = GetWorld()->SpawnActor<ALGRGridHighlight>(
			ALGRGridHighlight::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (IsValid(Highlight))
		{
			Highlight->SetCellCoverage(1.0f);
			Highlight->HideHighlight();
		}
		return Highlight;
	};

	if (!IsValid(PrimaryRangeHighlight))
	{
		PrimaryRangeHighlight = SpawnRangeHighlight();
	}
	if (!IsValid(SecondaryRangeHighlight))
	{
		SecondaryRangeHighlight = SpawnRangeHighlight();
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

	if (IsValid(SelectedPlacedBuilding))
	{
		GridHighlight->HideHighlight();
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

void ALGRPlayerController::UpdateSelectedBuildingRanges()
{
	if (!IsValid(SelectedPlacedBuilding)
		|| (!IsValid(GridManager) && !RefreshGridManager()))
	{
		HideRangeHighlights();
		return;
	}

	EnsureRangeHighlights();
	if (!IsValid(PrimaryRangeHighlight) || !IsValid(SecondaryRangeHighlight))
	{
		return;
	}

	const float CellSize = GridManager->GetCellSize();
	const FVector Center = GridManager->GridAreaToWorld(
		SelectedPlacedBuilding->GetGridOrigin(),
		SelectedPlacedBuilding->GetFootprintSize(),
		RangeHighlightZOffset);
	const FRotator GridRotation = GridManager->GetActorRotation();
	PrimaryRangeHighlight->SetActorRotation(GridRotation);
	SecondaryRangeHighlight->SetActorRotation(GridRotation);
	HideRangeHighlights();

	if (const ALGRArrowTowerBuilding* Tower = Cast<ALGRArrowTowerBuilding>(SelectedPlacedBuilding))
	{
		PrimaryRangeHighlight->SetCircleRadius(CellSize, Tower->GetAttackRadiusCells());
		const int32 NoiseDiameter = Tower->GetNoiseRadius() * 2 + 1;
		SecondaryRangeHighlight->SetFootprintSize(CellSize, FIntPoint(NoiseDiameter, NoiseDiameter));
		SecondaryRangeHighlight->ShowHighlight(Center, false);
		PrimaryRangeHighlight->ShowHighlight(Center + FVector(0.0f, 0.0f, 1.0f), true);
	}
	else if (const ALGRGardenBuilding* Garden = Cast<ALGRGardenBuilding>(SelectedPlacedBuilding))
	{
		const int32 EffectDiameter = Garden->GetEffectRadius() * 2 + 1;
		PrimaryRangeHighlight->SetFootprintSize(CellSize, FIntPoint(EffectDiameter, EffectDiameter));
		PrimaryRangeHighlight->ShowHighlight(Center, true);
	}
	else if (const ALGRBlacksmithBuilding* Blacksmith = Cast<ALGRBlacksmithBuilding>(SelectedPlacedBuilding))
	{
		PrimaryRangeHighlight->SetCircleRadius(CellSize, Blacksmith->GetEffectRadiusCells());
		PrimaryRangeHighlight->ShowHighlight(Center, true);
	}
}

void ALGRPlayerController::HideRangeHighlights()
{
	if (IsValid(PrimaryRangeHighlight))
	{
		PrimaryRangeHighlight->HideHighlight();
	}
	if (IsValid(SecondaryRangeHighlight))
	{
		SecondaryRangeHighlight->HideHighlight();
	}
}

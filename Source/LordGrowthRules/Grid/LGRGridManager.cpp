#include "LGRGridManager.h"

#include "../Buildings/LGRBuildingBase.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"

ALGRGridManager::ALGRGridManager()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void ALGRGridManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeGrid();
	if (bSpawnLordManorOnBeginPlay)
	{
		SpawnLordManorAtCenter();
	}

	if (bDrawDebugGridOnBeginPlay)
	{
		DrawDebugGrid(-1.0f);
	}
}

bool ALGRGridManager::SpawnLordManorAtCenter()
{
	if (IsValid(LordManor))
	{
		return true;
	}

	if (!LordManorClass || !IsValid(GetWorld()))
	{
		UE_LOG(LogTemp, Warning, TEXT("LGRGridManager cannot spawn the Lord Manor: LordManorClass is not configured."));
		return false;
	}

	const FIntPoint ManorFootprint(2, 2);
	if (GridWidth < ManorFootprint.X || GridHeight < ManorFootprint.Y)
	{
		UE_LOG(LogTemp, Error, TEXT("LGRGridManager grid is too small for the 2x2 Lord Manor."));
		return false;
	}

	LordManorGridOrigin = FIntPoint(
		(GridWidth - ManorFootprint.X) / 2,
		(GridHeight - ManorFootprint.Y) / 2);

	if (!CanOccupyArea(LordManorGridOrigin, ManorFootprint))
	{
		UE_LOG(LogTemp, Error, TEXT("LGRGridManager center cells are unavailable for the Lord Manor."));
		return false;
	}

	if ((GridWidth % 2) != 0 || (GridHeight % 2) != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("A 2x2 Lord Manor cannot be perfectly centered on an odd-sized grid."));
	}

	const FVector SpawnLocation = GridAreaToWorld(LordManorGridOrigin, ManorFootprint);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	LordManor = GetWorld()->SpawnActor<ALGRBuildingBase>(
		LordManorClass,
		SpawnLocation,
		GetActorRotation(),
		SpawnParameters);

	if (!IsValid(LordManor))
	{
		UE_LOG(LogTemp, Error, TEXT("LGRGridManager failed to spawn the Lord Manor actor."));
		return false;
	}

	LordManor->SetFootprintSizeForSystem(ManorFootprint);

	if (!OccupyArea(LordManorGridOrigin, ManorFootprint, LordManor))
	{
		LordManor->Destroy();
		LordManor = nullptr;
		return false;
	}

	LordManor->InitializePlacedBuilding(this, LordManorGridOrigin);
	return true;
}

void ALGRGridManager::InitializeGrid()
{
	GridWidth = FMath::Max(1, GridWidth);
	GridHeight = FMath::Max(1, GridHeight);
	CellSize = FMath::Max(1.0f, CellSize);

	Cells.Reset(GridWidth * GridHeight);
	Cells.Reserve(GridWidth * GridHeight);

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			FLGRGridCell& NewCell = Cells.AddDefaulted_GetRef();
			NewCell.Coordinates = FIntPoint(X, Y);
			NewCell.bBuildable = true;
			NewCell.OccupyingActor = nullptr;
		}
	}
}

FIntPoint ALGRGridManager::WorldToGrid(const FVector& WorldLocation) const
{
	const FVector LocalLocation = GetActorTransform().InverseTransformPositionNoScale(WorldLocation);
	return FIntPoint(
		FMath::FloorToInt(LocalLocation.X / CellSize),
		FMath::FloorToInt(LocalLocation.Y / CellSize));
}

FVector ALGRGridManager::GridToWorld(const FIntPoint& GridCoordinates, const float ZOffset) const
{
	const FVector LocalCellCenter(
		(static_cast<float>(GridCoordinates.X) + 0.5f) * CellSize,
		(static_cast<float>(GridCoordinates.Y) + 0.5f) * CellSize,
		ZOffset);

	return GetActorTransform().TransformPositionNoScale(LocalCellCenter);
}

FVector ALGRGridManager::GridAreaToWorld(
	const FIntPoint& OriginCoordinates,
	const FIntPoint& FootprintSize,
	const float ZOffset) const
{
	const FIntPoint SafeFootprint(FMath::Max(1, FootprintSize.X), FMath::Max(1, FootprintSize.Y));
	const FVector LocalAreaCenter(
		(static_cast<float>(OriginCoordinates.X) + static_cast<float>(SafeFootprint.X) * 0.5f) * CellSize,
		(static_cast<float>(OriginCoordinates.Y) + static_cast<float>(SafeFootprint.Y) * 0.5f) * CellSize,
		ZOffset);

	return GetActorTransform().TransformPositionNoScale(LocalAreaCenter);
}

bool ALGRGridManager::TryWorldToGrid(const FVector& WorldLocation, FIntPoint& OutGridCoordinates) const
{
	OutGridCoordinates = WorldToGrid(WorldLocation);
	return IsInBounds(OutGridCoordinates);
}

bool ALGRGridManager::IsInBounds(const FIntPoint& GridCoordinates) const
{
	return GridCoordinates.X >= 0
		&& GridCoordinates.Y >= 0
		&& GridCoordinates.X < GridWidth
		&& GridCoordinates.Y < GridHeight;
}

bool ALGRGridManager::IsCellBuildable(const FIntPoint& GridCoordinates) const
{
	const int32 Index = CoordinatesToIndex(GridCoordinates);
	return Cells.IsValidIndex(Index) && Cells[Index].bBuildable;
}

bool ALGRGridManager::IsCellFree(const FIntPoint& GridCoordinates) const
{
	const int32 Index = CoordinatesToIndex(GridCoordinates);
	return Cells.IsValidIndex(Index) && Cells[Index].IsFree();
}

bool ALGRGridManager::TryGetCell(const FIntPoint& GridCoordinates, FLGRGridCell& OutCell) const
{
	const int32 Index = CoordinatesToIndex(GridCoordinates);
	if (!Cells.IsValidIndex(Index))
	{
		return false;
	}

	OutCell = Cells[Index];
	return true;
}

AActor* ALGRGridManager::GetOccupyingActor(const FIntPoint& GridCoordinates) const
{
	const int32 Index = CoordinatesToIndex(GridCoordinates);
	if (!Cells.IsValidIndex(Index))
	{
		return nullptr;
	}

	return IsValid(Cells[Index].OccupyingActor.Get()) ? Cells[Index].OccupyingActor.Get() : nullptr;
}

bool ALGRGridManager::SetCellBuildable(const FIntPoint& GridCoordinates, const bool bNewBuildable)
{
	const int32 Index = CoordinatesToIndex(GridCoordinates);
	if (!Cells.IsValidIndex(Index))
	{
		return false;
	}

	if (!bNewBuildable && IsValid(Cells[Index].OccupyingActor.Get()))
	{
		return false;
	}

	Cells[Index].bBuildable = bNewBuildable;
	return true;
}

bool ALGRGridManager::OccupyCell(const FIntPoint& GridCoordinates, AActor* OccupyingActor)
{
	return OccupyArea(GridCoordinates, FIntPoint(1, 1), OccupyingActor);
}

bool ALGRGridManager::ReleaseCell(const FIntPoint& GridCoordinates, AActor* ExpectedOccupyingActor)
{
	const int32 Index = CoordinatesToIndex(GridCoordinates);
	if (!Cells.IsValidIndex(Index))
	{
		return false;
	}

	AActor* CurrentOccupyingActor = Cells[Index].OccupyingActor.Get();
	if (ExpectedOccupyingActor != nullptr && CurrentOccupyingActor != ExpectedOccupyingActor)
	{
		return false;
	}

	if (!IsValid(CurrentOccupyingActor))
	{
		Cells[Index].OccupyingActor = nullptr;
		return false;
	}

	Cells[Index].OccupyingActor = nullptr;
	return true;
}

bool ALGRGridManager::CanOccupyArea(const FIntPoint& OriginCoordinates, const FIntPoint& FootprintSize) const
{
	if (!IsValidFootprint(FootprintSize))
	{
		return false;
	}

	for (int32 Y = 0; Y < FootprintSize.Y; ++Y)
	{
		for (int32 X = 0; X < FootprintSize.X; ++X)
		{
			const FIntPoint Coordinates = OriginCoordinates + FIntPoint(X, Y);
			if (!IsCellFree(Coordinates))
			{
				return false;
			}
		}
	}

	return true;
}

bool ALGRGridManager::OccupyArea(const FIntPoint& OriginCoordinates, const FIntPoint& FootprintSize, AActor* OccupyingActor)
{
	if (!IsValid(OccupyingActor) || !CanOccupyArea(OriginCoordinates, FootprintSize))
	{
		return false;
	}

	for (int32 Y = 0; Y < FootprintSize.Y; ++Y)
	{
		for (int32 X = 0; X < FootprintSize.X; ++X)
		{
			const FIntPoint Coordinates = OriginCoordinates + FIntPoint(X, Y);
			Cells[CoordinatesToIndex(Coordinates)].OccupyingActor = OccupyingActor;
		}
	}

	return true;
}

int32 ALGRGridManager::ReleaseAllCellsOccupiedBy(AActor* OccupyingActor)
{
	if (!IsValid(OccupyingActor))
	{
		return 0;
	}

	int32 ReleasedCellCount = 0;
	for (FLGRGridCell& Cell : Cells)
	{
		if (Cell.OccupyingActor.Get() == OccupyingActor)
		{
			Cell.OccupyingActor = nullptr;
			++ReleasedCellCount;
		}
	}

	return ReleasedCellCount;
}

void ALGRGridManager::DrawDebugGrid(const float Duration) const
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || GridWidth <= 0 || GridHeight <= 0 || CellSize <= 0.0f)
	{
		return;
	}

	const bool bPersistentLines = Duration < 0.0f;
	const float LifeTime = bPersistentLines ? -1.0f : Duration;

	for (int32 X = 0; X <= GridWidth; ++X)
	{
		const float LocalX = static_cast<float>(X) * CellSize;
		const FVector Start = GetActorTransform().TransformPositionNoScale(FVector(LocalX, 0.0f, DebugLineZOffset));
		const FVector End = GetActorTransform().TransformPositionNoScale(
			FVector(LocalX, static_cast<float>(GridHeight) * CellSize, DebugLineZOffset));

		DrawDebugLine(World, Start, End, DebugGridColor, bPersistentLines, LifeTime, 0, DebugLineThickness);
	}

	for (int32 Y = 0; Y <= GridHeight; ++Y)
	{
		const float LocalY = static_cast<float>(Y) * CellSize;
		const FVector Start = GetActorTransform().TransformPositionNoScale(FVector(0.0f, LocalY, DebugLineZOffset));
		const FVector End = GetActorTransform().TransformPositionNoScale(
			FVector(static_cast<float>(GridWidth) * CellSize, LocalY, DebugLineZOffset));

		DrawDebugLine(World, Start, End, DebugGridColor, bPersistentLines, LifeTime, 0, DebugLineThickness);
	}
}

int32 ALGRGridManager::CoordinatesToIndex(const FIntPoint& GridCoordinates) const
{
	if (!IsInBounds(GridCoordinates))
	{
		return INDEX_NONE;
	}

	return GridCoordinates.Y * GridWidth + GridCoordinates.X;
}

bool ALGRGridManager::IsValidFootprint(const FIntPoint& FootprintSize) const
{
	return FootprintSize.X > 0 && FootprintSize.Y > 0;
}

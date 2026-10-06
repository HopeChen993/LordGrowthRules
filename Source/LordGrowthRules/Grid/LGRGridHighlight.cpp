#include "LGRGridHighlight.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ALGRGridHighlight::ALGRGridHighlight()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	HighlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighlightMesh"));
	HighlightMesh->SetupAttachment(SceneRoot);
	HighlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightMesh->SetGenerateOverlapEvents(false);
	HighlightMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		PlaneMesh = PlaneMeshFinder.Object;
		HighlightMesh->SetStaticMesh(PlaneMesh);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CircleMeshFinder(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CircleMeshFinder.Succeeded())
	{
		CircleMesh = CircleMeshFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AvailableMaterialFinder(
		TEXT("/Game/Materials/MI_GridAvailable.MI_GridAvailable"));
	if (AvailableMaterialFinder.Succeeded())
	{
		AvailableMaterial = AvailableMaterialFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlockedMaterialFinder(
		TEXT("/Game/Materials/MI_GridBlocked.MI_GridBlocked"));
	if (BlockedMaterialFinder.Succeeded())
	{
		BlockedMaterial = BlockedMaterialFinder.Object;
	}

	SetActorHiddenInGame(true);
}

void ALGRGridHighlight::SetCellSize(const float InCellSize)
{
	SetFootprintSize(InCellSize, FIntPoint(1, 1));
}

void ALGRGridHighlight::SetFootprintSize(const float InCellSize, const FIntPoint InFootprintSize)
{
	if (IsValid(PlaneMesh))
	{
		HighlightMesh->SetStaticMesh(PlaneMesh);
	}

	const float SafeSourceMeshSize = FMath::Max(1.0f, SourceMeshSize);
	const float SafeCellSize = FMath::Max(1.0f, InCellSize);
	const float Coverage = FMath::Clamp(CellCoverage, 0.1f, 1.0f);
	const int32 FootprintX = FMath::Max(1, InFootprintSize.X);
	const int32 FootprintY = FMath::Max(1, InFootprintSize.Y);
	const float ScaleX = SafeCellSize * static_cast<float>(FootprintX) * Coverage / SafeSourceMeshSize;
	const float ScaleY = SafeCellSize * static_cast<float>(FootprintY) * Coverage / SafeSourceMeshSize;
	HighlightMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
}

void ALGRGridHighlight::SetCircleRadius(const float InCellSize, const float RadiusInCells)
{
	if (IsValid(CircleMesh))
	{
		HighlightMesh->SetStaticMesh(CircleMesh);
	}

	const float SafeSourceMeshSize = FMath::Max(1.0f, SourceMeshSize);
	const float Diameter = FMath::Max(0.0f, RadiusInCells) * 2.0f * FMath::Max(1.0f, InCellSize);
	const float DiameterScale = Diameter / SafeSourceMeshSize;
	const float ThicknessScale = FMath::Max(0.1f, CircleThickness) / SafeSourceMeshSize;
	HighlightMesh->SetRelativeScale3D(FVector(DiameterScale, DiameterScale, ThicknessScale));
}

void ALGRGridHighlight::SetCellCoverage(const float InCellCoverage)
{
	CellCoverage = FMath::Clamp(InCellCoverage, 0.1f, 1.0f);
}

void ALGRGridHighlight::ShowHighlight(const FVector& WorldLocation, const bool bCanPlace)
{
	SetActorLocation(WorldLocation);
	SetActorHiddenInGame(false);

	if (UMaterialInterface* DesiredMaterial = bCanPlace ? AvailableMaterial.Get() : BlockedMaterial.Get())
	{
		HighlightMesh->SetMaterial(0, DesiredMaterial);
	}

	if (!bHasPlacementState || bLastCanPlace != bCanPlace)
	{
		bHasPlacementState = true;
		bLastCanPlace = bCanPlace;
		OnPlacementStateChanged(bCanPlace);
	}
}

void ALGRGridHighlight::HideHighlight()
{
	SetActorHiddenInGame(true);
}

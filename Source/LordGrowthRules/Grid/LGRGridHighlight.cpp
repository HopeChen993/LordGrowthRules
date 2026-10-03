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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		HighlightMesh->SetStaticMesh(PlaneMesh.Object);
	}

	SetActorHiddenInGame(true);
}

void ALGRGridHighlight::SetCellSize(const float InCellSize)
{
	SetFootprintSize(InCellSize, FIntPoint(1, 1));
}

void ALGRGridHighlight::SetFootprintSize(const float InCellSize, const FIntPoint InFootprintSize)
{
	const float SafeSourceMeshSize = FMath::Max(1.0f, SourceMeshSize);
	const float SafeCellSize = FMath::Max(1.0f, InCellSize);
	const float Coverage = FMath::Clamp(CellCoverage, 0.1f, 1.0f);
	const int32 FootprintX = FMath::Max(1, InFootprintSize.X);
	const int32 FootprintY = FMath::Max(1, InFootprintSize.Y);
	const float ScaleX = SafeCellSize * static_cast<float>(FootprintX) * Coverage / SafeSourceMeshSize;
	const float ScaleY = SafeCellSize * static_cast<float>(FootprintY) * Coverage / SafeSourceMeshSize;
	HighlightMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
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

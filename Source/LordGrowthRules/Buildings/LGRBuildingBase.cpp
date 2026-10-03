#include "LGRBuildingBase.h"

#include "../Core/LGRGameModeBase.h"
#include "../Grid/LGRGridManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ALGRBuildingBase::ALGRBuildingBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(SceneRoot);
	BuildingMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BuildingMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ALGRBuildingBase::BeginPlay()
{
	Super::BeginPlay();

	MaxHealth = FMath::Max(1.0f, MaxHealth);
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float ALGRBuildingBase::TakeDamage(
	const float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);
	return ApplyBuildingDamage(AppliedDamage, DamageCauser);
}

float ALGRBuildingBase::ApplyBuildingDamage(const float DamageAmount, AActor* DamageCauser)
{
	if (DamageAmount <= 0.0f || CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float ActualDamage = FMath::Min(DamageAmount, CurrentHealth);
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			if (ALGRGameModeBase* GameMode = World->GetAuthGameMode<ALGRGameModeBase>())
			{
				if (BuildingType == ELGRBuildingType::LordManor)
				{
					GameMode->EndGame(false);
				}
				else if (bPlacedOnGrid && PopulationCost > 0)
				{
					GameMode->MoveAssignedPopulationToRecovery(PopulationCost);
				}
			}
		}

		Destroy();
	}

	return ActualDamage;
}

float ALGRBuildingBase::GetHealthNormalized() const
{
	return MaxHealth > 0.0f
		? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 0.0f;
}

void ALGRBuildingBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bPlacedOnGrid && IsValid(GridManager))
	{
		GridManager->ReleaseAllCellsOccupiedBy(this);
		bPlacedOnGrid = false;
	}

	Super::EndPlay(EndPlayReason);
}

void ALGRBuildingBase::InitializePlacedBuilding(ALGRGridManager* InGridManager, const FIntPoint InGridOrigin)
{
	GridManager = InGridManager;
	GridOrigin = InGridOrigin;
	bPlacedOnGrid = IsValid(GridManager);

	if (!bPlacedOnGrid || !bAutoScalePrototypeMesh)
	{
		return;
	}

	const FIntPoint SafeFootprint = GetFootprintSize();
	const float CellSize = GridManager->GetCellSize();
	const float SafeSourceSize = FMath::Max(1.0f, PrototypeMeshSourceSize);
	const FVector PrototypeScale(
		CellSize * static_cast<float>(SafeFootprint.X) * 0.8f / SafeSourceSize,
		CellSize * static_cast<float>(SafeFootprint.Y) * 0.8f / SafeSourceSize,
		FMath::Max(1.0f, PrototypeHeight) / SafeSourceSize);

	BuildingMesh->SetRelativeScale3D(PrototypeScale);
	BuildingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, FMath::Max(1.0f, PrototypeHeight) * 0.5f));
}

FIntPoint ALGRBuildingBase::GetFootprintSize() const
{
	return FIntPoint(FMath::Max(1, FootprintSize.X), FMath::Max(1, FootprintSize.Y));
}

void ALGRBuildingBase::SetFootprintSizeForSystem(const FIntPoint& InFootprintSize)
{
	FootprintSize = FIntPoint(
		FMath::Max(1, InFootprintSize.X),
		FMath::Max(1, InFootprintSize.Y));
}

#include "LGRBuildingSettlementComponent.h"

#include "../Buildings/LGRArrowTowerBuilding.h"
#include "../Buildings/LGRBlacksmithBuilding.h"
#include "../Buildings/LGRGardenBuilding.h"
#include "../Buildings/LGRResidenceBuilding.h"
#include "../Core/LGRGameModeBase.h"
#include "EngineUtils.h"

ULGRBuildingSettlementComponent::ULGRBuildingSettlementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool ULGRBuildingSettlementComponent::ResolveDailyBuildingRules()
{
	ALGRGameModeBase* GameMode = Cast<ALGRGameModeBase>(GetOwner());
	UWorld* World = GetWorld();
	if (!IsValid(GameMode) || !IsValid(World))
	{
		return false;
	}

	TArray<ALGRResidenceBuilding*> Residences;
	TArray<ALGRArrowTowerBuilding*> ArrowTowers;
	TArray<ALGRGardenBuilding*> Gardens;
	TArray<ALGRBlacksmithBuilding*> Blacksmiths;

	for (TActorIterator<ALGRResidenceBuilding> It(World); It; ++It)
	{
		if (IsValid(*It) && It->IsPlacedOnGrid() && It->GetCurrentHealth() > 0.0f)
		{
			Residences.Add(*It);
		}
	}

	for (TActorIterator<ALGRArrowTowerBuilding> It(World); It; ++It)
	{
		if (IsValid(*It) && It->IsPlacedOnGrid() && It->GetCurrentHealth() > 0.0f)
		{
			ArrowTowers.Add(*It);
		}
	}

	for (TActorIterator<ALGRGardenBuilding> It(World); It; ++It)
	{
		if (IsValid(*It) && It->IsPlacedOnGrid() && It->GetCurrentHealth() > 0.0f)
		{
			Gardens.Add(*It);
		}
	}

	for (TActorIterator<ALGRBlacksmithBuilding> It(World); It; ++It)
	{
		if (IsValid(*It) && It->IsPlacedOnGrid() && It->GetCurrentHealth() > 0.0f)
		{
			Blacksmiths.Add(*It);
		}
	}

	for (ALGRGardenBuilding* Garden : Gardens)
	{
		bool bDisabledByPollution = false;
		for (const ALGRBlacksmithBuilding* Blacksmith : Blacksmiths)
		{
			if (Blacksmith->IsGridPositionInEffectRange(Garden->GetGridOrigin()))
			{
				bDisabledByPollution = true;
				break;
			}
		}
		Garden->SetDisabledByPollution(bDisabledByPollution);
	}

	int32 TotalPopulationGain = 0;
	for (ALGRResidenceBuilding* Residence : Residences)
	{
		const FIntPoint ResidenceOrigin = Residence->GetGridOrigin();
		int32 TotalNoise = 0;
		int32 TotalNoiseReduction = 0;
		int32 PopulationBonus = 0;

		for (const ALGRArrowTowerBuilding* ArrowTower : ArrowTowers)
		{
			const int32 Distance = GetSquareGridDistance(ResidenceOrigin, ArrowTower->GetGridOrigin());
			if (Distance <= ArrowTower->GetNoiseRadius())
			{
				const int32 FalloffDistance = FMath::Max(0, Distance - 1);
				const int32 NoiseContribution = FMath::Max(
					0,
					ArrowTower->GetNoiseStrength()
						- FalloffDistance * ArrowTower->GetNoiseFalloffPerCell());
				TotalNoise += NoiseContribution;
			}
		}

		for (const ALGRBlacksmithBuilding* Blacksmith : Blacksmiths)
		{
			if (Blacksmith->IsGridPositionInEffectRange(ResidenceOrigin))
			{
				TotalNoise += Blacksmith->GetNoiseStrength();
			}
		}

		for (const ALGRGardenBuilding* Garden : Gardens)
		{
			if (Garden->IsDisabledByPollution())
			{
				continue;
			}

			const int32 Distance = GetSquareGridDistance(ResidenceOrigin, Garden->GetGridOrigin());
			if (Distance <= Garden->GetEffectRadius())
			{
				TotalNoiseReduction += Garden->GetNoiseReduction();
				PopulationBonus += Garden->GetResidencePopulationBonus();
			}
		}

		const int32 EffectiveNoise = FMath::Max(0, TotalNoise - TotalNoiseReduction);
		const int32 PopulationGain = EffectiveNoise <= Residence->GetNoiseTolerance()
			? FMath::Max(0, Residence->GetBasePopulationGain() + PopulationBonus)
			: 0;

		Residence->SetLastSettlementResult(EffectiveNoise, PopulationGain);
		TotalPopulationGain += PopulationGain;
		OnResidenceSettled.Broadcast(Residence, EffectiveNoise, PopulationGain);
	}

	if (TotalPopulationGain > 0)
	{
		GameMode->AddPopulation(TotalPopulationGain);
	}

	OnDailyBuildingSettlementCompleted.Broadcast(TotalPopulationGain);
	return true;
}

int32 ULGRBuildingSettlementComponent::GetSquareGridDistance(
	const FIntPoint& A,
	const FIntPoint& B)
{
	return FMath::Max(FMath::Abs(A.X - B.X), FMath::Abs(A.Y - B.Y));
}

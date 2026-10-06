#include "LGRArrowTowerBuilding.h"

#include "LGRBlacksmithBuilding.h"
#include "EngineUtils.h"

ALGRArrowTowerBuilding::ALGRArrowTowerBuilding()
{
	BuildingType = ELGRBuildingType::ArrowTower;
	FootprintSize = FIntPoint(1, 1);
	PopulationCost = 5;
	MaxHealth = 100.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "ArrowTowerName", "箭塔");
	BuildingDescription = NSLOCTEXT(
		"LGRBuilding",
		"ArrowTowerDescription",
		"攻击半径为3格，并在周围5×5格内产生噪音。");
}

float ALGRArrowTowerBuilding::GetEffectiveAttackDamage() const
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsPlacedOnGrid())
	{
		return FMath::Max(0.0f, AttackDamage);
	}

	for (TActorIterator<ALGRBlacksmithBuilding> It(World); It; ++It)
	{
		const ALGRBlacksmithBuilding* Blacksmith = *It;
		if (IsValid(Blacksmith)
			&& Blacksmith->IsPlacedOnGrid()
			&& Blacksmith->GetCurrentHealth() > 0.0f
			&& Blacksmith->IsGridPositionInEffectRange(GetGridOrigin()))
		{
			return FMath::Max(0.0f, AttackDamage) * Blacksmith->GetAttackDamageMultiplier();
		}
	}

	return FMath::Max(0.0f, AttackDamage);
}

bool ALGRArrowTowerBuilding::IsBuffedByBlacksmith() const
{
	return GetEffectiveAttackDamage() > FMath::Max(0.0f, AttackDamage);
}

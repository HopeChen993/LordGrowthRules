#include "LGRBlacksmithBuilding.h"

ALGRBlacksmithBuilding::ALGRBlacksmithBuilding()
{
	BuildingType = ELGRBuildingType::Blacksmith;
	FootprintSize = FIntPoint(1, 1);
	PopulationCost = 5;
	MaxHealth = 75.0f;
	BuildingDisplayName = NSLOCTEXT("LGRBuilding", "BlacksmithName", "铁匠铺");
	BuildingDescription = NSLOCTEXT(
		"LGRBuilding",
		"BlacksmithDescription",
		"使半径1.5格内的箭塔攻击力提高10%，但会产生噪音并使范围内花园失效。");
}

bool ALGRBlacksmithBuilding::IsGridPositionInEffectRange(const FIntPoint GridPosition) const
{
	const FVector2D Offset(
		static_cast<double>(GridPosition.X - GetGridOrigin().X),
		static_cast<double>(GridPosition.Y - GetGridOrigin().Y));
	const float SafeRadius = FMath::Max(0.0f, EffectRadiusCells);
	return Offset.SizeSquared() <= FMath::Square(SafeRadius);
}

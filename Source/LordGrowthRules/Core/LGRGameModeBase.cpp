#include "LGRGameModeBase.h"

#include "../Systems/LGRBuildingSettlementComponent.h"

ALGRGameModeBase::ALGRGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	BuildingSettlementComponent = CreateDefaultSubobject<ULGRBuildingSettlementComponent>(
		TEXT("BuildingSettlementComponent"));
}

void ALGRGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentDay = FMath::Max(1, CurrentDay);
	CurrentPhase = ELGRGamePhase::Building;
	TotalPopulation = FMath::Max(0, InitialPopulation);
	AssignedPopulation = 0;
	RecoveringPopulation = 0;
	OnDayChanged.Broadcast(CurrentDay);
	OnGamePhaseChanged.Broadcast(CurrentPhase, CurrentPhase);
	BroadcastPopulationChanged();
}

int32 ALGRGameModeBase::GetAvailablePopulation() const
{
	return FMath::Max(0, TotalPopulation - AssignedPopulation - RecoveringPopulation);
}

bool ALGRGameModeBase::CanAffordPopulation(const int32 PopulationCost) const
{
	return PopulationCost >= 0 && GetAvailablePopulation() >= PopulationCost;
}

bool ALGRGameModeBase::TryAssignPopulation(const int32 PopulationCost)
{
	if (!CanAffordPopulation(PopulationCost))
	{
		return false;
	}

	AssignedPopulation += PopulationCost;
	BroadcastPopulationChanged();
	return true;
}

bool ALGRGameModeBase::RefundAssignedPopulation(const int32 PopulationAmount)
{
	if (PopulationAmount < 0 || PopulationAmount > AssignedPopulation)
	{
		return false;
	}

	AssignedPopulation -= PopulationAmount;
	BroadcastPopulationChanged();
	return true;
}

bool ALGRGameModeBase::MoveAssignedPopulationToRecovery(const int32 PopulationAmount)
{
	if (PopulationAmount < 0 || PopulationAmount > AssignedPopulation)
	{
		return false;
	}

	AssignedPopulation -= PopulationAmount;
	RecoveringPopulation += PopulationAmount;
	BroadcastPopulationChanged();
	return true;
}

void ALGRGameModeBase::AddPopulation(const int32 PopulationAmount)
{
	if (PopulationAmount <= 0)
	{
		return;
	}

	TotalPopulation += PopulationAmount;
	BroadcastPopulationChanged();
}

void ALGRGameModeBase::RecoverAllPopulation()
{
	if (RecoveringPopulation <= 0)
	{
		return;
	}

	RecoveringPopulation = 0;
	BroadcastPopulationChanged();
}

bool ALGRGameModeBase::RequestEndBuildingPhase()
{
	if (CurrentPhase != ELGRGamePhase::Building)
	{
		return false;
	}

	SetPhase(ELGRGamePhase::Resolving);
	return true;
}

bool ALGRGameModeBase::CompleteDayResolution()
{
	if (CurrentPhase != ELGRGamePhase::Resolving)
	{
		return false;
	}

	if (IsValid(BuildingSettlementComponent))
	{
		BuildingSettlementComponent->ResolveDailyBuildingRules();
	}

	if (CurrentDay % FMath::Max(1, DaysPerWave) == 0)
	{
		SetPhase(ELGRGamePhase::Combat);
	}
	else
	{
		AdvanceToNextDay();
	}

	return true;
}

bool ALGRGameModeBase::CompleteCombatPhase()
{
	if (CurrentPhase != ELGRGamePhase::Combat)
	{
		return false;
	}

	AdvanceToNextDay();
	return true;
}

void ALGRGameModeBase::EndGame(const bool bVictory)
{
	if (CurrentPhase == ELGRGamePhase::GameOver)
	{
		return;
	}

	bPlayerWon = bVictory;
	SetPhase(ELGRGamePhase::GameOver);
	OnGameOver.Broadcast(bPlayerWon);
}

void ALGRGameModeBase::SetPhase(const ELGRGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	const ELGRGamePhase PreviousPhase = CurrentPhase;
	CurrentPhase = NewPhase;
	OnGamePhaseChanged.Broadcast(PreviousPhase, CurrentPhase);
}

void ALGRGameModeBase::AdvanceToNextDay()
{
	RecoverAllPopulation();
	++CurrentDay;
	OnDayChanged.Broadcast(CurrentDay);
	SetPhase(ELGRGamePhase::Building);
}

void ALGRGameModeBase::BroadcastPopulationChanged()
{
	OnPopulationChanged.Broadcast(
		TotalPopulation,
		AssignedPopulation,
		RecoveringPopulation,
		GetAvailablePopulation());
}

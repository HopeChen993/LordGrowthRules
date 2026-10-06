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
	AssignedPopulation = FMath::Clamp(InitialAssignedPopulation, 0, TotalPopulation);
	ResetBuildActionsForNewDay();
	OnDayChanged.Broadcast(CurrentDay);
	OnGamePhaseChanged.Broadcast(CurrentPhase, CurrentPhase);
	BroadcastPopulationChanged();
}

int32 ALGRGameModeBase::GetAvailablePopulation() const
{
	return FMath::Max(0, TotalPopulation - AssignedPopulation);
}

bool ALGRGameModeBase::HasReachedVictoryPopulation() const
{
	return TotalPopulation >= FMath::Max(1, VictoryPopulationTarget);
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

bool ALGRGameModeBase::LoseAssignedPopulation(const int32 PopulationAmount)
{
	if (PopulationAmount < 0
		|| PopulationAmount > AssignedPopulation
		|| PopulationAmount > TotalPopulation)
	{
		return false;
	}

	AssignedPopulation -= PopulationAmount;
	TotalPopulation -= PopulationAmount;
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

bool ALGRGameModeBase::TryConsumeBuildAction()
{
	if (CurrentPhase != ELGRGamePhase::Building || RemainingBuildActions <= 0)
	{
		return false;
	}

	--RemainingBuildActions;
	BroadcastBuildActionsChanged();
	return true;
}

void ALGRGameModeBase::RefundBuildAction()
{
	if (RemainingBuildActions >= MaxBuildActionsThisDay)
	{
		return;
	}

	++RemainingBuildActions;
	BroadcastBuildActionsChanged();
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

	if (HasReachedVictoryPopulation())
	{
		EndGame(true);
		return true;
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
	++CurrentDay;
	SetPhase(ELGRGamePhase::Building);
	ResetBuildActionsForNewDay();
	OnDayChanged.Broadcast(CurrentDay);
}

void ALGRGameModeBase::ResetBuildActionsForNewDay()
{
	const int32 SafePopulationPerAction = FMath::Max(1, PopulationPerBuildAction);
	MaxBuildActionsThisDay = GetAvailablePopulation() / SafePopulationPerAction;
	RemainingBuildActions = MaxBuildActionsThisDay;
	BroadcastBuildActionsChanged();
}

void ALGRGameModeBase::BroadcastBuildActionsChanged()
{
	OnBuildActionsChanged.Broadcast(RemainingBuildActions, MaxBuildActionsThisDay);
}

void ALGRGameModeBase::BroadcastPopulationChanged()
{
	OnPopulationChanged.Broadcast(
		TotalPopulation,
		AssignedPopulation,
		GetAvailablePopulation());
}

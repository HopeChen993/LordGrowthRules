#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LGRGameModeBase.generated.h"

class ULGRBuildingSettlementComponent;

UENUM(BlueprintType)
enum class ELGRGamePhase : uint8
{
	Building UMETA(DisplayName = "Building"),
	Resolving UMETA(DisplayName = "Resolving"),
	Combat UMETA(DisplayName = "Combat"),
	GameOver UMETA(DisplayName = "Game Over")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLGRGamePhaseChangedSignature, ELGRGamePhase, PreviousPhase, ELGRGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLGRDayChangedSignature, int32, NewDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLGRGameOverSignature, bool, bPlayerWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FLGRPopulationChangedSignature,
	int32,
	TotalPopulation,
	int32,
	AssignedPopulation,
	int32,
	AvailablePopulation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLGRBuildActionsChangedSignature,
	int32,
	RemainingBuildActions,
	int32,
	MaxBuildActions);

UCLASS()
class LORDGROWTHRULES_API ALGRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALGRGameModeBase();

	UFUNCTION(BlueprintCallable, Category = "LGR|Game Flow")
	bool RequestEndBuildingPhase();

	UFUNCTION(BlueprintCallable, Category = "LGR|Game Flow")
	bool CompleteDayResolution();

	UFUNCTION(BlueprintCallable, Category = "LGR|Game Flow")
	bool CompleteCombatPhase();

	UFUNCTION(BlueprintCallable, Category = "LGR|Game Flow")
	void EndGame(bool bVictory);

	UFUNCTION(BlueprintPure, Category = "LGR|Game Flow")
	int32 GetCurrentDay() const { return CurrentDay; }

	UFUNCTION(BlueprintPure, Category = "LGR|Game Flow")
	int32 GetDaysPerWave() const { return DaysPerWave; }

	UFUNCTION(BlueprintPure, Category = "LGR|Game Flow")
	ELGRGamePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "LGR|Settlement")
	ULGRBuildingSettlementComponent* GetBuildingSettlementComponent() const
	{
		return BuildingSettlementComponent;
	}

	UFUNCTION(BlueprintPure, Category = "LGR|Population")
	int32 GetTotalPopulation() const { return TotalPopulation; }

	UFUNCTION(BlueprintPure, Category = "LGR|Population")
	int32 GetAssignedPopulation() const { return AssignedPopulation; }

	UFUNCTION(BlueprintPure, Category = "LGR|Population")
	int32 GetAvailablePopulation() const;

	UFUNCTION(BlueprintPure, Category = "LGR|Victory")
	int32 GetVictoryPopulationTarget() const { return VictoryPopulationTarget; }

	UFUNCTION(BlueprintPure, Category = "LGR|Victory")
	bool HasReachedVictoryPopulation() const;

	UFUNCTION(BlueprintPure, Category = "LGR|Population")
	bool CanAffordPopulation(int32 PopulationCost) const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Population")
	bool TryAssignPopulation(int32 PopulationCost);

	UFUNCTION(BlueprintCallable, Category = "LGR|Population")
	bool RefundAssignedPopulation(int32 PopulationAmount);

	// Removes occupants of a destroyed building from both assigned and total population.
	UFUNCTION(BlueprintCallable, Category = "LGR|Population")
	bool LoseAssignedPopulation(int32 PopulationAmount);

	UFUNCTION(BlueprintCallable, Category = "LGR|Population")
	void AddPopulation(int32 PopulationAmount);

	UFUNCTION(BlueprintPure, Category = "LGR|Building Actions")
	int32 GetRemainingBuildActions() const { return RemainingBuildActions; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building Actions")
	int32 GetMaxBuildActionsThisDay() const { return MaxBuildActionsThisDay; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building Actions")
	bool HasRemainingBuildActions() const { return RemainingBuildActions > 0; }

	UFUNCTION(BlueprintCallable, Category = "LGR|Building Actions")
	bool TryConsumeBuildAction();

	void RefundBuildAction();

	UPROPERTY(BlueprintAssignable, Category = "LGR|Game Flow")
	FLGRGamePhaseChangedSignature OnGamePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Game Flow")
	FLGRDayChangedSignature OnDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Game Flow")
	FLGRGameOverSignature OnGameOver;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Population")
	FLGRPopulationChangedSignature OnPopulationChanged;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Building Actions")
	FLGRBuildActionsChangedSignature OnBuildActionsChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Settlement")
	TObjectPtr<ULGRBuildingSettlementComponent> BuildingSettlementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Game Flow", meta = (ClampMin = "1"))
	int32 DaysPerWave = 5;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Game Flow")
	int32 CurrentDay = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Game Flow")
	ELGRGamePhase CurrentPhase = ELGRGamePhase::Building;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Game Flow")
	bool bPlayerWon = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Population", meta = (ClampMin = "0"))
	int32 InitialPopulation = 20;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Victory", meta = (ClampMin = "1", UIMin = "1"))
	int32 VictoryPopulationTarget = 50;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Population")
	int32 TotalPopulation = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Population")
	int32 AssignedPopulation = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building Actions", meta = (ClampMin = "1"))
	int32 PopulationPerBuildAction = 10;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building Actions")
	int32 MaxBuildActionsThisDay = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building Actions")
	int32 RemainingBuildActions = 0;

private:
	void ResetBuildActionsForNewDay();
	void BroadcastBuildActionsChanged();
	void SetPhase(ELGRGamePhase NewPhase);
	void AdvanceToNextDay();
	void BroadcastPopulationChanged();
};

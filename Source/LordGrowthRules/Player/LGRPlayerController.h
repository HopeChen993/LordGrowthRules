#pragma once

#include "CoreMinimal.h"
#include "../Buildings/LGRBuildingTypes.h"
#include "GameFramework/PlayerController.h"
#include "LGRPlayerController.generated.h"

class ALGRGridHighlight;
class ALGRGridManager;
class ALGRBuildingBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLGRHoveredGridCellChangedSignature,
	FIntPoint,
	GridCoordinates,
	bool,
	bCanPlace);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLGRBuildingPlacedSignature,
	ALGRBuildingBase*,
	Building,
	FIntPoint,
	GridOrigin);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLGRGridCellSelectionChangedSignature,
	FIntPoint,
	GridCoordinates,
	bool,
	bIsSelected);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLGRPlacedBuildingSelectionChangedSignature,
	ALGRBuildingBase*,
	Building);

UCLASS()
class LORDGROWTHRULES_API ALGRPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALGRPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "LGR|Cursor")
	bool GetMouseGroundHit(FHitResult& OutHit) const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Cursor")
	void ConfigureCursorForGameplay();

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid")
	bool RefreshGridManager();

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	bool GetHoveredGridCell(FIntPoint& OutGridCoordinates, bool& bOutCanPlace) const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Selection")
	bool SelectHoveredGridCell();

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Selection")
	void ClearSelectedGridCell();

	UFUNCTION(BlueprintPure, Category = "LGR|Grid Selection")
	bool GetSelectedGridCell(FIntPoint& OutGridCoordinates) const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Building Selection")
	bool SelectPlacedBuilding(ALGRBuildingBase* Building);

	UFUNCTION(BlueprintCallable, Category = "LGR|Building Selection")
	void ClearPlacedBuildingSelection();

	UFUNCTION(BlueprintPure, Category = "LGR|Building Selection")
	ALGRBuildingBase* GetSelectedPlacedBuilding() const { return SelectedPlacedBuilding; }

	UFUNCTION(BlueprintCallable, Category = "LGR|Building Placement")
	void SelectBuildingClass(TSubclassOf<ALGRBuildingBase> BuildingClass);

	UFUNCTION(BlueprintCallable, Category = "LGR|Building Placement")
	void ClearBuildingSelection();

	UFUNCTION(BlueprintPure, Category = "LGR|Building Placement")
	bool CanPlaceSelectedBuilding() const;

	UFUNCTION(BlueprintPure, Category = "LGR|Building Placement")
	bool CanDeploySelectedBuilding() const;

	UFUNCTION(BlueprintPure, Category = "LGR|Building Placement")
	int32 GetSelectedBuildingPopulationCost() const;

	UFUNCTION(BlueprintPure, Category = "LGR|Building Placement")
	void GetSelectedBuildingPlacementStatus(
		bool& bOutCanDeploy,
		ELGRPlacementFailureReason& OutFailureReason,
		FText& OutFailureText) const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Building Placement")
	bool TryDeploySelectedBuilding();

	UFUNCTION(BlueprintCallable, Category = "LGR|Building Placement")
	bool TryPlaceSelectedBuilding();

	UPROPERTY(BlueprintAssignable, Category = "LGR|Grid")
	FLGRHoveredGridCellChangedSignature OnHoveredGridCellChanged;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Grid Selection")
	FLGRGridCellSelectionChangedSignature OnGridCellSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Building Placement")
	FLGRBuildingPlacedSignature OnBuildingPlaced;

	UPROPERTY(BlueprintAssignable, Category = "LGR|Building Selection")
	FLGRPlacedBuildingSelectionChangedSignature OnPlacedBuildingSelectionChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Cursor")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Cursor")
	bool bTraceComplexGround = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid")
	TSubclassOf<ALGRGridHighlight> GridHighlightClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid")
	float GridHighlightZOffset = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building Selection")
	float RangeHighlightZOffset = 18.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Grid")
	TObjectPtr<ALGRGridManager> GridManager;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Grid")
	TObjectPtr<ALGRGridHighlight> GridHighlight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building Selection")
	TObjectPtr<ALGRGridHighlight> PrimaryRangeHighlight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building Selection")
	TObjectPtr<ALGRGridHighlight> SecondaryRangeHighlight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building Selection")
	TObjectPtr<ALGRBuildingBase> SelectedPlacedBuilding;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building Placement")
	TSubclassOf<ALGRBuildingBase> DefaultBuildingClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building Placement")
	TSubclassOf<ALGRBuildingBase> SelectedBuildingClass;

private:
	void EnsureGridHighlight();
	void EnsureRangeHighlights();
	void UpdateGridHover();
	void ClearGridHover();
	void UpdatePlacementHighlight();
	void UpdateSelectedBuildingRanges();
	void HideRangeHighlights();
	ELGRPlacementFailureReason GetPlacementFailureReasonAt(const FIntPoint& GridCoordinates) const;
	FText GetPlacementFailureText(ELGRPlacementFailureReason FailureReason) const;
	bool TryPlaceBuildingAt(const FIntPoint& GridCoordinates, bool bClearSelectionOnSuccess);

	FIntPoint HoveredGridCoordinates = FIntPoint::ZeroValue;
	bool bHasHoveredGridCell = false;
	bool bHoveredCellCanPlace = false;
	FIntPoint SelectedGridCoordinates = FIntPoint::ZeroValue;
	bool bHasSelectedGridCell = false;
};

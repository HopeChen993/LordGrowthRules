#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LGRGridTypes.h"
#include "LGRGridManager.generated.h"

class AActor;
class ALGRBuildingBase;
class USceneComponent;

UCLASS(BlueprintType, Blueprintable)
class LORDGROWTHRULES_API ALGRGridManager : public AActor
{
	GENERATED_BODY()

public:
	ALGRGridManager();

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid")
	void InitializeGrid();

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid|Lord Manor")
	bool SpawnLordManorAtCenter();

	UFUNCTION(BlueprintPure, Category = "LGR|Grid|Lord Manor")
	ALGRBuildingBase* GetLordManor() const { return LordManor; }

	UFUNCTION(BlueprintPure, Category = "LGR|Grid|Lord Manor")
	FIntPoint GetLordManorGridOrigin() const { return LordManorGridOrigin; }

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	FIntPoint WorldToGrid(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	FVector GridToWorld(const FIntPoint& GridCoordinates, float ZOffset = 0.0f) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	FVector GridAreaToWorld(const FIntPoint& OriginCoordinates, const FIntPoint& FootprintSize, float ZOffset = 0.0f) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	bool TryWorldToGrid(const FVector& WorldLocation, FIntPoint& OutGridCoordinates) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	bool IsInBounds(const FIntPoint& GridCoordinates) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	bool IsCellBuildable(const FIntPoint& GridCoordinates) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	bool IsCellFree(const FIntPoint& GridCoordinates) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	bool TryGetCell(const FIntPoint& GridCoordinates, FLGRGridCell& OutCell) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	AActor* GetOccupyingActor(const FIntPoint& GridCoordinates) const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid")
	bool SetCellBuildable(const FIntPoint& GridCoordinates, bool bNewBuildable);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid")
	bool OccupyCell(const FIntPoint& GridCoordinates, AActor* OccupyingActor);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid")
	bool ReleaseCell(const FIntPoint& GridCoordinates, AActor* ExpectedOccupyingActor = nullptr);

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	bool CanOccupyArea(const FIntPoint& OriginCoordinates, const FIntPoint& FootprintSize) const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid")
	bool OccupyArea(const FIntPoint& OriginCoordinates, const FIntPoint& FootprintSize, AActor* OccupyingActor);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid")
	int32 ReleaseAllCellsOccupiedBy(AActor* OccupyingActor);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid|Debug")
	void DrawDebugGrid(float Duration = -1.0f) const;

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	int32 GetGridWidth() const { return GridWidth; }

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	int32 GetGridHeight() const { return GridHeight; }

	UFUNCTION(BlueprintPure, Category = "LGR|Grid")
	float GetCellSize() const { return CellSize; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Grid")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGR|Grid", meta = (ClampMin = "1", UIMin = "1"))
	int32 GridWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGR|Grid", meta = (ClampMin = "1", UIMin = "1"))
	int32 GridHeight = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGR|Grid", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float CellSize = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGR|Grid|Debug")
	bool bDrawDebugGridOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGR|Grid|Debug")
	FColor DebugGridColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGR|Grid|Debug", meta = (ClampMin = "0.0"))
	float DebugLineThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGR|Grid|Debug")
	float DebugLineZOffset = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid|Lord Manor")
	bool bSpawnLordManorOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid|Lord Manor")
	TSubclassOf<ALGRBuildingBase> LordManorClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Grid|Lord Manor")
	TObjectPtr<ALGRBuildingBase> LordManor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Grid|Lord Manor")
	FIntPoint LordManorGridOrigin = FIntPoint::ZeroValue;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "LGR|Grid")
	TArray<FLGRGridCell> Cells;

	int32 CoordinatesToIndex(const FIntPoint& GridCoordinates) const;
	bool IsValidFootprint(const FIntPoint& FootprintSize) const;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LGRGridHighlight.generated.h"

class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS(BlueprintType, Blueprintable)
class LORDGROWTHRULES_API ALGRGridHighlight : public AActor
{
	GENERATED_BODY()

public:
	ALGRGridHighlight();

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Highlight")
	void SetCellSize(float InCellSize);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Highlight")
	void SetFootprintSize(float InCellSize, FIntPoint InFootprintSize);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Highlight")
	void SetCircleRadius(float InCellSize, float RadiusInCells);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Highlight")
	void SetCellCoverage(float InCellCoverage);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Highlight")
	void ShowHighlight(const FVector& WorldLocation, bool bCanPlace);

	UFUNCTION(BlueprintCallable, Category = "LGR|Grid Highlight")
	void HideHighlight();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Grid Highlight")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Grid Highlight")
	TObjectPtr<UStaticMeshComponent> HighlightMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> PlaneMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CircleMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid Highlight")
	TObjectPtr<UMaterialInterface> AvailableMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid Highlight")
	TObjectPtr<UMaterialInterface> BlockedMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid Highlight", meta = (ClampMin = "1.0"))
	float SourceMeshSize = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid Highlight", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float CellCoverage = 0.94f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Grid Highlight", meta = (ClampMin = "0.1"))
	float CircleThickness = 4.0f;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGR|Grid Highlight")
	void OnPlacementStateChanged(bool bCanPlace);

private:
	bool bHasPlacementState = false;
	bool bLastCanPlace = false;
};

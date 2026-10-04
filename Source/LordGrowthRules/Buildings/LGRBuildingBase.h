#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LGRBuildingTypes.h"
#include "LGRBuildingBase.generated.h"

class ALGRGridManager;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLGRBuildingHealthChangedSignature,
	float,
	CurrentHealth,
	float,
	MaxHealth);

UCLASS(BlueprintType, Blueprintable)
class LORDGROWTHRULES_API ALGRBuildingBase : public AActor
{
	GENERATED_BODY()

public:
	ALGRBuildingBase();

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "LGR|Building")
	void InitializePlacedBuilding(ALGRGridManager* InGridManager, FIntPoint InGridOrigin);

	UFUNCTION(BlueprintPure, Category = "LGR|Building")
	FIntPoint GetFootprintSize() const;

	// Used by authoritative setup code for fixed buildings such as the 2x2 Lord Manor.
	void SetFootprintSizeForSystem(const FIntPoint& InFootprintSize);

	UFUNCTION(BlueprintPure, Category = "LGR|Building")
	FIntPoint GetGridOrigin() const { return GridOrigin; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building")
	bool IsPlacedOnGrid() const { return bPlacedOnGrid; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building")
	int32 GetPopulationCost() const { return PopulationCost; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building")
	ELGRBuildingType GetBuildingType() const { return BuildingType; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building|UI")
	FText GetBuildingDisplayName() const { return BuildingDisplayName; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building|UI")
	FText GetBuildingDescription() const { return BuildingDescription; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building|UI")
	UTexture2D* GetBuildingIcon() const { return BuildingIcon; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "LGR|Building|Health")
	float GetHealthNormalized() const;

	UFUNCTION(BlueprintCallable, Category = "LGR|Building|Health")
	float ApplyBuildingDamage(float DamageAmount, AActor* DamageCauser = nullptr);

	UPROPERTY(BlueprintAssignable, Category = "LGR|Building|Health")
	FLGRBuildingHealthChangedSignature OnHealthChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Building")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Building")
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building")
	ELGRBuildingType BuildingType = ELGRBuildingType::Residence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building")
	FIntPoint FootprintSize = FIntPoint(1, 1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building", meta = (ClampMin = "0"))
	int32 PopulationCost = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building|Health", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building|UI")
	FText BuildingDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building|UI", meta = (MultiLine = "true"))
	FText BuildingDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building|UI")
	TObjectPtr<UTexture2D> BuildingIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building|Prototype")
	bool bAutoScalePrototypeMesh = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building|Prototype", meta = (ClampMin = "1.0"))
	float PrototypeMeshSourceSize = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Building|Prototype", meta = (ClampMin = "1.0"))
	float PrototypeHeight = 180.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building")
	FIntPoint GridOrigin = FIntPoint::ZeroValue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building")
	TObjectPtr<ALGRGridManager> GridManager;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LGR|Building")
	bool bPlacedOnGrid = false;
};

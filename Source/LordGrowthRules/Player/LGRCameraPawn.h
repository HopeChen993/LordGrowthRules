#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LGRCameraPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class USceneComponent;
class USpringArmComponent;

UCLASS()
class LORDGROWTHRULES_API ALGRCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ALGRCameraPawn();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "LGR|Camera")
	void MoveCamera(FVector2D InputValue);

	UFUNCTION(BlueprintCallable, Category = "LGR|Camera")
	void ZoomCamera(float InputValue);

	UFUNCTION(BlueprintCallable, Category = "LGR|Camera")
	void RotateCamera(float InputValue);

	UFUNCTION(BlueprintCallable, Category = "LGR|Camera")
	void SetCameraZoom(float NewArmLength);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Camera")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGR|Camera")
	TObjectPtr<UFloatingPawnMovement> FloatingMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Camera", meta = (ClampMin = "100.0"))
	float MinimumZoom = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Camera", meta = (ClampMin = "100.0"))
	float MaximumZoom = 3200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Camera", meta = (ClampMin = "1.0"))
	float ZoomStep = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Camera", meta = (ClampMin = "0.1"))
	float ZoomInterpolationSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LGR|Camera", meta = (ClampMin = "1.0"))
	float RotationSpeed = 75.0f;

private:
	float DesiredArmLength = 2200.0f;
};

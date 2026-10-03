#include "LGRCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"

ALGRCameraPawn::ALGRCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->TargetArmLength = 2200.0f;
	SpringArm->SetRelativeRotation(FRotator(-55.0f, -45.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	FloatingMovement->MaxSpeed = 1600.0f;
	FloatingMovement->Acceleration = 6000.0f;
	FloatingMovement->Deceleration = 8000.0f;
}

void ALGRCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	DesiredArmLength = FMath::Clamp(SpringArm->TargetArmLength, MinimumZoom, MaximumZoom);
}

void ALGRCameraPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		DesiredArmLength,
		DeltaSeconds,
		ZoomInterpolationSpeed);
}

void ALGRCameraPawn::MoveCamera(const FVector2D InputValue)
{
	if (InputValue.IsNearlyZero())
	{
		return;
	}

	const FRotator CameraYawRotation(0.0f, SpringArm->GetComponentRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InputValue.Y);
	AddMovementInput(RightDirection, InputValue.X);
}

void ALGRCameraPawn::ZoomCamera(const float InputValue)
{
	SetCameraZoom(DesiredArmLength - InputValue * ZoomStep);
}

void ALGRCameraPawn::RotateCamera(const float InputValue)
{
	if (FMath::IsNearlyZero(InputValue))
	{
		return;
	}

	FRotator NewRotation = SpringArm->GetRelativeRotation();
	NewRotation.Yaw += InputValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
	SpringArm->SetRelativeRotation(NewRotation);
}

void ALGRCameraPawn::SetCameraZoom(const float NewArmLength)
{
	DesiredArmLength = FMath::Clamp(NewArmLength, MinimumZoom, MaximumZoom);
}

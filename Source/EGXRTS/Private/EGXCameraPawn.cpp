#include "EGXCameraPawn.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EGXPlanetActor.h"
#include "EGXPlanetTypes.h"
#include "EnhancedInputComponent.h"

class UInputAction;
class UInputMappingContext;
class UInputActionValue;

AEGXCameraPawn::AEGXCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootSceneComponent;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->TargetArmLength = 1800.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void AEGXCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	ZoomDistance = SpringArm->TargetArmLength;
}

void AEGXCameraPawn::InitializeForPlanet(AEGXPlanetActor* InPlanet, const FVector& SurfacePoint)
{
	ActivePlanet = InPlanet;
	FocusWorldLocation = SurfacePoint;

	if (ActivePlanet)
	{
		FocusSurfaceNormal = ActivePlanet->GetSurfaceNormalAt(SurfacePoint);
		FocusWorldLocation = ActivePlanet->ProjectPointToSurface(SurfacePoint, 0.f);
	}

	SetActorLocation(FocusWorldLocation);
	UpdateViewTransform(0.f);
}

void AEGXCameraPawn::SetFocusFromSurfaceHit(const FEGXSurfaceHit& SurfaceHit)
{
	if (!SurfaceHit.bHit)
	{
		return;
	}

	ActivePlanet = SurfaceHit.Planet;
	FocusWorldLocation = SurfaceHit.WorldLocation;
	FocusSurfaceNormal = SurfaceHit.SurfaceNormal;
}

void AEGXCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyPan(DeltaTime);

	if (!FMath::IsNearlyZero(PendingZoomInput))
	{
		ZoomDistance = FMath::Clamp(
			ZoomDistance - PendingZoomInput * ZoomSpeed,
			MinZoom,
			MaxZoom);
	}

	if (!FMath::IsNearlyZero(PendingRotateInput))
	{
		YawDegrees += PendingRotateInput * RotationSpeedDegrees * DeltaTime;
	}

	UpdateViewTransform(DeltaTime);

	PendingPanInput = FVector2D::ZeroVector;
	PendingZoomInput = 0.f;
	PendingRotateInput = 0.f;
}

void AEGXCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (CameraPanAction)
		{
			EIC->BindAction(CameraPanAction, ETriggerEvent::Triggered, this, &AEGXCameraPawn::Input_CameraPan);
		}

		if (CameraZoomAction)
		{
			EIC->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AEGXCameraPawn::Input_CameraZoom);
		}

		if (CameraRotateAction)
		{
			EIC->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this, &AEGXCameraPawn::Input_CameraRotate);
		}
	}
}

void AEGXCameraPawn::Input_CameraPan(const FInputActionValue& Value)
{
	PendingPanInput = Value.Get<FVector2D>();
}

void AEGXCameraPawn::Input_CameraZoom(const FInputActionValue& Value)
{
	PendingZoomInput = Value.Get<float>();
}

void AEGXCameraPawn::Input_CameraRotate(const FInputActionValue& Value)
{
	PendingRotateInput = Value.Get<float>();
}

FVector AEGXCameraPawn::GetCameraForwardOnTangentPlane() const
{
	const FVector Up = FocusSurfaceNormal.GetSafeNormal();

	const FQuat YawQuat(Up, FMath::DegreesToRadians(YawDegrees));

	// Pick an arbitrary tangent reference that is stable enough.
	FVector ReferenceForward = FVector::CrossProduct(FVector::UpVector, Up);
	if (ReferenceForward.IsNearlyZero())
	{
		ReferenceForward = FVector::CrossProduct(FVector::ForwardVector, Up);
	}
	ReferenceForward.Normalize();

	FVector Forward = YawQuat.RotateVector(ReferenceForward);
	Forward = FVector::VectorPlaneProject(Forward, Up).GetSafeNormal();
	return Forward;
}

FVector AEGXCameraPawn::GetCameraRightOnTangentPlane() const
{
	const FVector Up = FocusSurfaceNormal.GetSafeNormal();
	const FVector Forward = GetCameraForwardOnTangentPlane();
	return FVector::CrossProduct(Up, Forward).GetSafeNormal();
}

void AEGXCameraPawn::ApplyPan(float DeltaTime)
{
	if (!ActivePlanet || PendingPanInput.IsNearlyZero())
	{
		return;
	}

	const FVector Forward = GetCameraForwardOnTangentPlane();
	const FVector Right = GetCameraRightOnTangentPlane();

	const FVector DesiredMove =
		(Forward * PendingPanInput.Y) +
		(Right * PendingPanInput.X);

	const FVector Delta = DesiredMove.GetClampedToMaxSize(1.f) * PanSpeed * DeltaTime;
	const FVector NewFocus = FocusWorldLocation + Delta;

	FocusWorldLocation = ActivePlanet->ProjectPointToSurface(NewFocus, 0.f);
	FocusSurfaceNormal = ActivePlanet->GetSurfaceNormalAt(FocusWorldLocation);
}

void AEGXCameraPawn::UpdateViewTransform(float DeltaTime)
{
	if (!ActivePlanet)
	{
		// Fallback if no planet yet.
		SpringArm->TargetArmLength = ZoomDistance;
		SetActorLocation(FocusWorldLocation);
		SetActorRotation(FRotator(-PitchDegrees, YawDegrees, 0.f));
		return;
	}

	//const FVector Up = FocusSurfaceNormal.GetSafeNormal();
	const FVector Forward = GetCameraForwardOnTangentPlane();
	const FVector Right = GetCameraRightOnTangentPlane();

	//const FRotationMatrix BasisRot = FRotationMatrix::MakeFromXZ(Forward, Right).Rotator();
	const FQuat PitchQuat(Right, FMath::DegreesToRadians(-PitchDegrees));

	const FVector BackDirection = PitchQuat.RotateVector(-Forward).GetSafeNormal();
	const FVector DesiredCameraLocation = FocusWorldLocation - BackDirection * ZoomDistance;

	const FVector SmoothedLocation = FMath::VInterpTo(
		GetActorLocation(),
		DesiredCameraLocation,
		DeltaTime,
		ViewSmoothingSpeed);

	SetActorLocation(SmoothedLocation);

	const FRotator DesiredRotation = (FocusWorldLocation - SmoothedLocation).Rotation();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, ViewSmoothingSpeed));

	SpringArm->TargetArmLength = 0.f;
}



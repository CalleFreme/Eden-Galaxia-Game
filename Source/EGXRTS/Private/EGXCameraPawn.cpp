#include "EGXCameraPawn.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EGXPlanetActor.h"
#include "EGXPlanetTypes.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

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
	
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;
	
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetUsingAbsoluteRotation(false);
	SpringArm->SetAbsolute(false, false, false);
	
	SpringArm->TargetArmLength = 1800.f;
	SpringArm->SetRelativeRotation(FRotator(-PitchDegrees, 0.f, 0.f));
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->SetUsingAbsoluteRotation(false);
	Camera->SetAbsolute(false, false, false);
	
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	
	FocusWorldLocation = FVector(0.f, 0.f, 1000.f);
	FocusSurfaceNormal = FVector::UpVector;
	ViewForwardTangent = FVector::ForwardVector;
	ZoomDistance = 1800.f;
}

void AEGXCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning,
		TEXT("Camera flags - SpringArm AbsRot:%d UsePCR:%d Camera AbsRot:%d UsePCR:%d"),
		SpringArm->IsUsingAbsoluteRotation(),
		SpringArm->bUsePawnControlRotation,
		Camera->IsUsingAbsoluteRotation(),
		Camera->bUsePawnControlRotation);
	ZoomDistance = SpringArm->TargetArmLength;

	if (FocusWorldLocation.IsNearlyZero())
	{
		FocusWorldLocation = GetActorLocation();
	}

	if (FocusSurfaceNormal.IsNearlyZero())
	{
		FocusSurfaceNormal = FVector::UpVector;
	}

	RebuildViewForwardFromCurrentTransform();
	UpdateViewTransform(0.f);
}

FVector AEGXCameraPawn::GetLocalUpVector() const
{
	return ActivePlanet ? FocusSurfaceNormal.GetSafeNormal() : FVector::UpVector;;
}

void AEGXCameraPawn::ConstrainViewForwardToSurface()
{
	const FVector Up = GetLocalUpVector();

	ViewForwardTangent = FVector::VectorPlaneProject(ViewForwardTangent, Up).GetSafeNormal();

	if (ViewForwardTangent.IsNearlyZero())
	{
		ViewForwardTangent = FVector::VectorPlaneProject(FVector::ForwardVector, Up).GetSafeNormal();

		if (ViewForwardTangent.IsNearlyZero())
		{
			ViewForwardTangent = FVector::VectorPlaneProject(FVector::RightVector, Up).GetSafeNormal();
		}
	}
}

void AEGXCameraPawn::RebuildViewForwardFromCurrentTransform()
{
	const FVector Up = GetLocalUpVector();

	// Use what the camera is actually looking along, projected onto the current tangent plane.
	if (Camera)
	{
		ViewForwardTangent = FVector::VectorPlaneProject(Camera->GetForwardVector(), Up).GetSafeNormal();
	}

	if (ViewForwardTangent.IsNearlyZero())
	{
		ViewForwardTangent = FVector::VectorPlaneProject(GetActorForwardVector(), Up).GetSafeNormal();
	}

	ConstrainViewForwardToSurface();
}

static FVector MakeInitialSurfaceForward(const FVector& Up)
{
	FVector Forward = FVector::VectorPlaneProject(FVector::ForwardVector, Up).GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::VectorPlaneProject(FVector::RightVector, Up).GetSafeNormal();
	}
	return Forward;
}

void AEGXCameraPawn::InitializeForPlanet(AEGXPlanetActor* InPlanet, const FVector& SurfacePoint)
{
	ActivePlanet = InPlanet;
	//FocusWorldLocation = SurfacePoint;

	if (ActivePlanet)
	{
		FocusWorldLocation = ActivePlanet->ProjectPointToSurface(SurfacePoint, 0.f);
		FocusSurfaceNormal = ActivePlanet->GetSurfaceNormalAt(FocusWorldLocation);
		ViewForwardTangent = MakeInitialSurfaceForward(FocusSurfaceNormal);
	}
	else
	{
		FocusWorldLocation = SurfacePoint;
		FocusSurfaceNormal = FVector::UpVector;
		ViewForwardTangent = FVector::ForwardVector;
	}

	ConstrainViewForwardToSurface();
	UpdateViewTransform(0.f);
}

void AEGXCameraPawn::InitializeForFlat(const FVector& WorldFocus)
{
	ActivePlanet = nullptr;
	FocusWorldLocation = WorldFocus;
	FocusSurfaceNormal = FVector::UpVector;
	ViewForwardTangent = FVector::ForwardVector;
	ConstrainViewForwardToSurface();
	UpdateViewTransform(0.f);
}

void AEGXCameraPawn::SetFocusWorldLocation(const FVector& InFocusWorldLocation)
{
	FocusWorldLocation = InFocusWorldLocation;

	if (ActivePlanet)
	{
		FocusWorldLocation = ActivePlanet->ProjectPointToSurface(FocusWorldLocation, 0.f);
		FocusSurfaceNormal = ActivePlanet->GetSurfaceNormalAt(FocusWorldLocation);
	}
	else
	{
		FocusSurfaceNormal = FVector::UpVector;
	}

	ConstrainViewForwardToSurface();
}

void AEGXCameraPawn::ClearPlanetMode()
{
	ActivePlanet = nullptr;
	FocusSurfaceNormal = FVector::UpVector;
	ConstrainViewForwardToSurface();
}

void AEGXCameraPawn::SetFocusFromSurfaceHit(const FEGXSurfaceHit& SurfaceHit)
{
	if (!SurfaceHit.bHit)
	{
		return;
	}

	ActivePlanet = SurfaceHit.Planet;
	FocusWorldLocation = SurfaceHit.WorldLocation;
	FocusSurfaceNormal = SurfaceHit.SurfaceNormal.GetSafeNormal();

	if (!ActivePlanet)
	{
		FocusSurfaceNormal = FVector::UpVector;
	}

	ConstrainViewForwardToSurface();
}

void AEGXCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyRotate(DeltaTime);
	ApplyPan(DeltaTime);

	if (!FMath::IsNearlyZero(PendingZoomInput))
	{
		ZoomDistance = FMath::Clamp(
			ZoomDistance - PendingZoomInput * GetEffectiveZoomSpeed(),
			GetEffectiveMinZoom(),
			GetEffectiveMaxZoom());
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

	UE_LOG(LogTemp, Verbose, TEXT("CameraPawn Rotate Input=%f"), PendingRotateInput);
}

FVector AEGXCameraPawn::GetStablePlanetReferenceForward(const FVector& Up) const
{
	FVector ReferenceForward = FVector::CrossProduct(FVector::UpVector, Up);
	if (ReferenceForward.IsNearlyZero())
	{
		ReferenceForward = FVector::CrossProduct(FVector::ForwardVector, Up);
	}

	return ReferenceForward.GetSafeNormal();
}

FVector AEGXCameraPawn::GetCameraForwardOnTangentPlane() const
{
	return ViewForwardTangent.GetSafeNormal();
}

FVector AEGXCameraPawn::GetCameraRightOnTangentPlane() const
{
	const FVector Up = GetLocalUpVector();
	return FVector::CrossProduct(Up, GetCameraForwardOnTangentPlane()).GetSafeNormal();
}

void AEGXCameraPawn::ApplyRotate(float DeltaTime)
{
	if (FMath::IsNearlyZero(PendingRotateInput))
	{
		return;
	}

	const FVector Up = GetLocalUpVector();
	const float AngleDegrees = PendingRotateInput * RotationSpeedDegrees * DeltaTime;
	const FQuat Rot(Up, FMath::DegreesToRadians(AngleDegrees));

	ViewForwardTangent = Rot.RotateVector(ViewForwardTangent).GetSafeNormal();
	ConstrainViewForwardToSurface();
}

void AEGXCameraPawn::ApplyPan(float DeltaTime)
{
	if (PendingPanInput.IsNearlyZero())
	{
		return;
	}

	const FVector Forward = GetCameraForwardOnTangentPlane();
	const FVector Right = GetCameraRightOnTangentPlane();

	if (!ActivePlanet)
	{
		const FVector DesiredMove =
			(Forward * PendingPanInput.Y) +
			(Right * PendingPanInput.X);

		FocusWorldLocation += DesiredMove.GetClampedToMaxSize(1.f) * GetEffectivePanSpeed() * DeltaTime;
		FocusSurfaceNormal = FVector::UpVector;
		return;
	}

	const FVector PlanetCenter = ActivePlanet->GetActorLocation();
	const float Radius = FVector::Distance(FocusWorldLocation, PlanetCenter);
	if (Radius <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	FVector RadiusDir = (FocusWorldLocation - PlanetCenter).GetSafeNormal();

	const float EffectivePanSpeed = GetEffectivePanSpeed();
	const float ForwardDistance = PendingPanInput.Y * EffectivePanSpeed * DeltaTime;
	const float RightDistance = PendingPanInput.X * EffectivePanSpeed * DeltaTime;

	const float ForwardAngleRad = ForwardDistance / Radius;
	const float RightAngleRad = RightDistance / Radius;

	// W/S = along current screen forward/back
	const FQuat ForwardMoveRot(Right, ForwardAngleRad);

	// A/D = along current screen left/right
	const FQuat SideMoveRot(-Forward, RightAngleRad);

	const FQuat CombinedRot = SideMoveRot * ForwardMoveRot;
	RadiusDir = CombinedRot.RotateVector(RadiusDir).GetSafeNormal();

	FocusSurfaceNormal = RadiusDir;
	FocusWorldLocation = PlanetCenter + RadiusDir * Radius;

	ConstrainViewForwardToSurface();
}

void AEGXCameraPawn::UpdateViewTransform(float DeltaTime)
{
	ZoomDistance = FMath::Clamp(ZoomDistance, GetEffectiveMinZoom(), GetEffectiveMaxZoom());
	SpringArm->TargetArmLength = ZoomDistance;
	SpringArm->SetRelativeRotation(FRotator(-PitchDegrees, 0.f, 0.f));

	const FVector Up = GetLocalUpVector();
	const FVector DesiredFocusAnchor = ActivePlanet
	? FocusWorldLocation + Up * 50.f
	: FocusWorldLocation;
	const FVector Forward = GetCameraForwardOnTangentPlane();
	const FRotator DesiredRotation = FRotationMatrix::MakeFromXZ(Forward, Up).Rotator();

	const FVector SmoothedLocation = FMath::VInterpTo(
		GetActorLocation(),
		DesiredFocusAnchor,
		DeltaTime,
		ViewSmoothingSpeed);

	const FRotator SmoothedRotation = FMath::RInterpTo(
		GetActorRotation(),
		DesiredRotation,
		DeltaTime,
		ViewSmoothingSpeed);

	SetActorLocation(SmoothedLocation);
	SetActorRotation(SmoothedRotation);
}

float AEGXCameraPawn::GetEffectiveMinZoom() const
{
	if (!bScaleZoomAndPanToPlanet || !ActivePlanet)
	{
		return MinZoom;
	}

	return FMath::Max(MinZoom, ActivePlanet->PlanetRadius * PlanetMinZoomRadiusRatio);
}

float AEGXCameraPawn::GetEffectiveMaxZoom() const
{
	if (!bScaleZoomAndPanToPlanet || !ActivePlanet)
	{
		return MaxZoom;
	}

	return FMath::Max(MaxZoom, ActivePlanet->PlanetRadius * PlanetMaxZoomRadiusRatio);
}

float AEGXCameraPawn::GetEffectiveZoomSpeed() const
{
	if (!bScaleZoomAndPanToPlanet || !ActivePlanet)
	{
		return ZoomSpeed;
	}

	return FMath::Max(ZoomSpeed, ActivePlanet->PlanetRadius * 0.08f);
}

float AEGXCameraPawn::GetEffectivePanSpeed() const
{
	if (!bScaleZoomAndPanToPlanet || !ActivePlanet)
	{
		return PanSpeed;
	}

	const float EffectiveMinZoom = GetEffectiveMinZoom();
	const float EffectiveMaxZoom = GetEffectiveMaxZoom();
	const float ZoomAlpha = FMath::Clamp((ZoomDistance - EffectiveMinZoom) / FMath::Max(EffectiveMaxZoom - EffectiveMinZoom, 1.f), 0.f, 1.f);
	const float ZoomPan = PanSpeed * FMath::Lerp(1.f, FullyZoomedOutPanMultiplier, FMath::Pow(ZoomAlpha, 1.35f));
	const float StrategicPan = (2.f * PI * ActivePlanet->PlanetRadius) / FMath::Max(StrategicPanPlanetCircumferenceSeconds, 1.f);
	return FMath::Lerp(ZoomPan, StrategicPan, FMath::Pow(ZoomAlpha, 2.25f));
}



#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EGXTypes.h"
#include "EGXCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class AEGXPlanetActor;

UCLASS()
class EGXRTS_API AEGXCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AEGXCameraPawn();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category="EGX Camera")
	void InitializeForPlanet(AEGXPlanetActor* InPlanet, const FVector& SurfacePoint);

	UFUNCTION(BlueprintCallable, Category="EGX Camera")
	void SetFocusFromSurfaceHit(const FEGXSurfaceHit& SurfaceHit);

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Input")
	TObjectPtr<UInputAction> MoveCameraAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Input")
	TObjectPtr<UInputAction> ZoomCameraAction;

	void Input_CameraPan(const FInputActionValue& Value);
	void Input_CameraZoom(const FInputActionValue& Value);
	void Input_CameraRotate(const FInputActionValue& Value);

	void ApplyPan(float DeltaTime);
	void UpdateViewTransform(float DeltaTime);

	FVector GetCameraForwardOnTangentPlane() const;
	FVector GetCameraRightOnTangentPlane() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> CameraPanAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> CameraZoomAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> CameraRotateAction;
	
	FVector PendingMovementInput = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float PanSpeed = 3500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float ZoomSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float MinZoom = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float MaxZoom = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float PitchDegrees = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float ViewSmoothingSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float RotationSpeedDegrees = 90.f;

	UPROPERTY(BlueprintReadOnly, Category="Camera")
	TObjectPtr<AEGXPlanetActor> ActivePlanet = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Camera")
	FVector FocusWorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Camera")
	FVector FocusSurfaceNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category="Camera")
	float ZoomDistance = 1800.f;

	UPROPERTY(BlueprintReadOnly, Category="Camera")
	float YawDegrees = 0.f;

	FVector2D PendingPanInput = FVector2D::ZeroVector;
	float PendingZoomInput = 0.f;
	float PendingRotateInput = 0.f;
};

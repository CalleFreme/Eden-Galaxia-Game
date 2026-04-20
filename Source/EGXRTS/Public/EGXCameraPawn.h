#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EGXTypes.h"
#include "EGXCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputActionValue;
class AEGXPlanetActor;
struct FEGXSurfaceHit;

UCLASS()
class EGXRTS_API AEGXCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AEGXCameraPawn();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category="EGX Camera")
	void InitializeForPlanet(AEGXPlanetActor* InPlanet, const FVector& SurfacePoint);
	
	UFUNCTION(BlueprintCallable, Category="EGX Camera")
	void InitializeForFlat(const FVector & InFocusWorldLocation);	

	UFUNCTION(BlueprintCallable, Category="EGX Camera")
	void SetFocusWorldLocation(const FVector& InFocusWorldLocation);
	
	UFUNCTION(BlueprintPure, Category="EGX Camera")
	AEGXPlanetActor* GetActivePlanet() const { return ActivePlanet; }
	
	UFUNCTION(BlueprintPure, Category="EGX Camera")
	FVector GetFocusWorldLocation() const { return FocusWorldLocation; }

	UFUNCTION(BlueprintPure, Category="EGX Camera")
	FVector GetFocusSurfaceNormal() const { return FocusSurfaceNormal; }
	
	UFUNCTION(BlueprintCallable, Category="EGX Camera")
	void ClearPlanetMode();
	
	UFUNCTION(BlueprintCallable, Category="EGX Camera")
	void SetFocusFromSurfaceHit(const FEGXSurfaceHit& SurfaceHit);

	void Input_CameraPan(const FInputActionValue& Value);
	void Input_CameraZoom(const FInputActionValue& Value);
	void Input_CameraRotate(const FInputActionValue& Value);
protected:
	FVector GetStablePlanetReferenceForward(const FVector& Up) const;

	FVector GetLocalUpVector() const;
	void RebuildViewForwardFromCurrentTransform();
	void ConstrainViewForwardToSurface();
	void ApplyRotate(float DeltaTime);
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX Camera")
	float PanSpeed = 3500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX Camera")
	float ZoomSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX Camera")
	float MinZoom = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX Camera")
	float MaxZoom = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX Camera")
	float PitchDegrees = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX Camera")
	float ViewSmoothingSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX Camera")
	float RotationSpeedDegrees = 90.f;

	UPROPERTY(BlueprintReadOnly, Category="EGX Camera")
	TObjectPtr<AEGXPlanetActor> ActivePlanet = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="EGX Camera")
	FVector FocusWorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="EGX Camera")
	FVector FocusSurfaceNormal = FVector::UpVector;
		
	UPROPERTY(VisibleInstanceOnly, Category="EGX Camera")
	FVector ViewForwardTangent = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category="EGX Camera")
	float ZoomDistance = 1800.f;

	FVector2D PendingPanInput = FVector2D::ZeroVector;
	float PendingZoomInput = 0.f;
	float PendingRotateInput = 0.f;
};

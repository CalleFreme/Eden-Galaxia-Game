// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EGXTypes.h"
#include "EGXUnitBase.generated.h"

class UDecalComponent;
class UCapsuleComponent;
struct FEGXOrder;

UCLASS()
class EGXAI_API AEGXUnitBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEGXUnitBase();
	
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="EGX Unit")
	virtual void IssueOrder(const FEGXOrder& Order);

	UFUNCTION(BlueprintCallable, Category="EGX Unit")
	virtual void SetSelectedLocal(bool bSelected);

	UFUNCTION(BlueprintCallable, Category="EGX Unit")
	virtual bool IsOwnedBy(const APlayerController* PC) const;
	
	UFUNCTION(BlueprintCallable)
	void SetActivePlanet(AEGXPlanetActor* InPlanet);

	UFUNCTION(BlueprintCallable)
	void SnapToPlanetSurface(bool bAlignRotation = true);
	
protected:
	virtual void BeginPlay() override;
	
	AEGXPlanetActor* FindNearestPlanet() const;
	
	void SetPlanetMoveTarget(AEGXPlanetActor* InPlanet, const FVector& InTargetWorld);
	void TickPlanetMovement(float DeltaSeconds);
	float GetSurfaceClearance() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
	TObjectPtr<UDecalComponent> SelectionDecal = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Selection")
	bool bUseCustomDepthOutlineWhenSelected = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Selection")
	bool bHideSelectionDecalWhenNotSelected = true;
	
	UPROPERTY(VisibleInstanceOnly, Category="Selection")
	bool bSelectedLocal = false;
	
	UPROPERTY(VisibleInstanceOnly, Category="Movement|Planet")
	FVector MoveTargetSurfaceNormal = FVector::UpVector;
	
	UPROPERTY(VisibleInstanceOnly, Category="Movement|Planet")
	TObjectPtr<AEGXPlanetActor> ActivePlanet = nullptr;

	UPROPERTY(VisibleInstanceOnly, Category="Movement|Planet")
	FVector MoveTargetWorld = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, Category="Movement|Planet")
	bool bHasPlanetMoveTarget = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Planet")
	float PlanetMoveSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Planet")
	float SurfaceClearanceOverride = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Planet")
	float ArrivalDistance = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Planet")
	float RotationInterpSpeed = 8.f;
};

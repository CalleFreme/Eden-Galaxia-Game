// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGXBuildingBase.generated.h"

class UBoxComponent;
class UDecalComponent;
class UStaticMeshComponent;
class AEGXPlanetActor;

UENUM(BlueprintType)
enum class EEGXBuildingState : uint8
{
	Placed			 UMETA(DisplayName="Placed"),
	UnderConstruction UMETA(DisplayName="Under Construction"),
	Operational		 UMETA(DisplayName="Operational"),
	Destroyed		 UMETA(DisplayName="Destroyed")
};
UCLASS(Abstract)
class EGXECONOMY_API AEGXBuildingBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEGXBuildingBase();

	virtual void Tick(float DeltaSeconds) override;

	// -------------------------
	// Selection
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	virtual void SetSelectedLocal(bool bSelected);

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	bool IsSelectedLocal() const
	{
		return bSelectedLocal;
	}

	// -------------------------
	// Ownership
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	void SetOwningPlayerId(int32 InPlayerId);

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	int32 GetOwningPlayerId() const
	{
		return OwningPlayerId;
	}

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	void SetTeamId(int32 InTeamId);

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	int32 GetTeamId() const
	{
		return TeamId;
	}

	// -------------------------
	// Construction
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	virtual void StartConstruction(float InStartingProgress = 0.f);

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	virtual void AddConstructionProgress(float DeltaProgress);

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	virtual void FinishConstruction();

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	bool IsUnderConstruction() const
	{
		return BuildingState == EEGXBuildingState::UnderConstruction;
	}

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	bool IsOperational() const
	{
		return BuildingState == EEGXBuildingState::Operational;
	}

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	float GetConstructionProgress() const
	{
		return ConstructionProgress;
	}

	// -------------------------
	// Health / Damage
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	virtual void ApplyDamageToBuilding(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category="EGX|Building")
	virtual void RepairBuilding(float RepairAmount);

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	float GetHealth() const
	{
		return CurrentHealth;
	}

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	float GetMaxHealth() const
	{
		return MaxHealth;
	}

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	bool IsDestroyed() const
	{
		return BuildingState == EEGXBuildingState::Destroyed;
	}

	// -------------------------
	// Placement / footprint
	// -------------------------

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	FVector2D GetFootprintSize() const
	{
		return FootprintSize;
	}

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	float GetPlacementRadius() const
	{
		return PlacementRadius;
	}

	UFUNCTION(BlueprintPure, Category="EGX|Building")
	EEGXBuildingState GetBuildingState() const
	{
		return BuildingState;
	}

protected:
	virtual void BeginPlay() override;
	AEGXPlanetActor* FindNearestPlanet() const;
	
	UFUNCTION(BlueprintImplementableEvent, Category="EGX|Building")
	void BP_OnSelectionChanged(bool bNowSelected);

	UFUNCTION(BlueprintImplementableEvent, Category="EGX|Building")
	void BP_OnConstructionStarted();

	UFUNCTION(BlueprintImplementableEvent, Category="EGX|Building")
	void BP_OnConstructionFinished();

	UFUNCTION(BlueprintImplementableEvent, Category="EGX|Building")
	void BP_OnDestroyed();

	UFUNCTION(BlueprintImplementableEvent, Category="EGX|Building")
	void BP_OnHealthChanged(float NewHealth, float NewMaxHealth);

	UFUNCTION(BlueprintNativeEvent, Category="EGX|Building")
	void UpdateVisualState();
	virtual void UpdateVisualState_Implementation();

	virtual void HandleDestroyed();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Components")
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Components")
	TObjectPtr<UBoxComponent> PlacementBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Components")
	TObjectPtr<UDecalComponent> SelectionDecal;

	// -------------------------
	// State
	// -------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building")
	EEGXBuildingState BuildingState = EEGXBuildingState::Placed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Building")
	bool bSelectedLocal = false;

	// May be replaced with a faction/player-state reference
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building")
	int32 OwningPlayerId = INDEX_NONE;

	// May be replaced with a faction/player-state reference
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building")
	int32 TeamId = INDEX_NONE;

	// -------------------------
	// Health
	// -------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building|Health", meta=(ClampMin="1.0"))
	float MaxHealth = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Building|Health")
	float CurrentHealth = 1000.f;

	// -------------------------
	// Construction
	// -------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building|Construction", meta=(ClampMin="0.1"))
	float ConstructionTimeSeconds = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Building|Construction", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ConstructionProgress = 1.f;

	// -------------------------
	// Placement
	// -------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building|Placement")
	FVector2D FootprintSize = FVector2D(400.f, 400.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building|Placement", meta=(ClampMin="0.0"))
	float PlacementRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building|Placement", meta=(ClampMin="0.0"))
	float MaxAllowedSurfaceAngleDegrees = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Building|Visual")
	bool bHideSelectionDecalWhenNotSelected = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="EGX|Building|Placement")
	float SurfaceClearance = 0.f;

	UFUNCTION(BlueprintCallable, Category="EGX|Building|Placement")
	void SnapToPlanetSurface(AEGXPlanetActor* Planet, const FVector& NearWorldPoint, float YawDegrees = 0.f);
	
	// TO DO: Add a buildability query
	// UFUNCTION(BlueprintCallable, Category="EGX|Building|Placement")
	// virtual bool CanBePlacedAtLocation(const FVector& Location, const FVector& SurfaceNormal) const;
};

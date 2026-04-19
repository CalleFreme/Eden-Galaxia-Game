#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGXPlanetActor.generated.h"

class UProceduralMeshComponent;

UCLASS()
class EGXPLANET_API AEGXPlanetActor : public AActor
{
	GENERATED_BODY()

public:
	AEGXPlanetActor();

	UFUNCTION(CallInEditor, BlueprintCallable)
	void GeneratePlanet();

	UFUNCTION(BlueprintCallable)
	FVector GetSurfacePointFromNormal(const FVector& UnitNormal, float HeightOffset = 0.0f) const;

	UFUNCTION(BlueprintCallable)
	FRotator GetSurfaceRotationFromNormal(const FVector& UnitNormal) const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> PlanetMesh;

	UPROPERTY(EditAnywhere, Category="Planet")
	float PlanetRadius = 50000.0f;

	UPROPERTY(EditAnywhere, Category="Planet")
	int32 Resolution = 16;
};

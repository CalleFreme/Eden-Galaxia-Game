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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet")
	FVector PlanetCenter = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet")
	float PlanetRadius = 10000.0f;

	UPROPERTY(EditAnywhere, Category="Planet")
	int32 Resolution = 16;
	
	UFUNCTION(BlueprintCallable, Category="Planet")
	FVector GetSurfaceNormalAt(const FVector& WorldPoint) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	FVector ProjectPointToSurface(const FVector& WorldPoint, float HeightOffset = 0.f) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	bool RaycastToPlanet(const FVector& RayOrigin, const FVector& RayDirection, FVector& OutPoint, FVector& OutNormal) const;
	
	UFUNCTION(CallInEditor, BlueprintCallable)
	void GeneratePlanet();

	UFUNCTION(BlueprintCallable)
	FVector GetSurfacePointFromNormal(const FVector& UnitNormal, float HeightOffset = 0.0f) const;

	UFUNCTION(BlueprintCallable)
	FRotator GetSurfaceRotationFromNormal(const FVector& UnitNormal) const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRootComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> PlanetMesh;


};

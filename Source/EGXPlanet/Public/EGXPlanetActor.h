#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGXPlanetActor.generated.h"

class UProceduralMeshComponent;
class USceneComponent;
class UMaterialInterface;

UCLASS()
class EGXPLANET_API AEGXPlanetActor : public AActor
{
	GENERATED_BODY()

public:
	AEGXPlanetActor();
	
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Planet")
	TObjectPtr<USceneComponent> SceneRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Planet")
	TObjectPtr<UProceduralMeshComponent> PlanetMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet")
	TObjectPtr<UMaterialInterface> PlanetMaterial = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet")
	FVector PlanetCenter = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet")
	float PlanetRadius = 5000.0f;

	UPROPERTY(EditAnywhere, Category="Planet")
	int32 Resolution = 24;
		
	UFUNCTION(CallInEditor, BlueprintCallable)
	void GeneratePlanet();
	
	UFUNCTION(BlueprintCallable, Category="Planet")
	FVector GetSurfaceNormalAt(const FVector& WorldPoint) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	FVector ProjectPointToSurface(const FVector& WorldPoint, float HeightOffset = 0.f) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	bool RaycastToPlanet(const FVector& RayOrigin, const FVector& RayDirection, FVector& OutPoint, FVector& OutNormal) const;

	UFUNCTION(BlueprintCallable)
	FVector GetSurfacePointFromNormal(const FVector& UnitNormal, float HeightOffset = 0.0f) const;

	UFUNCTION(BlueprintCallable)
	FRotator GetSurfaceRotationFromNormal(const FVector& UnitNormal) const;

protected:


};

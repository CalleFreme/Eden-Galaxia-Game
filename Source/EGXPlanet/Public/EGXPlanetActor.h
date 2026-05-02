#pragma once

#include "CoreMinimal.h"
#include "EGXPlanetTypes.h"
#include "GameFramework/Actor.h"
#include "EGXPlanetActor.generated.h"

class UMaterialInterface;
class UProceduralMeshComponent;
class USceneComponent;

UCLASS()
class EGXPLANET_API AEGXPlanetActor : public AActor
{
	GENERATED_BODY()

public:
	AEGXPlanetActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet", meta=(ClampMin="100.0"))
	float PlanetRadius = 50000.0f;

	UPROPERTY(EditAnywhere, Category="Planet|Mesh", meta=(ClampMin="2", ClampMax="64"))
	int32 Resolution = 32;

	UPROPERTY(EditAnywhere, Category="Planet|Mesh", meta=(ClampMin="1", ClampMax="32"))
	int32 FacePatchCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD")
	bool bEnableRuntimeLOD = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="2", ClampMax="96"))
	int32 LOD0Resolution = 28;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="2", ClampMax="64"))
	int32 LOD1Resolution = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="2", ClampMax="32"))
	int32 LOD2Resolution = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="2", ClampMax="16"))
	int32 LOD3Resolution = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="0.01"))
	float LOD0DistanceRatio = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="0.01"))
	float LOD1DistanceRatio = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="0.01"))
	float LOD2DistanceRatio = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="0.05"))
	float LODUpdateIntervalSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|LOD", meta=(ClampMin="0.0"))
	float LODCameraMoveThreshold = 8000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|Mesh")
	bool bGenerateTerrainCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|Rendering")
	bool bCastTerrainShadow = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|Material")
	EEGXPlanetVertexColorMode VertexColorMode = EEGXPlanetVertexColorMode::MaterialMasks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|Generation")
	FEGXPlanetGenerationSettings GenerationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|Mutable", meta=(ClampMin="8", ClampMax="4096"))
	int32 MutableCellResolution = 256;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Planet|Mutable", meta=(ClampMin="0.0"))
	float MaxExpectedDeformation = 900.f;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Planet")
	void GeneratePlanet();

	UFUNCTION(BlueprintCallable, Category="Planet|Sampling")
	FEGXPlanetSurfaceSample SampleSurfaceFromNormal(const FVector& UnitNormal) const;

	UFUNCTION(BlueprintCallable, Category="Planet|Sampling")
	FEGXPlanetSurfaceSample SampleSurfaceAtWorldPoint(const FVector& WorldPoint) const;

	UFUNCTION(BlueprintCallable, Category="Planet|Sampling")
	float GetResourceDensityAt(const FVector& UnitNormal, EEGXResourceType ResourceType) const;

	UFUNCTION(BlueprintCallable, Category="Planet|Mutable")
	void ApplyRadialHeightEdit(const FVector& WorldCenter, float Radius, float HeightDelta);

	UFUNCTION(BlueprintCallable, Category="Planet|Mutable")
	void ApplyRadialEcologyEdit(const FVector& WorldCenter, float Radius, float EcologyDelta);

	UFUNCTION(BlueprintCallable, Category="Planet|Mutable")
	void ClearMutableTerrain();

	UFUNCTION(BlueprintCallable, Category="Planet")
	FVector GetSurfaceNormalAt(const FVector& WorldPoint) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	FVector ProjectPointToSurface(const FVector& WorldPoint, float HeightOffset = 0.f) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	bool RaycastToPlanet(const FVector& RayOrigin, const FVector& RayDirection, FVector& OutPoint, FVector& OutNormal) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	FVector GetSurfacePointFromNormal(const FVector& UnitNormal, float HeightOffset = 0.0f) const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	FRotator GetSurfaceRotationFromNormal(const FVector& UnitNormal) const;

protected:
	UPROPERTY()
	TMap<int64, FEGXPlanetMutableCell> MutableCells;

	float SampleBaseHeight(const FVector& UnitNormal) const;
	float SampleDeformationHeight(const FVector& UnitNormal) const;
	float SampleEcology(const FVector& UnitNormal) const;
	float SampleWaterDepth(const FVector& UnitNormal, float FinalHeight) const;
	float SampleTemperature(const FVector& UnitNormal, float FinalHeight) const;
	float SampleMoisture(const FVector& UnitNormal, float FinalHeight, float WaterDepth) const;
	float SampleNoise01(const FVector& UnitNormal, float Scale, float SeedOffset) const;
	float SampleRidgedNoise01(const FVector& UnitNormal, float Scale, float SeedOffset) const;
	float SmoothTerrainByErosion(float Height, float ContinentMask, float MountainMask) const;
	float EstimateSlopeDegrees(const FVector& UnitNormal, float FinalHeight) const;
	FVector EstimateSurfaceNormal(const FVector& UnitNormal) const;
	EEGXPlanetBiome ResolveBiome(const FVector& UnitNormal, float FinalHeight, float WaterDepth, float Ecology, float Temperature, float Moisture) const;
	FColor MakeVertexColor(const FEGXPlanetSurfaceSample& Sample) const;
	FColor MakeDebugBiomeColor(const FEGXPlanetSurfaceSample& Sample) const;
	FColor MakeMaterialMaskColor(const FEGXPlanetSurfaceSample& Sample) const;
	void ApplyPlanetMaterialParameters();

	int64 MakeMutableCellKey(int32 FaceIndex, int32 X, int32 Y) const;
	int64 MakeMutableCellKeyFromNormal(const FVector& UnitNormal) const;
	void NormalToFaceUV(const FVector& UnitNormal, int32& OutFaceIndex, FVector2D& OutUV) const;
	FVector FaceUVToNormal(int32 FaceIndex, const FVector2D& UV) const;
	int32 GetPatchResolutionForLOD(const FVector& PatchCenterNormal, const FVector& ViewLocation) const;
	FVector GetLODViewLocation() const;
	void RebuildPlanetMesh(bool bForceFullRebuild = true);

	float TimeSinceLastLODUpdate = 0.f;
	FVector LastLODViewLocation = FVector(FLT_MAX, FLT_MAX, FLT_MAX);
	TArray<int32> LastPatchResolutions;
};

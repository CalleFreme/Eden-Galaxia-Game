#pragma once

#include "CoreMinimal.h"
#include "EGXTypes.h"
#include "EGXPlanetTypes.generated.h"

class AActor;
class AEGXPlanetActor;

USTRUCT(BlueprintType)
struct EGXPLANET_API FEGXSurfaceHit
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bHit = false;

	UPROPERTY(BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector SurfaceNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AEGXPlanetActor> Planet = nullptr;
};

UENUM(BlueprintType)
enum class EEGXPlanetBiome : uint8
{
	Barren,
	Tundra,
	Temperate,
	Desert,
	Wetlands,
	Mountain,
	Ice
};

UENUM(BlueprintType)
enum class EEGXPlanetVertexColorMode : uint8
{
	DebugBiome,
	MaterialMasks
};

USTRUCT(BlueprintType)
struct EGXPLANET_API FEGXPlanetGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Seed")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Continents", meta=(ClampMin="0.000001"))
	float ContinentScale = 0.000035f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Continents", meta=(ClampMin="0.1"))
	float ContinentHeight = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Continents", meta=(ClampMin="0.1", ClampMax="5.0"))
	float ContinentSharpness = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Continents", meta=(ClampMin="-1.0", ClampMax="1.0"))
	float LandBias = -0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mountains", meta=(ClampMin="0.000001"))
	float MountainScale = 0.00011f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mountains", meta=(ClampMin="0.0"))
	float MountainHeight = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mountains", meta=(ClampMin="0.1", ClampMax="8.0"))
	float MountainSharpness = 2.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mountains", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MountainCoverage = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain Detail", meta=(ClampMin="0.000001"))
	float HillScale = 0.00042f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain Detail", meta=(ClampMin="0.0"))
	float HillHeight = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain Detail", meta=(ClampMin="0.000001"))
	float RoughnessScale = 0.00125f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain Detail", meta=(ClampMin="0.0"))
	float RoughnessHeight = 28.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain Shape", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ErosionStrength = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain Shape", meta=(ClampMin="0.0", ClampMax="1.0"))
	float PlainsStrength = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain Shape", meta=(ClampMin="0.0"))
	float MaxTerrainHeight = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water")
	bool bHasWater = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water")
	float SeaLevel = -120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Climate", meta=(ClampMin="-1.0", ClampMax="1.0"))
	float TemperatureBias = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Climate", meta=(ClampMin="0.0", ClampMax="1.0"))
	float PolarFalloff = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Climate", meta=(ClampMin="0.000001"))
	float MoistureScale = 0.00008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Climate", meta=(ClampMin="-1.0", ClampMax="1.0"))
	float MoistureBias = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ecology", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BaseEcology = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resources", meta=(ClampMin="0.000001"))
	float OreScale = 0.00032f;
};

USTRUCT(BlueprintType)
struct EGXPLANET_API FEGXPlanetSurfaceSample
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FEGXPlanetCoord Coord;

	UPROPERTY(BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector SurfaceNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly)
	float BaseHeight = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float DeformationHeight = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float FinalHeight = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float SlopeDegrees = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float WaterDepth = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Ecology = 1.f;

	UPROPERTY(BlueprintReadOnly)
	float Temperature = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Moisture = 0.f;

	UPROPERTY(BlueprintReadOnly)
	EEGXPlanetBiome Biome = EEGXPlanetBiome::Barren;

	UPROPERTY(BlueprintReadOnly)
	float OreDensity = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float IceDensity = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float BiomassDensity = 0.f;
};

USTRUCT()
struct EGXPLANET_API FEGXPlanetMutableCell
{
	GENERATED_BODY()

	UPROPERTY()
	float HeightDelta = 0.f;

	UPROPERTY()
	float EcologyDelta = 0.f;
};

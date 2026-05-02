#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EGXTypes.generated.h"

class AActor;
class AEGXPlanetActor;
class AEGXBuildingBase;

UENUM(BlueprintType)
enum class EEGXResourceType : uint8
{
	None,
	Ore,
	WaterIce UMETA(DisplayName="Water Ice"),
	Ice UMETA(Hidden, DisplayName="Deprecated Ice"),
	Biomass,
	Alloys,
	Fuel,
	Energy
};

UENUM(BlueprintType)
enum class EEGXOreType : uint8
{
	None,
	Gold,
	Titanium,
	Iron,
	Lead,
	Copper,
	Tin,
	Silver,
	Uradium
};

UENUM(BlueprintType)
enum class EEGXEnergyType : uint8
{
	None,
	Water,
	Hydrogen,
	NuclearFission,
	NuclearFusion,
	Wind,
	FossilFuel,
	GeoThermal,
	Incineration
};

USTRUCT(BlueprintType)
struct FEGXResourceAmount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEGXResourceType Type = EEGXResourceType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct FEGXPlanetCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Normal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Altitude = 0.0f;
};

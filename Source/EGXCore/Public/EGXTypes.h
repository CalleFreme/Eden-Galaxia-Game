#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GXETypes.generated.h"

UENUM(BlueprintType)
enum class EEGXResourceType : uint8
{
	None,
	Ore,
	Ice,
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

#pragma once

#include "CoreMinimal.h"
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
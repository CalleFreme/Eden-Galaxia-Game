#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/SoftObjectPtr.h"
#include "EGXBuildDefinition.generated.h"

class AActor;

UCLASS(BlueprintType)
class EGXGAMEPLAY_API UEGXBuildDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Build")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Build")
	float BuildTimeSeconds = 5.f;

	// Soft class pointer keeps the reference data-driven and load-friendly.
	// Could later be replaced with a shared abstract base, such as AEGXConstructibleBase
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Build")
	TSoftClassPtr<AActor> ResultActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Build")
	FVector2D Footprint = FVector2D(1.f, 1.f);
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGXPlayerStart.generated.h"

class UArrowComponent;
class AEGXPlanetActor;

UCLASS()
class EGXRTS_API AEGXPlayerStart : public AActor
{
	GENERATED_BODY()

public:
	AEGXPlayerStart();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Player Start")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EGX|Player Start")
	TObjectPtr<UArrowComponent> Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Player Start")
	int32 PlayerId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Player Start")
	TObjectPtr<AEGXPlanetActor> PlanetOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Player Start", meta=(ClampMin="0.0"))
	float SurfaceClearance = 150.f;
};

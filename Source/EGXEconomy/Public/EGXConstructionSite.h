#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGXTypes.h"
#include "EGXConstructionSite.generated.h"

/*
 * A temporary actor spawned during placement, which:
 * - stores the future building class
 * - accepts worker build progress
 * - spawns the real building on completion
 */

UCLASS()
class EGXECONOMY_API AEGXConstructionSite : public AActor
{
	GENERATED_BODY()

	public:
	AEGXConstructionSite();

	UFUNCTION(BlueprintCallable)
	void AddProgress(float Delta);

	UFUNCTION(BlueprintCallable)
	bool IsComplete() const { return BuildProgress >= BuildRequired; }

	protected:
	UPROPERTY(EditAnywhere)
	float BuildRequired = 100.0f;

	UPROPERTY(VisibleAnywhere)
	float BuildProgress = 0.0f;
};

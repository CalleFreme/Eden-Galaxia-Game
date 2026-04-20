#pragma once

#include "CoreMinimal.h"
#include "EGXUnitBase.h"
#include "EGXStewardCommander.generated.h"

UCLASS()
class EGXAI_API AEGXStewardCommander : public AEGXUnitBase
{
	GENERATED_BODY()

public:
	AEGXStewardCommander();

	UFUNCTION(BlueprintCallable)
	void EnterBuildMode();

	UFUNCTION(BlueprintCallable)
	void ExitBuildMode();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Commander")
	float BuildRange = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Commander")
	float CombatPower = 100.0f;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GXECameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class EGXRTS_API AEGXCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AEGXCameraPawn();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EGXWorkerUnit.generated.h"

class UEGXInventoryComponent;

UCLASS()
class EGXAI_API AEGXWorkerUnit : public ACharacter
{
	GENERATED_BODY()

public:
	AEGXWorkerUnit();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UEGXInventoryComponent> Inventory;
};

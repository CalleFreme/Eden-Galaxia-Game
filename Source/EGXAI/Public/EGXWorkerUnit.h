#pragma once

#include "CoreMinimal.h"
#include "EGXUnitBase.h"
#include "EGXWorkerUnit.generated.h"

class UEGXInventoryComponent;

UCLASS()
class EGXAI_API AEGXWorkerUnit : public AEGXUnitBase
{
	GENERATED_BODY()

public:
	AEGXWorkerUnit();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UEGXInventoryComponent> Inventory;
};

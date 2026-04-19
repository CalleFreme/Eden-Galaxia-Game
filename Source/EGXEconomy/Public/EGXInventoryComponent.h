#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EGXTypes.h"
#include "EGXInventoryComponent.generated.h"

UCLASS(ClassGroup=(EGX), meta=(BlueprintSpawnableComponent))
class EGXEconomy_API UEGXInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintCallable)
	int32 GetAmount(EEGXResourceType Type) const;

	UFUNCTION(BlueprintCallable)
	void AddResource(EEGXResourceType Type, int32 Delta);

	protected:
	UPROPERTY(EditAnywhere)
	TMap<EEGXResourceType, int32> Resources;
};

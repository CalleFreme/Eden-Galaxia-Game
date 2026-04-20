// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EGXSelectableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UEGXSelectableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EGXGAMEPLAY_API IEGXSelectableInterface
{
	GENERATED_BODY()

public:
	virtual void SetSelectedLocal(bool bSelected) = 0;
	virtual bool CanBeSelectedBy(const APlayerController* PC) const = 0;
	virtual FVector GetSelectionWorldLocation() const = 0;
};

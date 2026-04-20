// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EGXTypes.h"
#include "EGXUnitBase.generated.h"

UCLASS()
class EGXAI_API AEGXUnitBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEGXUnitBase();

	UFUNCTION(BlueprintCallable, Category="EGX Unit")
	virtual void IssueOrder(const FEGXOrder& Order);

	UFUNCTION(BlueprintCallable, Category="EGX Unit")
	virtual void SetSelectedLocal(bool bSelected);

	UFUNCTION(BlueprintCallable, Category="EGX Unit")
	virtual bool IsOwnedBy(const APlayerController* PC) const;
	
protected:	
	UPROPERTY(BlueprintReadOnly, Category="EGX Unit")
	bool bSelectedLocal = false;
};

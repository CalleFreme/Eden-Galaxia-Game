// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EGXHUD.generated.h"

/**
 * 
 */
UCLASS()
class EGXRTS_API AEGXHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Selection")
	FLinearColor SelectionFillColor = FLinearColor(0.f, 0.5f, 1.f, 0.15f);

	UPROPERTY(EditDefaultsOnly, Category="Selection")
	FLinearColor SelectionBorderColor = FLinearColor(0.1f, 0.8f, 1.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category="Selection")
	float SelectionBorderThickness = 2.f;
};

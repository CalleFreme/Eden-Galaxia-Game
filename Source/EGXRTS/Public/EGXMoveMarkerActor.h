// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGXMoveMarkerActor.generated.h"

class UDecalComponent;

UCLASS()
class EGXRTS_API AEGXMoveMarkerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEGXMoveMarkerActor();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDecalComponent> Decal = nullptr;
};

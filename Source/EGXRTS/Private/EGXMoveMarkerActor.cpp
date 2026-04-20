// Fill out your copyright notice in the Description page of Project Settings.


#include "EGXMoveMarkerActor.h"
#include "Components/DecalComponent.h"

// Sets default values
AEGXMoveMarkerActor::AEGXMoveMarkerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	Decal->SetupAttachment(Root);
	Decal->DecalSize = FVector(16.f, 64.f, 64.f);
	Decal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	SetLifeSpan(1.0f);
}




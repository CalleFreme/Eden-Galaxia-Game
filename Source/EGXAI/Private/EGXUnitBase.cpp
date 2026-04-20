// Fill out your copyright notice in the Description page of Project Settings.


#include "EGXUnitBase.h"
#include "AIController.h"
#include "EGXOrderTypes.h"


// Sets default values
AEGXUnitBase::AEGXUnitBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AEGXUnitBase::IssueOrder(const FEGXOrder& Order)
{
	switch (Order.Type)
	{
	case EEGXOrderType::Move:
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			AIC->MoveToLocation(Order.TargetLocation);
		}
		break;

	case EEGXOrderType::Attack:
		// Later: set combat target, StateTree state, etc.
		break;

	default:
		break;
	}
}

void AEGXUnitBase::SetSelectedLocal(bool bSelected)
{
	bSelectedLocal = bSelected;
	// Later: toggle decal/widget/outline.
}

bool AEGXUnitBase::IsOwnedBy(const APlayerController* PC) const
{
	return PC && GetOwner() == PC;
}


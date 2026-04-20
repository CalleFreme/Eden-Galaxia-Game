// Fill out your copyright notice in the Description page of Project Settings.


#include "EGXUnitBase.h"
#include "AIController.h"
#include "EGXOrderTypes.h"
#include "EGXPlanetActor.h"
#include "EGXSurfacePlacementLibrary.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

AEGXUnitBase::AEGXUnitBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SelectionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectionDecal"));
	SelectionDecal->SetupAttachment(RootComponent);
	SelectionDecal->DecalSize = FVector(32.f, 96.f, 96.f);
	SelectionDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	SelectionDecal->SetVisibility(false);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(TEXT("Pawn"));
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
	
	GetCharacterMovement()->GravityScale = 0.f;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AEGXUnitBase::BeginPlay()
{
	Super::BeginPlay();

	if (!ActivePlanet)
	{
		ActivePlanet = FindNearestPlanet();
	}

	if (ActivePlanet)
	{
		SnapToPlanetSurface(true);
	}
}

AEGXPlanetActor* AEGXUnitBase::FindNearestPlanet() const
{
	AEGXPlanetActor* BestPlanet = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (TActorIterator<AEGXPlanetActor> It(GetWorld()); It; ++It)
	{
		AEGXPlanetActor* Planet = *It;
		if (!IsValid(Planet))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(GetActorLocation(), Planet->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestPlanet = Planet;
		}
	}

	return BestPlanet;
}

void AEGXUnitBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bHasPlanetMoveTarget && ActivePlanet)
	{
		TickPlanetMovement(DeltaSeconds);
	}
	else if (ActivePlanet)
	{
		SnapToPlanetSurface(true);
	}
}

void AEGXUnitBase::IssueOrder(const FEGXOrder& Order)
{
	switch (Order.Type)
	{
	case EEGXOrderType::Move:
		{
			AEGXPlanetActor* PlanetForMove = ActivePlanet;

			if (!PlanetForMove)
			{
				for (TActorIterator<AEGXPlanetActor> It(GetWorld()); It; ++It)
				{
					PlanetForMove = *It;
					break;
				}
			}

			if (PlanetForMove)
			{
				SetPlanetMoveTarget(PlanetForMove, Order.TargetLocation);
			}
			else if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				AIC->MoveToLocation(Order.TargetLocation);
			}
		}
		break;

	case EEGXOrderType::Attack:
		break;

	default:
		break;
	}
}

void AEGXUnitBase::SetSelectedLocal(bool bSelected)
{
	if (bSelectedLocal == bSelected)
	{
		return;
	}

	bSelectedLocal = bSelected;

	if (SelectionDecal)
	{
		SelectionDecal->SetVisibility(bSelectedLocal || !bHideSelectionDecalWhenNotSelected);
	}

	if (bUseCustomDepthOutlineWhenSelected)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
	}
}

bool AEGXUnitBase::IsOwnedBy(const APlayerController* PC) const
{
	return PC && GetOwner() == PC;
}

void AEGXUnitBase::SetActivePlanet(AEGXPlanetActor* InPlanet)
{
	ActivePlanet = InPlanet;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (ActivePlanet)
		{
			MoveComp->GravityScale = 0.f;
			MoveComp->SetMovementMode(MOVE_Flying);
		}
	}

	if (ActivePlanet)
	{
		SnapToPlanetSurface(true);
	}
}

float AEGXUnitBase::GetSurfaceClearance() const
{
	if (SurfaceClearanceOverride > 0.f)
	{
		return SurfaceClearanceOverride;
	}

	FVector Origin, Extent;
	GetActorBounds(true, Origin, Extent);
	return Extent.Z;
}

void AEGXUnitBase::SnapToPlanetSurface(bool bAlignRotation)
{
	if (!ActivePlanet)
	{
		return;
	}

	const float Clearance = GetSurfaceClearance();
	const FVector SurfacePoint = ActivePlanet->ProjectPointToSurface(GetActorLocation(), Clearance);
	const FVector SurfaceNormal = ActivePlanet->GetSurfaceNormalAt(SurfacePoint);

	SetActorLocation(SurfacePoint);

	if (bAlignRotation)
	{
		const FRotator Rot = UEGXSurfacePlacementLibrary::MakeTangentFacingRotation(
			SurfaceNormal,
			GetActorForwardVector());

		SetActorRotation(Rot);
	}
}

void AEGXUnitBase::SetPlanetMoveTarget(AEGXPlanetActor* InPlanet, const FVector& InTargetWorld)
{
	ActivePlanet = InPlanet;
	MoveTargetWorld = InPlanet ? InPlanet->ProjectPointToSurface(InTargetWorld, 0.f) : InTargetWorld;
	MoveTargetSurfaceNormal = InPlanet ? InPlanet->GetSurfaceNormalAt(MoveTargetWorld) : FVector::UpVector;
	bHasPlanetMoveTarget = ActivePlanet != nullptr;

	if (ActivePlanet)
	{
		SnapToPlanetSurface(true);
	}
}

void AEGXUnitBase::TickPlanetMovement(float DeltaSeconds)
{
	if (!ActivePlanet)
	{
		bHasPlanetMoveTarget = false;
		return;
	}

	const FVector PlanetCenter = ActivePlanet->GetActorLocation();
	const float Radius = FVector::Distance(
		ActivePlanet->ProjectPointToSurface(GetActorLocation(), 0.f),
		PlanetCenter);

	if (Radius <= KINDA_SMALL_NUMBER)
	{
		bHasPlanetMoveTarget = false;
		return;
	}

	const FVector CurrentNormal = ActivePlanet->GetSurfaceNormalAt(GetActorLocation());
	const FVector TargetNormal = MoveTargetSurfaceNormal.GetSafeNormal();

	const float Dot = FMath::Clamp(FVector::DotProduct(CurrentNormal, TargetNormal), -1.f, 1.f);
	const float AngularDistance = FMath::Acos(Dot);
	const float SurfaceDistance = AngularDistance * Radius;

	if (SurfaceDistance <= ArrivalDistance)
	{
		bHasPlanetMoveTarget = false;

		const float Clearance = GetSurfaceClearance();
		const FVector FinalLocation = ActivePlanet->ProjectPointToSurface(MoveTargetWorld, Clearance);
		const FVector FinalUp = ActivePlanet->GetSurfaceNormalAt(FinalLocation);

		FVector FinalForward = FVector::VectorPlaneProject(
			MoveTargetWorld - GetActorLocation(),
			FinalUp).GetSafeNormal();

		if (FinalForward.IsNearlyZero())
		{
			FinalForward = FVector::VectorPlaneProject(GetActorForwardVector(), FinalUp).GetSafeNormal();
		}

		SetActorLocation(FinalLocation);
		SetActorRotation(UEGXSurfacePlacementLibrary::MakeTangentFacingRotation(FinalUp, FinalForward));
		return;
	}

	const float StepDistance = PlanetMoveSpeed * DeltaSeconds;
	const float StepAngle = StepDistance / Radius;

	FVector Axis = FVector::CrossProduct(CurrentNormal, TargetNormal).GetSafeNormal();
	if (Axis.IsNearlyZero())
	{
		bHasPlanetMoveTarget = false;
		return;
	}

	const float AppliedAngle = FMath::Min(StepAngle, AngularDistance);
	const FQuat StepRot(Axis, AppliedAngle);

	const FVector NewNormal = StepRot.RotateVector(CurrentNormal).GetSafeNormal();
	const float Clearance = GetSurfaceClearance();
	const FVector NewLocation = PlanetCenter + NewNormal * (Radius + Clearance);

	FVector TangentForward = FVector::VectorPlaneProject(NewLocation - GetActorLocation(), NewNormal).GetSafeNormal();
	if (TangentForward.IsNearlyZero())
	{
		TangentForward = FVector::VectorPlaneProject(GetActorForwardVector(), NewNormal).GetSafeNormal();
	}

	const FRotator TargetRot = UEGXSurfacePlacementLibrary::MakeTangentFacingRotation(NewNormal, TangentForward);
	const FRotator SmoothedRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaSeconds, RotationInterpSpeed);

	SetActorLocation(NewLocation);
	SetActorRotation(SmoothedRot);
}
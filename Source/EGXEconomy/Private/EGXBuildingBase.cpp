// Fill out your copyright notice in the Description page of Project Settings.


#include "EGXBuildingBase.h"
#include "EGXPlanetActor.h"
#include "EGXSurfacePlacementLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

AEGXBuildingBase::AEGXBuildingBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(Root);
	BuildingMesh->SetCollisionProfileName(TEXT("BlockAll"));
	BuildingMesh->SetGenerateOverlapEvents(false);

	PlacementBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("PlacementBounds"));
	PlacementBounds->SetupAttachment(Root);
	PlacementBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlacementBounds->SetBoxExtent(FVector(200.f, 200.f, 200.f));

	SelectionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectionDecal"));
	SelectionDecal->SetupAttachment(Root);
	SelectionDecal->DecalSize = FVector(128.f, 256.f, 256.f);
	SelectionDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	SelectionDecal->SetVisibility(false);

	CurrentHealth = MaxHealth;
}

// Called when the game starts or when spawned
void AEGXBuildingBase::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	ConstructionProgress = FMath::Clamp(ConstructionProgress, 0.f, 1.f);

	PlacementBounds->SetBoxExtent(FVector(
		FootprintSize.X * 0.5f,
		FootprintSize.Y * 0.5f,
		200.f));

	if (AEGXPlanetActor* Planet = FindNearestPlanet())
	{
		SnapToPlanetSurface(Planet, GetActorLocation(), GetActorRotation().Yaw);
	}

	UpdateVisualState();
}

AEGXPlanetActor* AEGXBuildingBase::FindNearestPlanet() const
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

// Called every frame
void AEGXBuildingBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEGXBuildingBase::SetSelectedLocal(bool bInSelected)
{
	if (bSelectedLocal == bInSelected)
	{
		return;
	}

	bSelectedLocal = bInSelected;

	if (SelectionDecal)
	{
		SelectionDecal->SetVisibility(bSelectedLocal || !bHideSelectionDecalWhenNotSelected);
	}

	BP_OnSelectionChanged(bSelectedLocal);
}

void AEGXBuildingBase::SetOwningPlayerId(int32 InPlayerId)
{
	OwningPlayerId = InPlayerId;
}

void AEGXBuildingBase::SetTeamId(int32 InTeamId)
{
	TeamId = InTeamId;
}

void AEGXBuildingBase::StartConstruction(float InStartingProgress)
{
	BuildingState = EEGXBuildingState::UnderConstruction;
	ConstructionProgress = FMath::Clamp(InStartingProgress, 0.f, 1.f);

	// Optional design choice:
	// start with partial health while under construction
	CurrentHealth = FMath::Max(1.f, MaxHealth * FMath::Max(0.1f, ConstructionProgress));

	UpdateVisualState();
	BP_OnConstructionStarted();

	if (ConstructionProgress >= 1.f)
	{
		FinishConstruction();
	}
}

void AEGXBuildingBase::AddConstructionProgress(float DeltaProgress)
{
	if (BuildingState != EEGXBuildingState::UnderConstruction)
	{
		return;
	}

	if (DeltaProgress <= 0.f)
	{
		return;
	}

	ConstructionProgress = FMath::Clamp(ConstructionProgress + DeltaProgress, 0.f, 1.f);

	// Let health rise with construction.
	CurrentHealth = FMath::Clamp(MaxHealth * FMath::Max(0.1f, ConstructionProgress), 1.f, MaxHealth);

	BP_OnHealthChanged(CurrentHealth, MaxHealth);
	UpdateVisualState();

	if (ConstructionProgress >= 1.f)
	{
		FinishConstruction();
	}
}

void AEGXBuildingBase::FinishConstruction()
{
	ConstructionProgress = 1.f;
	CurrentHealth = MaxHealth;
	BuildingState = EEGXBuildingState::Operational;

	BP_OnHealthChanged(CurrentHealth, MaxHealth);
	UpdateVisualState();
	BP_OnConstructionFinished();
}

void AEGXBuildingBase::ApplyDamageToBuilding(float DamageAmount)
{
	if (DamageAmount <= 0.f || BuildingState == EEGXBuildingState::Destroyed)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);

	BP_OnHealthChanged(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		HandleDestroyed();
	}
}

void AEGXBuildingBase::RepairBuilding(float RepairAmount)
{
	if (RepairAmount <= 0.f || BuildingState == EEGXBuildingState::Destroyed)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + RepairAmount, 0.f, MaxHealth);
	BP_OnHealthChanged(CurrentHealth, MaxHealth);

	UpdateVisualState();
}

void AEGXBuildingBase::HandleDestroyed()
{
	if (BuildingState == EEGXBuildingState::Destroyed)
	{
		return;
	}

	BuildingState = EEGXBuildingState::Destroyed;
	CurrentHealth = 0.f;

	UpdateVisualState();
	BP_OnDestroyed();

	// For the first version, disable collision and hide selection.
	SetSelectedLocal(false);

	if (BuildingMesh)
	{
		BuildingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorEnableCollision(false);

	// Later:
	// - spawn wreckage
	// - notify subsystems
	// - maybe keep reclaimable ruins
	Destroy();
}

void AEGXBuildingBase::UpdateVisualState_Implementation()
{
	if (!BuildingMesh)
	{
		return;
	}

	switch (BuildingState)
	{
	case EEGXBuildingState::Placed:
		// This will only work if our material actually uses this scalar parameter. If not, nothing visual happens.
		BuildingMesh->SetScalarParameterValueOnMaterials(TEXT("ConstructionAlpha"), 1.0f);
		break;

	case EEGXBuildingState::UnderConstruction:
		BuildingMesh->SetScalarParameterValueOnMaterials(TEXT("ConstructionAlpha"), ConstructionProgress);
		break;

	case EEGXBuildingState::Operational:
		BuildingMesh->SetScalarParameterValueOnMaterials(TEXT("ConstructionAlpha"), 1.0f);
		break;

	case EEGXBuildingState::Destroyed:
		BuildingMesh->SetScalarParameterValueOnMaterials(TEXT("ConstructionAlpha"), 0.0f);
		break;

	default:
		break;
	}
}

void AEGXBuildingBase::SnapToPlanetSurface(AEGXPlanetActor* Planet, const FVector& NearWorldPoint, float YawDegrees)
{
	if (!Planet)
	{
		return;
	}

	float Clearance = SurfaceClearance;
	if (Clearance <= 0.f)
	{
		FVector Origin, Extent;
		GetActorBounds(true, Origin, Extent);
		Clearance = Extent.Z;
	}

	const FTransform Xf = UEGXSurfacePlacementLibrary::MakePlanetPlacementTransform(
		Planet,
		NearWorldPoint,
		Clearance,
		YawDegrees);

	SetActorLocationAndRotation(Xf.GetLocation(), Xf.Rotator());
}
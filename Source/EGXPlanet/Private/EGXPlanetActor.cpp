#include "EGXPlanetActor.h"
#include "ProceduralMeshComponent.h"

AEGXPlanetActor::AEGXPlanetActor()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PlanetMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PlanetMesh"));
	PlanetMesh->SetupAttachment(RootComponent);
}

void AEGXPlanetActor::GeneratePlanet()
{
	// First pass: generate a simple sphere or cube-sphere chunk set.
	// Keep this extremely simple at first. Prove placement + movement + camera before advanced topology.
}

FVector AEGXPlanetActor::GetSurfacePointFromNormal(const FVector& UnitNormal, float HeightOffset) const
{
	return GetActorLocation() + UnitNormal.GetSafeNormal() * (PlanetRadius + HeightOffset);
}

FRotator AEGXPlanetActor::GetSurfaceRotationFromNormal(const FVector& UnitNormal) const
{
	const FVector Up = UnitNormal.GetSafeNormal();
	return FRotationMatrix::MakeFromZ(Up).Rotator();
}

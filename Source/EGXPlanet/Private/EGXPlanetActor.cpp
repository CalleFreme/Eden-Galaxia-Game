#include "EGXPlanetActor.h"
#include "ProceduralMeshComponent.h"

AEGXPlanetActor::AEGXPlanetActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRootComponent;

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

FVector AEGXPlanetActor::GetSurfaceNormalAt(const FVector& WorldPoint) const
{
	return (WorldPoint - PlanetCenter).GetSafeNormal();
}

FVector AEGXPlanetActor::ProjectPointToSurface(const FVector& WorldPoint, float HeightOffset) const
{
	const FVector Normal = GetSurfaceNormalAt(WorldPoint);
	return PlanetCenter + Normal * (PlanetRadius + HeightOffset);
}

bool AEGXPlanetActor::RaycastToPlanet(const FVector& RayOrigin, const FVector& RayDirection, FVector& OutPoint, FVector& OutNormal) const
{
	const FVector Dir = RayDirection.GetSafeNormal();
	const FVector L = RayOrigin - PlanetCenter;

	const float A = FVector::DotProduct(Dir, Dir);
	const float B = 2.f * FVector::DotProduct(Dir, L);
	const float C = FVector::DotProduct(L, L) - FMath::Square(PlanetRadius);

	const float Discriminant = B * B - 4.f * A * C;
	if (Discriminant < 0.f)
	{
		return false;
	}

	const float SqrtD = FMath::Sqrt(Discriminant);
	const float T0 = (-B - SqrtD) / (2.f * A);
	const float T1 = (-B + SqrtD) / (2.f * A);

	float T = TNumericLimits<float>::Max();
	if (T0 > 0.f) T = T0;
	else if (T1 > 0.f) T = T1;

	if (!FMath::IsFinite(T) || T == TNumericLimits<float>::Max())
	{
		return false;
	}

	OutPoint = RayOrigin + Dir * T;
	OutNormal = GetSurfaceNormalAt(OutPoint);
	return true;
}

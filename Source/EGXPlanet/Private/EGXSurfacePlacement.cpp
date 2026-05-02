#include "EGXSurfacePlacementLibrary.h"
#include "EGXPlanetActor.h"
#include "GameFramework/Actor.h"

FTransform UEGXSurfacePlacementLibrary::MakeSurfaceAlignedTransform(const FVector& Origin, const FVector& SurfaceNormal, float YawDegrees)
{
	const FVector Up = SurfaceNormal.GetSafeNormal();
	const FQuat Base = FRotationMatrix::MakeFromZ(Up).ToQuat();
	const FQuat Yaw = FQuat(Up, FMath::DegreesToRadians(YawDegrees));
	return FTransform(Yaw * Base, Origin);
}

FTransform UEGXSurfacePlacementLibrary::MakePlanetPlacementTransform(
	const AEGXPlanetActor* Planet,
	const FVector& WorldPoint,
	float Clearance,
	float YawDegrees)
{
	if (!Planet)
	{
		return FTransform(FRotator(0.f, YawDegrees, 0.f), WorldPoint);
	}

	const FVector Normal = Planet->GetSurfaceNormalAt(WorldPoint);
	const FVector Position = Planet->ProjectPointToSurface(WorldPoint, Clearance);
	return MakeSurfaceAlignedTransform(Position, Normal, YawDegrees);
}

FRotator UEGXSurfacePlacementLibrary::MakeTangentFacingRotation(
	const FVector& SurfaceNormal,
	const FVector& DesiredForward)
{
	const FVector Up = SurfaceNormal.GetSafeNormal();

	FVector Forward = FVector::VectorPlaneProject(DesiredForward, Up).GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::VectorPlaneProject(FVector::ForwardVector, Up).GetSafeNormal();
		if (Forward.IsNearlyZero())
		{
			Forward = FVector::VectorPlaneProject(FVector::RightVector, Up).GetSafeNormal();
		}
	}

	return FRotationMatrix::MakeFromXZ(Forward, Up).Rotator();
}

bool UEGXSurfacePlacementLibrary::SnapActorToPlanetSurface(
	AActor* Actor,
	const AEGXPlanetActor* Planet,
	float Clearance,
	bool bAlignRotation,
	float YawDegrees)
{
	if (!Actor || !Planet)
	{
		return false;
	}

	float EffectiveClearance = Clearance;
	if (EffectiveClearance <= 0.f)
	{
		FVector Origin;
		FVector Extent;
		Actor->GetActorBounds(true, Origin, Extent);
		EffectiveClearance = Extent.Z;
	}

	const FTransform SurfaceTransform = MakePlanetPlacementTransform(
		Planet,
		Actor->GetActorLocation(),
		EffectiveClearance,
		YawDegrees);

	if (bAlignRotation)
	{
		Actor->SetActorLocationAndRotation(SurfaceTransform.GetLocation(), SurfaceTransform.Rotator());
	}
	else
	{
		Actor->SetActorLocation(SurfaceTransform.GetLocation());
	}

	return true;
}

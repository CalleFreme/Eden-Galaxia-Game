#include "EGXSurfacePlacementLibrary.h"

FTransform UEGXSurfacePlacementLibrary::MakeSurfaceAlignedTransform(const FVector& Origin, const FVector& SurfaceNormal, float YawDegrees)
{
	const FVector Up = SurfaceNormal.GetSafeNormal();
	const FQuat Base = FRotationMatrix::MakeFromZ(Up).ToQuat();
	const FQuat Yaw = FQuat(Up, FMath::DegreesToRadians(YawDegrees));
	return FTransform(Yaw * Base, Origin);
}

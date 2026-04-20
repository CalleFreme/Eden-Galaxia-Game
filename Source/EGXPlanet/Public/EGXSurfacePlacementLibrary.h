#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "EGXSurfacePlacementLibrary.generated.h"

UCLASS()
class EGXPLANET_API UEGXSurfacePlacementLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="EGX|SurfacePlacement")
	static FTransform MakeSurfaceAlignedTransform(const FVector& Origin, const FVector& SurfaceNormal, float YawDegrees);
	
	UFUNCTION(BlueprintPure, Category="EGX|SurfacePlacement")
	static FTransform MakePlanetPlacementTransform(
		const AEGXPlanetActor* Planet,
		const FVector& WorldPoint,
		float Clearance,
		float YawDegrees);

	UFUNCTION(BlueprintPure, Category="EGX|SurfacePlacement")
	static FRotator MakeTangentFacingRotation(
		const FVector& SurfaceNormal,
		const FVector& DesiredForward);
};

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "EGXSurfacePlacementLibrary.generated.h"

UCLASS()
class EGXPLANET_API UEGXSurfacePlacementLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="EGX|Planet")
	static FTransform MakeSurfaceAlignedTransform(const FVector& Origin, const FVector& SurfaceNormal, float YawDegrees);
};

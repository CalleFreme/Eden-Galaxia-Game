#include "EGXConstructionSite.h"

AEGXConstructionSite::AEGXConstructionSite()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEGXConstructionSite::AddProgress(float Delta)
{
	BuildProgress = FMath::Clamp(BuildProgress + Delta, 0.0f, BuildRequired);
}

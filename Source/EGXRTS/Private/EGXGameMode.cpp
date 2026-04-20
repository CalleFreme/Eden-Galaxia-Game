#include "EGXGameMode.h"

#include "EGXCameraPawn.h"
#include "EGXPlayerController.h"
#include "EGXHUD.h"
#include "EGXPlanetActor.h"
#include "EGXStewardCommander.h"
#include "EGXWorkerUnit.h"
#include "EGXScoutUnit.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AEGXGameMode::AEGXGameMode()
{
	PlayerControllerClass = AEGXPlayerController::StaticClass();
	DefaultPawnClass = AEGXCameraPawn::StaticClass();
	HUDClass = AEGXHUD::StaticClass();
}

void AEGXGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("EGXGameMode BeginPlay"));

	InitializePlayerCamera();

	if (bSpawnStarterUnits)
	{
		SpawnStarterUnits();
	}
}

void AEGXGameMode::InitializePlayerCamera()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializePlayerCamera: No PlayerController found."));
		return;
	}

	AEGXCameraPawn* CameraPawn = Cast<AEGXCameraPawn>(PC->GetPawn());
	if (!CameraPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializePlayerCamera: PlayerController has no AEGXCameraPawn pawn."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("InitializePlayerCamera: Camera pawn found: %s"), *GetNameSafe(CameraPawn));

	if (!bInitializeCameraFromPlanet)
	{
		CameraPawn->InitializeForFlat(FallbackCameraLocation);
		return;
	}

	AEGXPlanetActor* Planet = nullptr;
	for (TActorIterator<AEGXPlanetActor> It(GetWorld()); It; ++It)
	{
		Planet = *It;
		break;
	}

	if (!Planet)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializePlayerCamera: No planet found. Using flat fallback."));
		CameraPawn->InitializeForFlat(FallbackCameraLocation);
		return;
	}

	// This puts us at top of planet. Make configurable.
	const FVector SurfacePoint = Planet->GetSurfacePointFromNormal(FVector::UpVector, 0.f);
	CameraPawn->InitializeForPlanet(Planet, SurfacePoint);

	UE_LOG(LogTemp, Warning, TEXT("InitializePlayerCamera: Camera initialized for planet %s at %s"),
		*GetNameSafe(Planet),
		*SurfacePoint.ToString());
}

void AEGXGameMode::SpawnStarterUnits()
{
	AEGXPlanetActor* Planet = nullptr;
	for (TActorIterator<AEGXPlanetActor> It(GetWorld()); It; ++It)
	{
		Planet = *It;
		break;
	}

	FVector SpawnCenter = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FVector SurfaceNormal = FVector::UpVector;

	if (Planet)
	{
		SurfaceNormal = FVector::UpVector;
		SpawnCenter = Planet->GetSurfacePointFromNormal(SurfaceNormal, 150.f);
		SpawnRotation = Planet->GetSurfaceRotationFromNormal(SurfaceNormal);
	}
	else
	{
		SpawnCenter = FVector(0.f, 0.f, 100.f);
		SpawnRotation = FRotator::ZeroRotator;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto MakePlanetSurfaceSpawn = [&](const FVector2D& TangentOffset) -> FTransform
	{
		if (!Planet)
		{
			const FVector WorldPoint = SpawnCenter + FVector(TangentOffset.X, TangentOffset.Y, 0.f);
			return FTransform(SpawnRotation, WorldPoint);
		}

		const FVector Up = SurfaceNormal.GetSafeNormal();

		FVector TangentA = FVector::CrossProduct(FVector::UpVector, Up);
		if (TangentA.IsNearlyZero())
		{
			TangentA = FVector::CrossProduct(FVector::ForwardVector, Up);
		}
		TangentA.Normalize();

		const FVector TangentB = FVector::CrossProduct(Up, TangentA).GetSafeNormal();

		const FVector RawPoint = SpawnCenter + (TangentA * TangentOffset.X) + (TangentB * TangentOffset.Y);
		const FVector SurfacePoint = Planet->ProjectPointToSurface(RawPoint, 150.f);
		const FVector SurfaceUp = Planet->GetSurfaceNormalAt(SurfacePoint);
		const FRotator SurfaceRot = Planet->GetSurfaceRotationFromNormal(SurfaceUp);

		return FTransform(SurfaceRot, SurfacePoint);
	};

	if (StewardCommanderClass)
	{
		const FTransform Xf = MakePlanetSurfaceSpawn(FVector2D::ZeroVector);
		if (AEGXStewardCommander* StewardCommander = GetWorld()->SpawnActor<AEGXStewardCommander>(StewardCommanderClass, Xf.GetLocation(), Xf.Rotator(), Params))
		{
			StewardCommander->SetActivePlanet(Planet);
			StewardCommander->SnapToPlanetSurface(true);
		}
	}

	if (WorkerUnitClass)
	{
		for (int32 i = 0; i < NumStartingWorkers; ++i)
		{
			const FVector2D Offset(
				FMath::FRandRange(-StarterSpawnRadius, StarterSpawnRadius),
				FMath::FRandRange(-StarterSpawnRadius, StarterSpawnRadius));

			const FTransform Xf = MakePlanetSurfaceSpawn(Offset);
			if (AEGXWorkerUnit* Worker = GetWorld()->SpawnActor<AEGXWorkerUnit>(WorkerUnitClass, Xf.GetLocation(), Xf.Rotator(), Params))
			{
				Worker->SetActivePlanet(Planet);
				Worker->SnapToPlanetSurface(true);
			}
		}
	}

	if (ScoutUnitClass)
	{
		const FTransform Xf = MakePlanetSurfaceSpawn(FVector2D(StarterSpawnRadius, 0.f));
		if (AEGXScoutUnit* Scout = GetWorld()->SpawnActor<AEGXScoutUnit>(ScoutUnitClass, Xf.GetLocation(), Xf.Rotator(), Params))
		{
			Scout->SetActivePlanet(Planet);
			Scout->SnapToPlanetSurface(true);
		}
	}
}
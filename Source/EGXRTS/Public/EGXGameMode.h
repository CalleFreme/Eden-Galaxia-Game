#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EGXGameMode.generated.h"

class AEGXPlanetActor;
class AEGXCameraPawn;
class AEGXStewardCommander;
class AEGXWorkerUnit;
class AEGXScoutUnit;
class AEGXPlayerStart;

UCLASS()
class EGXRTS_API AEGXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEGXGameMode();
	
	virtual void BeginPlay() override;
	
protected:
	void InitializePlayerCamera(const AActor* PreferredFocusActor = nullptr);
	AEGXStewardCommander* SpawnStarterUnits();
	AEGXPlayerStart* FindPlayerStartActor(int32 PlayerId) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	bool bInitializeCameraFromPlanet = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	bool bSpawnStarterUnits = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	FVector FallbackCameraLocation = FVector(0.f, 0.f, 2000.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	FVector FallbackCameraFocus = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	TSubclassOf<AEGXStewardCommander> StewardCommanderClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	TSubclassOf<AEGXWorkerUnit> WorkerUnitClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	TSubclassOf<AEGXScoutUnit> ScoutUnitClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	int32 NumStartingWorkers = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	float StarterSpawnRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EGX|Startup")
	FVector StarterSpawnSurfaceNormal = FVector::UpVector;
	
};

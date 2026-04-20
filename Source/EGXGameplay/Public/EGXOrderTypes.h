#pragma once

#include "CoreMinimal.h"
#include "EGXOrderTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EEGXOrderType : uint8
{
	None,
	Move,
	Attack,
	Gather,
	Build,
	Repair,
	Assist,
	Stop
};

USTRUCT(BlueprintType)
struct EGXGAMEPLAY_API FEGXOrder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EEGXOrderType Type = EEGXOrderType::None;

	UPROPERTY(BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor = nullptr;

	// Used when Type == Build
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPrimaryAssetId BuildDefintionId;
};
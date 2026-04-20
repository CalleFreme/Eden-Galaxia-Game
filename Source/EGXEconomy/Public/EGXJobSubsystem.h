#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EGXTypes.h"
#include "EGXJobSubsystem.generated.h"

UENUM(BlueprintType)
enum class EEGXJobType : uint8
{
	None,
	Harvest,
	Haul,
	Build,
	Repair
};

USTRUCT(BlueprintType)
struct FEGXJobRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEGXJobType Type = EEGXJobType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

UCLASS()
class EGXECONOMY_API UEGXJobSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintCallable)
	void EnqueueJob(const FEGXJobRecord& Job);

	UFUNCTION(BlueprintCallable)
	bool TryClaimBestJob(FEGXJobRecord& OutJob);

	protected:
	UPROPERTY()
	TArray<FEGXJobRecord> PendingJobs;
};

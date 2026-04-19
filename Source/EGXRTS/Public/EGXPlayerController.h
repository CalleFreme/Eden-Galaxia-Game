#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EGXPlayerController.generated.h"

class UEGXSelectionSubsystem;

UCLASS()
class EGXRTS_API AEGXPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEGXPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void Input_StartSelect();
	void Input_EndSelect();
	void Input_IssueMoveCommand();
};

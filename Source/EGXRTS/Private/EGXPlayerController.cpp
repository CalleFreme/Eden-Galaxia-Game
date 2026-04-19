#include "EGXPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AEGXPlayerController::AEGXPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AEGXPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AEGXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Bind Input Actions from BP/asset setup.
	}
}

void AEGXPlayerController::Input_StartSelect() {}
void AEGXPlayerController::Input_EndSelect() {}
void AEGXPlayerController::Input_IssueMoveCommand() {}

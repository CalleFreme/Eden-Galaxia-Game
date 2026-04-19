#include "EGXGameMode.h"
#include "EGXCameraPawn.h"
#include "EGXPlayerController.h"

AEGXGameMode::AEGXGameMode()
{
	PlayerControllerClass = AEGXPlayerController::StaticClass();
	DefaultPawnClass = AEGXCameraPawn::StaticClass();
}

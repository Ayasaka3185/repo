#include "ProjectUIGameMode.h"

#include "ProjectUIPausePlayerController.h"

AProjectUIGameMode::AProjectUIGameMode()
{
	PlayerControllerClass = AProjectUIPausePlayerController::StaticClass();
}

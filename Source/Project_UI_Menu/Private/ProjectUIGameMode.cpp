#include "ProjectUIGameMode.h"

#include "Dialogue/UIDemoPlayerCharacter.h"
#include "ProjectUIPausePlayerController.h"

AProjectUIGameMode::AProjectUIGameMode()
{
	PlayerControllerClass = AProjectUIPausePlayerController::StaticClass();
	DefaultPawnClass = AUIDemoPlayerCharacter::StaticClass();
}

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectUIPausePlayerController.generated.h"

class UPauseMenuWidget;

UCLASS()
class PROJECT_UI_MENU_API AProjectUIPausePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void TogglePauseMenu();

	UPROPERTY(Transient)
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectUIPausePlayerController.generated.h"

class UPauseMenuWidget;
class AUIDemoDialogueNPC;

UCLASS()
class PROJECT_UI_MENU_API AProjectUIPausePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void TogglePauseMenu();
	void TryInteract();
	void SpawnDemoNPCIfNeeded();
	AUIDemoDialogueNPC* FindNearestInteractableNPC() const;

	UPROPERTY(Transient)
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<AUIDemoDialogueNPC> SpawnedDemoNPC;
};

#include "ProjectUIPausePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "PauseMenuWidget.h"

void AProjectUIPausePlayerController::BeginPlay()
{
	Super::BeginPlay();

	const FString MapName = GetWorld() ? GetWorld()->GetMapName() : FString();
	const bool bInMenuMap = MapName.Contains(TEXT("L_UI_Demo"));
	SetShowMouseCursor(bInMenuMap);
	if (bInMenuMap)
	{
		SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}

void AProjectUIPausePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		FInputKeyBinding& EscapeBinding = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AProjectUIPausePlayerController::TogglePauseMenu);
		EscapeBinding.bExecuteWhenPaused = true;

		FInputKeyBinding& TestBinding = InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AProjectUIPausePlayerController::TogglePauseMenu);
		TestBinding.bExecuteWhenPaused = true;
	}
}

void AProjectUIPausePlayerController::TogglePauseMenu()
{
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		return;
	}

	PauseMenuWidget = CreateWidget<UPauseMenuWidget>(this, UPauseMenuWidget::StaticClass());
	if (!PauseMenuWidget)
	{
		return;
	}

	SetPause(true);
	SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	PauseMenuWidget->AddToViewport(100);
	PauseMenuWidget->SetKeyboardFocus();
}

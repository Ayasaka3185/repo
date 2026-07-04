#include "ProjectUIPausePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Dialogue/DialogueComponent.h"
#include "Dialogue/UIDemoDialogueNPC.h"
#include "Engine/World.h"
#include "EngineUtils.h"
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
		SpawnDemoNPCIfNeeded();
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

		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AProjectUIPausePlayerController::TryInteract);
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

void AProjectUIPausePlayerController::TryInteract()
{
	AUIDemoDialogueNPC* NPC = FindNearestInteractableNPC();
	if (!NPC || !NPC->GetDialogueComponent())
	{
		return;
	}

	NPC->GetDialogueComponent()->StartDialogue(this);
}

void AProjectUIPausePlayerController::SpawnDemoNPCIfNeeded()
{
	if (!GetWorld() || SpawnedDemoNPC)
	{
		return;
	}

	for (TActorIterator<AUIDemoDialogueNPC> It(GetWorld()); It; ++It)
	{
		SpawnedDemoNPC = *It;
		return;
	}

	APawn* PlayerPawn = GetPawn();
	const FVector BaseLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
	const FRotator BaseRotation = PlayerPawn ? PlayerPawn->GetActorRotation() : FRotator::ZeroRotator;
	const FVector DemoNPCSpawnLocation = BaseLocation + BaseRotation.RotateVector(FVector(650.f, 180.f, 96.f));
	SpawnedDemoNPC = GetWorld()->SpawnActor<AUIDemoDialogueNPC>(AUIDemoDialogueNPC::StaticClass(), DemoNPCSpawnLocation, FRotator::ZeroRotator);
}

AUIDemoDialogueNPC* AProjectUIPausePlayerController::FindNearestInteractableNPC() const
{
	if (!GetWorld() || !GetPawn())
	{
		return nullptr;
	}

	AUIDemoDialogueNPC* BestNPC = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();
	for (TActorIterator<AUIDemoDialogueNPC> It(GetWorld()); It; ++It)
	{
		AUIDemoDialogueNPC* NPC = *It;
		if (!NPC || !NPC->IsPlayerInInteractionRange(GetPawn()))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetPawn()->GetActorLocation(), NPC->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestNPC = NPC;
		}
	}
	return BestNPC;
}

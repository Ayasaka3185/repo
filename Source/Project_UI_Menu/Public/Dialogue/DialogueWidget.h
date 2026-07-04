#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue/UIDialogueTypes.h"
#include "DialogueWidget.generated.h"

class UBorder;
class UButton;
class UCanvasPanel;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;
class UDialogueComponent;
class USoundBase;

UCLASS()
class PROJECT_UI_MENU_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDialogueWidget(const FObjectInitializer& ObjectInitializer);

	void StartDialogue(UDialogueComponent* InDialogueComponent, const FUIDialogueNode& FirstNode);
	void ShowNode(const FUIDialogueNode& Node);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void AdvanceDefault();
	void ChooseCurrentSelection();
	void SetChoiceIndex(int32 ChoiceIndex, bool bPlayHoverSound);
	void UpdateChoiceVisuals();
	void EndDialogue();
	void PlayConfirmClick() const;
	void PlayHoverClick() const;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SpeakerText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ChoiceList;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> ShortcutHint_Panel;

	UPROPERTY(Transient)
	TObjectPtr<UDialogueComponent> DialogueComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ChoiceTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> ChoiceBorders;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> HoverSound;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> ConfirmClickSound;

	FUIDialogueNode CurrentNode;
	int32 SelectedChoiceIndex = 0;
	int32 HoveredChoiceIndex = INDEX_NONE;

	UFUNCTION()
	void HandleChoice0Hovered();
	UFUNCTION()
	void HandleChoice1Hovered();
	UFUNCTION()
	void HandleChoice2Hovered();
	UFUNCTION()
	void HandleChoice3Hovered();
	UFUNCTION()
	void HandleChoice4Hovered();
	UFUNCTION()
	void HandleChoice5Hovered();
	UFUNCTION()
	void HandleChoiceClicked();
	UFUNCTION()
	void HandleChoiceUnhovered();
};

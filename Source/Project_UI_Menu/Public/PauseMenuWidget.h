#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UBorder;
class UButton;
class UCanvasPanel;
class UHorizontalBox;
class USoundBase;
class UTextBlock;
class UVerticalBox;
class UWidget;

UENUM()
enum class EPauseMenuScreen : uint8
{
	Main,
	Options,
	SaveList,
	ReturnMainConfirm,
	QuitConfirm
};

UENUM()
enum class EPauseConfirmChoice : uint8
{
	Yes,
	No
};

UCLASS()
class PROJECT_UI_MENU_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pause Menu|Navigation")
	FName MainMenuLevelName = TEXT("/Game/UI/demo/L_UI_Demo");

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void BuildMainMenu();
	void BuildOptionsMenu();
	void BuildSaveListMenu();
	void BuildConfirmMenu(TObjectPtr<UCanvasPanel>& OutPanel, const FName& PanelName, const FString& Title, const FString& Question, bool bReturnMain);
	void RebuildShortcutHints(bool bIncludeReset, const FString& SpaceLabel = TEXT("\u9009\u62E9"), bool bIncludeDelete = false);
	void SetScreen(EPauseMenuScreen Screen);
	void SetPauseItem(int32 ItemIndex, bool bPlayHoverSound);
	void UpdatePauseItemVisuals();
	void SetOptionCategory(int32 CategoryIndex, bool bPlayHoverSound);
	void UpdateOptionCategoryVisuals();
	void SetSaveRecord(int32 RecordIndex, bool bPlayHoverSound);
	void UpdateSaveRecordVisuals();
	void RefreshSaveList();
	void DeleteSelectedSaveRecord();
	void SetConfirmChoice(EPauseConfirmChoice Choice);
	void ConfirmCurrentSelection();
	void SaveCurrentGame();
	void LoadSavedGame(int32 RecordIndex = 0);
	void ClosePauseMenu();
	void PlayConfirmClick() const;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> MainPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> OptionsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> SaveListPanel;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> SaveRecordList;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SaveDetailTitle;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SaveDetailLevel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SaveDetailTime;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SaveDetailPlayTime;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> ReturnMainConfirmPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> QuitConfirmPanel;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> ShortcutHint_Panel;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> PauseItemTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> PauseItemBorders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> OptionCategoryTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> OptionCategoryBorders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> SaveRecordTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> SaveRecordSubtexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> SaveRecordBorders;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> T_ReturnYes;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> T_ReturnNo;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> T_QuitYes;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> T_QuitNo;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> HoverSound;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> ConfirmClickSound;

	FString SaveSlotName = TEXT("Project_UI_Menu_SaveSlot");
	EPauseMenuScreen ActiveScreen = EPauseMenuScreen::Main;
	EPauseConfirmChoice ConfirmChoice = EPauseConfirmChoice::No;
	int32 SelectedPauseItemIndex = 0;
	int32 HoveredPauseItemIndex = INDEX_NONE;
	int32 SelectedOptionCategoryIndex = 0;
	int32 HoveredOptionCategoryIndex = INDEX_NONE;
	int32 SelectedSaveRecordIndex = 0;
	int32 HoveredSaveRecordIndex = INDEX_NONE;
	bool bSaveListIsSaving = true;
	bool bConfirmTargetsReturnMain = false;

	UFUNCTION()
	void HandlePauseOptionHovered();
	UFUNCTION()
	void HandlePausePhotoHovered();
	UFUNCTION()
	void HandlePauseExtrasHovered();
	UFUNCTION()
	void HandlePauseTutorialHovered();
	UFUNCTION()
	void HandlePauseSaveHovered();
	UFUNCTION()
	void HandlePauseLoadHovered();
	UFUNCTION()
	void HandlePauseRestartHovered();
	UFUNCTION()
	void HandlePauseReturnMainHovered();
	UFUNCTION()
	void HandlePauseQuitHovered();
	UFUNCTION()
	void HandlePauseItemClicked();
	UFUNCTION()
	void HandlePauseItemUnhovered();

	UFUNCTION()
	void HandleOptionCategory0Hovered();
	UFUNCTION()
	void HandleOptionCategory1Hovered();
	UFUNCTION()
	void HandleOptionCategory2Hovered();
	UFUNCTION()
	void HandleOptionCategory3Hovered();
	UFUNCTION()
	void HandleOptionCategory4Hovered();
	UFUNCTION()
	void HandleOptionCategory5Hovered();
	UFUNCTION()
	void HandleOptionCategory6Hovered();
	UFUNCTION()
	void HandleOptionCategory7Hovered();
	UFUNCTION()
	void HandleOptionCategory8Hovered();
	UFUNCTION()
	void HandleOptionCategory9Hovered();
	UFUNCTION()
	void HandleOptionCategoryClicked();
	UFUNCTION()
	void HandleOptionCategoryUnhovered();

	UFUNCTION()
	void HandleSaveRecord0Hovered();
	UFUNCTION()
	void HandleSaveRecord1Hovered();
	UFUNCTION()
	void HandleSaveRecord2Hovered();
	UFUNCTION()
	void HandleSaveRecord3Hovered();
	UFUNCTION()
	void HandleSaveRecord4Hovered();
	UFUNCTION()
	void HandleSaveRecord5Hovered();
	UFUNCTION()
	void HandleSaveRecordClicked();
	UFUNCTION()
	void HandleSaveRecordUnhovered();

	UFUNCTION()
	void HandleConfirmYesHovered();
	UFUNCTION()
	void HandleConfirmNoHovered();
	UFUNCTION()
	void HandleConfirmYesClicked();
	UFUNCTION()
	void HandleConfirmNoClicked();
};

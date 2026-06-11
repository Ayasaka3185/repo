#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UBorder;
class UButton;
class UCanvasPanel;
class UCheckBox;
class UComboBoxString;
class UHorizontalBox;
class UAudioComponent;
class USoundBase;
class UImage;
class UMediaPlayer;
class UMediaTexture;
class USlider;
class UTextBlock;
class UVerticalBox;
class UWidget;

UENUM(BlueprintType)
enum class EMainMenuItem : uint8
{
	None,
	Story,
	NoReturn,
	Options,
	Extras,
	Behind,
	Login,
	Quit
};

UENUM(BlueprintType)
enum class EMainMenuScreen : uint8
{
	Main,
	Story,
	Options,
	QuitConfirm
};

UENUM(BlueprintType)
enum class EQuitConfirmChoice : uint8
{
	Yes,
	No
};

UCLASS()
class PROJECT_UI_MENU_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void SetMenuFocus(EMainMenuItem Item);

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void ClearMenuHover();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void UpdateMenuVisuals();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OpenStoryMenu();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OpenOptionsMenu();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OpenQuitConfirm();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void CloseSubMenu();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void ResetOptionsToDefaults();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|State")
	EMainMenuItem CurrentMenuItem = EMainMenuItem::Story;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|State")
	EMainMenuItem HoveredMenuItem = EMainMenuItem::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Style")
	FLinearColor ActiveTextColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Style")
	FLinearColor InactiveTextColor = FLinearColor(0.78f, 0.78f, 0.78f, 0.55f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Style")
	FLinearColor HoverBoxColor = FLinearColor(1.f, 1.f, 1.f, 0.22f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Style")
	FLinearColor HiddenBoxColor = FLinearColor(1.f, 1.f, 1.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Audio")
	TObjectPtr<USoundBase> HoverSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Audio")
	TObjectPtr<USoundBase> ConfirmClickSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Audio")
	TObjectPtr<USoundBase> BackgroundLoopSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BackgroundLoopVolume = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Audio", meta = (ClampMin = "0.0"))
	float BackgroundFadeInDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Audio", meta = (ClampMin = "0.0"))
	float BackgroundFadeOutDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Transition", meta = (ClampMin = "0.01"))
	float SubMenuFadeInDuration = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Video")
	FString BackgroundVideoPath = TEXT("Movies/ingame.mp4");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Navigation")
	FName StoryLevelName = TEXT("/Engine/Maps/Templates/OpenWorld");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Text")
	FText StoryDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Text")
	FText NoReturnDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Text")
	FText OptionsDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Text")
	FText ExtrasDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Text")
	FText BehindDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Text")
	FText LoginDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Text")
	FText QuitDescription;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> B_Story;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> B_NoReturn;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> B_Options;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> B_Extras;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> B_Behind;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> B_Login;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> B_Quit;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Story;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_NoReturn;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Options;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Extras;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Behind;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Login;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Quit;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_Story;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_NoReturn;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_Options;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_Extras;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_Behind;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_Login;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_Quit;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> T_Description;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UImage> Background;

	UPROPERTY(BlueprintReadOnly, Category = "Main Menu|Widgets")
	TObjectPtr<UHorizontalBox> ShortcutHint_Panel;

private:
	void BindMenuClicks();
	void BindMenuButton(UButton* Button, EMainMenuItem Item);
	void SetItemVisual(EMainMenuItem Item, UBorder* Border, UTextBlock* TextBlock);
	FText GetDescriptionForItem(EMainMenuItem Item) const;
	void PlayConfirmClick() const;
	void BuildSubMenuLayer();
	void BuildStoryMenu();
	void BuildOptionsMenu();
	void BuildQuitConfirmMenu();
	void RebuildShortcutHints(bool bIncludeReset);
	void SetScreen(EMainMenuScreen Screen);
	void SetStoryItem(int32 ItemIndex, bool bPlayHoverSound);
	void UpdateStoryItemVisuals();
	void ConfirmStorySelection();
	void SetOptionCategory(int32 CategoryIndex, bool bPlayHoverSound);
	void UpdateOptionCategoryVisuals();
	void SetQuitChoice(EQuitConfirmChoice Choice);
	void ConfirmCurrentSelection();
	void SetWidgetCollapsed(UWidget* Widget, bool bCollapsed) const;
	void StartBackgroundLoop();
	void StopBackgroundLoop();
	void StartBackgroundVideo();
	void StopBackgroundVideo();

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> SubMenuLayer;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> StoryPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> OptionsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> QuitConfirmPanel;

	UPROPERTY(Transient)
	TObjectPtr<UButton> Btn_QuitYes;

	UPROPERTY(Transient)
	TObjectPtr<UButton> Btn_QuitNo;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> T_QuitYes;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> T_QuitNo;

	UPROPERTY(Transient)
	TObjectPtr<USlider> Slider_CameraSensitivity;

	UPROPERTY(Transient)
	TObjectPtr<USlider> Slider_HudOpacity;

	UPROPERTY(Transient)
	TObjectPtr<USlider> Slider_Brightness;

	UPROPERTY(Transient)
	TObjectPtr<USlider> Slider_MasterVolume;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> Toggle_Vibration;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> Toggle_Subtitles;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> Combo_Language;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> StoryItemTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> StoryItemButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> StoryItemBorders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> OptionCategoryTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> OptionCategoryButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> OptionCategoryBorders;

	EMainMenuScreen ActiveScreen = EMainMenuScreen::Main;
	EQuitConfirmChoice QuitChoice = EQuitConfirmChoice::No;
	int32 SelectedStoryItemIndex = 0;
	int32 HoveredStoryItemIndex = INDEX_NONE;
	int32 SelectedOptionCategoryIndex = 0;
	int32 HoveredOptionCategoryIndex = INDEX_NONE;
	bool bSubMenuFadeInActive = false;
	float SubMenuFadeElapsed = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BackgroundLoopAudioComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> BackgroundMediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> BackgroundMediaTexture;

	UFUNCTION()
	void HandleStoryHovered();

	UFUNCTION()
	void HandleNoReturnHovered();

	UFUNCTION()
	void HandleOptionsHovered();

	UFUNCTION()
	void HandleExtrasHovered();

	UFUNCTION()
	void HandleBehindHovered();

	UFUNCTION()
	void HandleLoginHovered();

	UFUNCTION()
	void HandleQuitHovered();

	UFUNCTION()
	void HandleMenuUnhovered();

	UFUNCTION()
	void HandleOptionsClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleMainMenuConfirmClicked();

	UFUNCTION()
	void HandleStoryContinueHovered();

	UFUNCTION()
	void HandleStoryNewGameHovered();

	UFUNCTION()
	void HandleStoryLoadGameHovered();

	UFUNCTION()
	void HandleStoryChapterHovered();

	UFUNCTION()
	void HandleStoryMovieHovered();

	UFUNCTION()
	void HandleStoryItemClicked();

	UFUNCTION()
	void HandleStoryItemUnhovered();

	UFUNCTION()
	void HandleOptionCategoryControlHovered();

	UFUNCTION()
	void HandleOptionCategoryHapticsHovered();

	UFUNCTION()
	void HandleOptionCategoryHudHovered();

	UFUNCTION()
	void HandleOptionCategorySubtitlesHovered();

	UFUNCTION()
	void HandleOptionCategoryDisplayHovered();

	UFUNCTION()
	void HandleOptionCategoryGraphicsHovered();

	UFUNCTION()
	void HandleOptionCategorySoundHovered();

	UFUNCTION()
	void HandleOptionCategoryLanguageHovered();

	UFUNCTION()
	void HandleOptionCategorySystemHovered();

	UFUNCTION()
	void HandleOptionCategoryAccessibilityHovered();

	UFUNCTION()
	void HandleOptionCategoryClicked();

	UFUNCTION()
	void HandleOptionCategoryUnhovered();

	UFUNCTION()
	void HandleQuitYesHovered();

	UFUNCTION()
	void HandleQuitNoHovered();

	UFUNCTION()
	void HandleQuitYesClicked();

	UFUNCTION()
	void HandleQuitNoClicked();
};

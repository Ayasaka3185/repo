#include "MainMenuWidget.h"

#include "Components/AudioComponent.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LoadingScreenWidget.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "Misc/Paths.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundWave.h"
#include "UObject/ConstructorHelpers.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StoryDescription = FText::FromString(TEXT("\u6E38\u73A9\u300A\u6E38\u620F\u540D\u79F0\u300B\u6545\u4E8B\u4E3B\u7EBF\u3002"));
	NoReturnDescription = FText::FromString(TEXT("\u5F00\u542F\u4E00\u573A Roguelike \u5F81\u9014\uFF0C\u5728\u4E00\u7CFB\u5217\u968F\u673A\u906D\u9047\u6218\u4E2D\u5B58\u6D3B\u4E0B\u6765\u3002"));
	OptionsDescription = FText::FromString(TEXT("\u8C03\u6574\u6E38\u620F\u8BBE\u7F6E\u3002"));
	ExtrasDescription = FText::FromString(TEXT("\u67E5\u770B\u9644\u52A0\u5185\u5BB9\u3002"));
	BehindDescription = FText::FromString(TEXT("\u67E5\u770B\u6E38\u620F\u5E55\u540E\u5185\u5BB9\u3002"));
	LoginDescription = FText::FromString(TEXT("\u767B\u5F55 PLAYSTATION \u8D26\u6237\u3002"));
	QuitDescription = FText::FromString(TEXT("\u9000\u51FA\u5230\u684C\u9762\u3002"));

	static ConstructorHelpers::FObjectFinder<USoundBase> HoverSoundFinder(TEXT("/Game/UI/Audio/short_click.short_click"));
	if (HoverSoundFinder.Succeeded())
	{
		HoverSound = HoverSoundFinder.Object;
		ConfirmClickSound = HoverSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> ConfirmClickSoundFinder(TEXT("/Game/UI/Audio/confirm_click.confirm_click"));
	if (ConfirmClickSoundFinder.Succeeded())
	{
		ConfirmClickSound = ConfirmClickSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> BackgroundLoopSoundFinder(TEXT("/Game/UI/Audio/sea_waves_loop.sea_waves_loop"));
	if (BackgroundLoopSoundFinder.Succeeded())
	{
		BackgroundLoopSound = BackgroundLoopSoundFinder.Object;
		if (USoundWave* BackgroundLoopWave = Cast<USoundWave>(BackgroundLoopSound))
		{
			BackgroundLoopWave->bLooping = true;
		}
	}
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindMenuButton(Btn_Story, EMainMenuItem::Story);
	BindMenuButton(Btn_NoReturn, EMainMenuItem::NoReturn);
	BindMenuButton(Btn_Options, EMainMenuItem::Options);
	BindMenuButton(Btn_Extras, EMainMenuItem::Extras);
	BindMenuButton(Btn_Behind, EMainMenuItem::Behind);
	BindMenuButton(Btn_Login, EMainMenuItem::Login);
	BindMenuButton(Btn_Quit, EMainMenuItem::Quit);
	BindMenuClicks();

	CurrentMenuItem = EMainMenuItem::Story;
	HoveredMenuItem = EMainMenuItem::None;
	ActiveScreen = EMainMenuScreen::Main;
	QuitChoice = EQuitConfirmChoice::No;
	UpdateMenuVisuals();
	SetScreen(EMainMenuScreen::Main);
	StartBackgroundVideo();
	StartBackgroundLoop();
	SetIsFocusable(true);
	SetKeyboardFocus();
}

TSharedRef<SWidget> UMainMenuWidget::RebuildWidget()
{
	BuildSubMenuLayer();
	return Super::RebuildWidget();
}

void UMainMenuWidget::NativeDestruct()
{
	StopBackgroundVideo();
	StopBackgroundLoop();
	Super::NativeDestruct();
}

void UMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bSubMenuFadeInActive || !SubMenuLayer)
	{
		return;
	}

	SubMenuFadeElapsed = FMath::Min(SubMenuFadeElapsed + InDeltaTime, SubMenuFadeInDuration);
	const float FadeAlpha = FMath::InterpEaseInOut(0.f, 1.f, SubMenuFadeElapsed / SubMenuFadeInDuration, 2.f);
	SubMenuLayer->SetRenderOpacity(FadeAlpha);
	if (SubMenuFadeElapsed >= SubMenuFadeInDuration)
	{
		bSubMenuFadeInActive = false;
		SubMenuLayer->SetRenderOpacity(1.f);
	}
}

void UMainMenuWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (ActiveScreen == EMainMenuScreen::Main)
	{
		ClearMenuHover();
	}
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape && ActiveScreen != EMainMenuScreen::Main)
	{
		CloseSubMenu();
		return FReply::Handled();
	}

	if (Key == EKeys::SpaceBar && ActiveScreen != EMainMenuScreen::Main)
	{
		ConfirmCurrentSelection();
		return FReply::Handled();
	}

	if (Key == EKeys::R && ActiveScreen == EMainMenuScreen::Options)
	{
		ResetOptionsToDefaults();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::SetMenuFocus(EMainMenuItem Item)
{
	if (Item == EMainMenuItem::None)
	{
		return;
	}

	StartBackgroundLoop();

	if (HoverSound && Item != HoveredMenuItem)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}

	CurrentMenuItem = Item;
	HoveredMenuItem = Item;
	UpdateMenuVisuals();
}

void UMainMenuWidget::ClearMenuHover()
{
	HoveredMenuItem = EMainMenuItem::None;
	UpdateMenuVisuals();
}

void UMainMenuWidget::UpdateMenuVisuals()
{
	SetItemVisual(EMainMenuItem::Story, B_Story, T_Story);
	SetItemVisual(EMainMenuItem::NoReturn, B_NoReturn, T_NoReturn);
	SetItemVisual(EMainMenuItem::Options, B_Options, T_Options);
	SetItemVisual(EMainMenuItem::Extras, B_Extras, T_Extras);
	SetItemVisual(EMainMenuItem::Behind, B_Behind, T_Behind);
	SetItemVisual(EMainMenuItem::Login, B_Login, T_Login);
	SetItemVisual(EMainMenuItem::Quit, B_Quit, T_Quit);

	if (T_Description)
	{
		T_Description->SetText(GetDescriptionForItem(CurrentMenuItem));
	}
}

void UMainMenuWidget::OpenStoryMenu()
{
	PlayConfirmClick();
	SetStoryItem(0, false);
	SetScreen(EMainMenuScreen::Story);
}

void UMainMenuWidget::OpenOptionsMenu()
{
	PlayConfirmClick();
	SetOptionCategory(0, false);
	SetScreen(EMainMenuScreen::Options);
}

void UMainMenuWidget::OpenQuitConfirm()
{
	PlayConfirmClick();
	SetQuitChoice(EQuitConfirmChoice::No);
	SetScreen(EMainMenuScreen::QuitConfirm);
}

void UMainMenuWidget::CloseSubMenu()
{
	PlayConfirmClick();
	SetScreen(EMainMenuScreen::Main);
}

void UMainMenuWidget::ResetOptionsToDefaults()
{
	PlayConfirmClick();
	if (Slider_CameraSensitivity)
	{
		Slider_CameraSensitivity->SetValue(0.5f);
	}
	if (Slider_HudOpacity)
	{
		Slider_HudOpacity->SetValue(0.85f);
	}
	if (Slider_Brightness)
	{
		Slider_Brightness->SetValue(0.55f);
	}
	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->SetValue(0.75f);
	}
	if (Toggle_Vibration)
	{
		Toggle_Vibration->SetCheckedState(ECheckBoxState::Checked);
	}
	if (Toggle_Subtitles)
	{
		Toggle_Subtitles->SetCheckedState(ECheckBoxState::Unchecked);
	}
	if (Combo_Language)
	{
		Combo_Language->SetSelectedOption(TEXT("\u7B80\u4F53\u4E2D\u6587"));
	}
}

void UMainMenuWidget::BindMenuClicks()
{
	const TArray<TObjectPtr<UButton>> ConfirmOnlyButtons = {
		Btn_NoReturn,
		Btn_Extras,
		Btn_Behind,
		Btn_Login
	};
	for (UButton* Button : ConfirmOnlyButtons)
	{
		if (Button)
		{
			Button->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleMainMenuConfirmClicked);
			Button->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleMainMenuConfirmClicked);
		}
	}

	if (Btn_Story)
	{
		Btn_Story->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleMainMenuConfirmClicked);
		Btn_Story->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleOptionsClicked);
		Btn_Story->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleQuitClicked);
		Btn_Story->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleMainMenuConfirmClicked);
	}

	if (Btn_Options)
	{
		Btn_Options->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleOptionsClicked);
		Btn_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleOptionsClicked);
	}

	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleQuitClicked);
		Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitClicked);
	}
}

void UMainMenuWidget::BindMenuButton(UButton* Button, EMainMenuItem Item)
{
	if (!Button)
	{
		return;
	}

	Button->OnUnhovered.RemoveDynamic(this, &UMainMenuWidget::HandleMenuUnhovered);
	Button->OnUnhovered.AddDynamic(this, &UMainMenuWidget::HandleMenuUnhovered);

	switch (Item)
	{
	case EMainMenuItem::Story:
		Button->OnHovered.RemoveDynamic(this, &UMainMenuWidget::HandleStoryHovered);
		Button->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleStoryHovered);
		break;
	case EMainMenuItem::NoReturn:
		Button->OnHovered.RemoveDynamic(this, &UMainMenuWidget::HandleNoReturnHovered);
		Button->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleNoReturnHovered);
		break;
	case EMainMenuItem::Options:
		Button->OnHovered.RemoveDynamic(this, &UMainMenuWidget::HandleOptionsHovered);
		Button->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionsHovered);
		break;
	case EMainMenuItem::Extras:
		Button->OnHovered.RemoveDynamic(this, &UMainMenuWidget::HandleExtrasHovered);
		Button->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleExtrasHovered);
		break;
	case EMainMenuItem::Behind:
		Button->OnHovered.RemoveDynamic(this, &UMainMenuWidget::HandleBehindHovered);
		Button->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleBehindHovered);
		break;
	case EMainMenuItem::Login:
		Button->OnHovered.RemoveDynamic(this, &UMainMenuWidget::HandleLoginHovered);
		Button->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleLoginHovered);
		break;
	case EMainMenuItem::Quit:
		Button->OnHovered.RemoveDynamic(this, &UMainMenuWidget::HandleQuitHovered);
		Button->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleQuitHovered);
		break;
	default:
		break;
	}
}

void UMainMenuWidget::SetItemVisual(EMainMenuItem Item, UBorder* Border, UTextBlock* TextBlock)
{
	if (TextBlock)
	{
		TextBlock->SetColorAndOpacity(FSlateColor(Item == CurrentMenuItem ? ActiveTextColor : InactiveTextColor));
	}

	if (Border)
	{
		Border->SetBrushColor(Item == HoveredMenuItem ? HoverBoxColor : HiddenBoxColor);
	}
}

void UMainMenuWidget::PlayConfirmClick() const
{
	if (ConfirmClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ConfirmClickSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
}

static UTextBlock* CreateMenuText(UWidgetTree* Tree, const FString& Text, float FontSize, const FLinearColor& Color)
{
	UTextBlock* TextBlock = Tree->ConstructWidget<UTextBlock>();
	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(ETextJustify::Left);
	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	return TextBlock;
}

static UBorder* CreateLine(UWidgetTree* Tree, float Alpha)
{
	UBorder* Line = Tree->ConstructWidget<UBorder>();
	Line->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, Alpha));
	Line->SetPadding(FMargin(0.f));
	return Line;
}

static UButton* CreateTextButton(UWidgetTree* Tree, const FString& Text, TObjectPtr<UTextBlock>& OutText)
{
	UButton* Button = Tree->ConstructWidget<UButton>();
	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
	ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.10f));
	ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.18f));
	Button->SetStyle(ButtonStyle);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	OutText = CreateMenuText(Tree, Text, 28.f, FLinearColor(0.65f, 0.65f, 0.65f, 1.f));
	Button->SetContent(OutText);
	return Button;
}

static void AddCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment = FVector2D::ZeroVector, int32 ZOrder = 0)
{
	UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
	Slot->SetAnchors(Anchors);
	Slot->SetOffsets(Offsets);
	Slot->SetAlignment(Alignment);
	Slot->SetZOrder(ZOrder);
}

void UMainMenuWidget::BuildSubMenuLayer()
{
	if (!WidgetTree || SubMenuLayer)
	{
		return;
	}

	UWidget* ExistingRoot = WidgetTree->RootWidget;
	if (!ExistingRoot)
	{
		return;
	}

	UOverlay* RuntimeRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Runtime_MenuRoot"));
	WidgetTree->RootWidget = RuntimeRoot;
	UOverlaySlot* ExistingSlot = RuntimeRoot->AddChildToOverlay(ExistingRoot);
	ExistingSlot->SetHorizontalAlignment(HAlign_Fill);
	ExistingSlot->SetVerticalAlignment(VAlign_Fill);

	SubMenuLayer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SubMenu_Layer"));
	UOverlaySlot* LayerSlot = RuntimeRoot->AddChildToOverlay(SubMenuLayer);
	LayerSlot->SetHorizontalAlignment(HAlign_Fill);
	LayerSlot->SetVerticalAlignment(VAlign_Fill);

	UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("SubMenu_BackgroundBlur"));
	Blur->SetBlurStrength(8.f);
	Blur->SetBlurRadius(14);
	AddCanvasChild(SubMenuLayer, Blur, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 0);

	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SubMenu_DimMask"));
	Dim->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.72f));
	AddCanvasChild(SubMenuLayer, Dim, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 1);

	BuildStoryMenu();
	BuildOptionsMenu();
	BuildQuitConfirmMenu();
	RebuildShortcutHints(false);
}

void UMainMenuWidget::BuildStoryMenu()
{
	StoryItemTexts.Empty();
	StoryItemButtons.Empty();
	StoryItemBorders.Empty();

	StoryPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Story_Panel"));
	AddCanvasChild(SubMenuLayer, StoryPanel, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 2);

	UTextBlock* Title = CreateMenuText(WidgetTree, TEXT("\u5267\u60C5"), 30.f, FLinearColor::White);
	AddCanvasChild(StoryPanel, Title, FAnchors(0.f, 0.f), FMargin(74.f, 62.f, 140.f, 44.f));

	UBorder* TopLine = CreateLine(WidgetTree, 0.10f);
	AddCanvasChild(StoryPanel, TopLine, FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(140.f, 80.f, 70.f, 1.f));

	UVerticalBox* StoryList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Story_ItemList"));
	const TArray<FString> StoryItems = {
		TEXT("\u7EE7\u7EED"),
		TEXT("\u65B0\u6E38\u620F"),
		TEXT("\u8F7D\u5165\u6E38\u620F"),
		TEXT("\u7AE0\u8282"),
		TEXT("\u52A8\u753B")
	};

	for (int32 Index = 0; Index < StoryItems.Num(); ++Index)
	{
		UButton* StoryButton = WidgetTree->ConstructWidget<UButton>();
		FButtonStyle StoryButtonStyle = StoryButton->GetStyle();
		StoryButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		StoryButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		StoryButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
		StoryButton->SetStyle(StoryButtonStyle);
		StoryButton->SetBackgroundColor(FLinearColor::Transparent);

		UBorder* StoryBorder = WidgetTree->ConstructWidget<UBorder>();
		StoryBorder->SetPadding(FMargin(36.f, 11.f, 36.f, 12.f));
		StoryBorder->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));

		UTextBlock* StoryText = CreateMenuText(WidgetTree, StoryItems[Index], 24.f, FLinearColor(0.34f, 0.34f, 0.34f, 1.f));
		StoryText->SetJustification(ETextJustify::Left);
		StoryBorder->SetContent(StoryText);
		StoryButton->SetContent(StoryBorder);
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(StoryBorder->Slot))
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Left);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
		}

		StoryItemTexts.Add(StoryText);
		StoryItemButtons.Add(StoryButton);
		StoryItemBorders.Add(StoryBorder);

		switch (Index)
		{
		case 0:
			StoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleStoryContinueHovered);
			break;
		case 1:
			StoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleStoryNewGameHovered);
			break;
		case 2:
			StoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleStoryLoadGameHovered);
			break;
		case 3:
			StoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleStoryChapterHovered);
			break;
		case 4:
			StoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleStoryMovieHovered);
			break;
		default:
			break;
		}
		StoryButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleStoryItemClicked);
		StoryButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::HandleStoryItemUnhovered);

		UVerticalBoxSlot* StorySlot = StoryList->AddChildToVerticalBox(StoryButton);
		StorySlot->SetHorizontalAlignment(HAlign_Left);
		StorySlot->SetPadding(FMargin(0.f, 0.f, 0.f, Index == 0 ? 4.f : 20.f));

		if (Index == 0)
		{
			UTextBlock* Subtitle = CreateMenuText(WidgetTree, TEXT("\u5C0F\u6807\u98981"), 22.f, FLinearColor(0.30f, 0.30f, 0.30f, 1.f));
			UVerticalBoxSlot* SubtitleSlot = StoryList->AddChildToVerticalBox(Subtitle);
			SubtitleSlot->SetHorizontalAlignment(HAlign_Left);
			SubtitleSlot->SetPadding(FMargin(58.f, 0.f, 0.f, 50.f));
		}
	}

	AddCanvasChild(StoryPanel, StoryList, FAnchors(0.f, 0.f), FMargin(80.f, 232.f, 260.f, 560.f));

	UBorder* BottomLine = CreateLine(WidgetTree, 0.10f);
	AddCanvasChild(StoryPanel, BottomLine, FAnchors(0.f, 1.f, 1.f, 1.f), FMargin(70.f, -118.f, 70.f, 1.f));
	SetStoryItem(0, false);
}

void UMainMenuWidget::BuildOptionsMenu()
{
	OptionsPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Options_Panel"));
	AddCanvasChild(SubMenuLayer, OptionsPanel, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 2);

	UTextBlock* Title = CreateMenuText(WidgetTree, TEXT("\u9009\u9879"), 30.f, FLinearColor::White);
	AddCanvasChild(OptionsPanel, Title, FAnchors(0.f, 0.f), FMargin(74.f, 62.f, 140.f, 44.f));

	UBorder* TopLine = CreateLine(WidgetTree, 0.10f);
	AddCanvasChild(OptionsPanel, TopLine, FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(172.f, 80.f, 70.f, 1.f));

	UVerticalBox* Categories = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Options_CategoryList"));
	const TArray<FString> CategoryTexts = {
		TEXT("\u63A7\u5236"),
		TEXT("\u89E6\u89C9\u53CD\u9988"),
		TEXT("HUD"),
		TEXT("\u5B57\u5E55"),
		TEXT("\u663E\u793A"),
		TEXT("\u56FE\u50CF"),
		TEXT("\u58F0\u97F3"),
		TEXT("\u8BED\u8A00"),
		TEXT("\u7CFB\u7EDF"),
		TEXT("\u8F85\u52A9\u529F\u80FD")
	};
	for (int32 Index = 0; Index < CategoryTexts.Num(); ++Index)
	{
		UButton* CategoryButton = WidgetTree->ConstructWidget<UButton>();
		FButtonStyle CategoryButtonStyle = CategoryButton->GetStyle();
		CategoryButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		CategoryButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		CategoryButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
		CategoryButton->SetStyle(CategoryButtonStyle);
		CategoryButton->SetBackgroundColor(FLinearColor::Transparent);

		UBorder* CategoryBorder = WidgetTree->ConstructWidget<UBorder>();
		CategoryBorder->SetPadding(FMargin(36.f, 11.f, 36.f, 12.f));
		CategoryBorder->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));

		UTextBlock* CategoryText = CreateMenuText(WidgetTree, CategoryTexts[Index], 22.f, FLinearColor(0.22f, 0.22f, 0.22f, 1.f));
		CategoryText->SetJustification(ETextJustify::Left);
		CategoryBorder->SetContent(CategoryText);
		CategoryButton->SetContent(CategoryBorder);
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(CategoryBorder->Slot))
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Left);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
		}

		OptionCategoryTexts.Add(CategoryText);
		OptionCategoryButtons.Add(CategoryButton);
		OptionCategoryBorders.Add(CategoryBorder);

		switch (Index)
		{
		case 0:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryControlHovered);
			break;
		case 1:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryHapticsHovered);
			break;
		case 2:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryHudHovered);
			break;
		case 3:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategorySubtitlesHovered);
			break;
		case 4:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryDisplayHovered);
			break;
		case 5:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryGraphicsHovered);
			break;
		case 6:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategorySoundHovered);
			break;
		case 7:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryLanguageHovered);
			break;
		case 8:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategorySystemHovered);
			break;
		case 9:
			CategoryButton->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryAccessibilityHovered);
			break;
		default:
			break;
		}
		CategoryButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryClicked);
		CategoryButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::HandleOptionCategoryUnhovered);

		UVerticalBoxSlot* CategorySlot = Categories->AddChildToVerticalBox(CategoryButton);
		CategorySlot->SetHorizontalAlignment(HAlign_Left);
		CategorySlot->SetPadding(FMargin(0.f, 0.f, 0.f, 18.f));
	}
	AddCanvasChild(OptionsPanel, Categories, FAnchors(0.f, 0.f), FMargin(100.f, 150.f, 220.f, 620.f));

	UBorder* BottomLine = CreateLine(WidgetTree, 0.10f);
	AddCanvasChild(OptionsPanel, BottomLine, FAnchors(0.f, 1.f, 1.f, 1.f), FMargin(70.f, -118.f, 70.f, 1.f));
	SetOptionCategory(0, false);
}

void UMainMenuWidget::BuildQuitConfirmMenu()
{
	QuitConfirmPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("QuitConfirm_Panel"));
	AddCanvasChild(SubMenuLayer, QuitConfirmPanel, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 3);

	UTextBlock* Title = CreateMenuText(WidgetTree, TEXT("\u9000\u51FA\u5230\u684C\u9762"), 28.f, FLinearColor::White);
	AddCanvasChild(QuitConfirmPanel, Title, FAnchors(0.f, 0.f), FMargin(484.f, 420.f, 260.f, 40.f));

	UBorder* TopLine = CreateLine(WidgetTree, 0.10f);
	AddCanvasChild(QuitConfirmPanel, TopLine, FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(665.f, 436.f, 485.f, 1.f));

	UTextBlock* Question = CreateMenuText(WidgetTree, TEXT("\u662F\u5426\u8981\u9000\u51FA\u5230\u684C\u9762\uFF1F"), 26.f, FLinearColor(0.48f, 0.48f, 0.48f, 1.f));
	AddCanvasChild(QuitConfirmPanel, Question, FAnchors(0.f, 0.f), FMargin(516.f, 520.f, 520.f, 42.f));

	UBorder* BottomLine = CreateLine(WidgetTree, 0.10f);
	AddCanvasChild(QuitConfirmPanel, BottomLine, FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(484.f, 650.f, 485.f, 1.f));

	Btn_QuitYes = CreateTextButton(WidgetTree, TEXT("\u662F"), T_QuitYes);
	Btn_QuitNo = CreateTextButton(WidgetTree, TEXT("\u5426"), T_QuitNo);
	AddCanvasChild(QuitConfirmPanel, Btn_QuitYes, FAnchors(1.f, 0.f), FMargin(-578.f, 667.f, 70.f, 48.f));
	AddCanvasChild(QuitConfirmPanel, Btn_QuitNo, FAnchors(1.f, 0.f), FMargin(-504.f, 667.f, 70.f, 48.f));

	Btn_QuitYes->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleQuitYesHovered);
	Btn_QuitNo->OnHovered.AddDynamic(this, &UMainMenuWidget::HandleQuitNoHovered);
	Btn_QuitYes->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitYesClicked);
	Btn_QuitNo->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitNoClicked);
	SetQuitChoice(EQuitConfirmChoice::No);
}

void UMainMenuWidget::RebuildShortcutHints(bool bIncludeReset)
{
	if (ShortcutHint_Panel && ShortcutHint_Panel->GetParent())
	{
		ShortcutHint_Panel->RemoveFromParent();
	}

	ShortcutHint_Panel = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ShortcutHint_Panel"));
	AddCanvasChild(SubMenuLayer, ShortcutHint_Panel, FAnchors(1.f, 1.f), FMargin(-540.f, -100.f, 60.f, 44.f), FVector2D::ZeroVector, 10);

	auto AddHint = [this](const FString& Key, const FString& Label)
	{
		UBorder* HintFrame = WidgetTree->ConstructWidget<UBorder>();
		HintFrame->SetBrushColor(FLinearColor(0.42f, 0.42f, 0.42f, 0.72f));
		HintFrame->SetPadding(FMargin(1.f));

		UBorder* HintInner = WidgetTree->ConstructWidget<UBorder>();
		HintInner->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.64f));
		HintInner->SetPadding(FMargin(8.f, 4.f));

		UHorizontalBox* Hint = WidgetTree->ConstructWidget<UHorizontalBox>();
		UBorder* KeyBox = WidgetTree->ConstructWidget<UBorder>();
		KeyBox->SetBrushColor(FLinearColor::White);
		KeyBox->SetPadding(FMargin(5.f, 2.f));
		KeyBox->SetContent(CreateMenuText(WidgetTree, Key, 9.f, FLinearColor::Black));
		Hint->AddChildToHorizontalBox(KeyBox);
		UTextBlock* LabelText = CreateMenuText(WidgetTree, Label, 20.f, FLinearColor(0.78f, 0.78f, 0.78f, 1.f));
		UHorizontalBoxSlot* LabelSlot = Hint->AddChildToHorizontalBox(LabelText);
		LabelSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));

		HintInner->SetContent(Hint);
		HintFrame->SetContent(HintInner);
		UHorizontalBoxSlot* HintSlot = ShortcutHint_Panel->AddChildToHorizontalBox(HintFrame);
		HintSlot->SetPadding(FMargin(14.f, 0.f, 0.f, 0.f));
	};

	AddHint(TEXT("SPACE"), TEXT("\u9009\u62E9"));
	if (bIncludeReset)
	{
		AddHint(TEXT("R"), TEXT("\u91CD\u7F6E\u4E3A\u9ED8\u8BA4\u8BBE\u7F6E"));
	}
	AddHint(TEXT("ESC"), TEXT("\u8FD4\u56DE"));
}

void UMainMenuWidget::SetScreen(EMainMenuScreen Screen)
{
	const bool bWasMain = ActiveScreen == EMainMenuScreen::Main;
	ActiveScreen = Screen;
	const bool bMain = Screen == EMainMenuScreen::Main;
	SetWidgetCollapsed(SubMenuLayer, bMain);
	SetWidgetCollapsed(StoryPanel, Screen != EMainMenuScreen::Story);
	SetWidgetCollapsed(OptionsPanel, Screen != EMainMenuScreen::Options);
	SetWidgetCollapsed(QuitConfirmPanel, Screen != EMainMenuScreen::QuitConfirm);

	if (!bMain)
	{
		if (bWasMain && SubMenuLayer)
		{
			SubMenuFadeElapsed = 0.f;
			bSubMenuFadeInActive = true;
			SubMenuLayer->SetRenderOpacity(0.f);
		}
		RebuildShortcutHints(Screen == EMainMenuScreen::Options);
		SetKeyboardFocus();
	}
	else if (SubMenuLayer)
	{
		bSubMenuFadeInActive = false;
		SubMenuLayer->SetRenderOpacity(1.f);
	}
}

void UMainMenuWidget::SetStoryItem(int32 ItemIndex, bool bPlayHoverSound)
{
	if (!StoryItemTexts.IsValidIndex(ItemIndex))
	{
		return;
	}

	if (bPlayHoverSound && HoverSound && ItemIndex != SelectedStoryItemIndex)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}

	SelectedStoryItemIndex = ItemIndex;
	HoveredStoryItemIndex = bPlayHoverSound ? ItemIndex : INDEX_NONE;
	UpdateStoryItemVisuals();
}

void UMainMenuWidget::UpdateStoryItemVisuals()
{
	for (int32 Index = 0; Index < StoryItemTexts.Num(); ++Index)
	{
		const bool bSelected = Index == SelectedStoryItemIndex;
		if (UTextBlock* StoryText = StoryItemTexts[Index])
		{
			StoryText->SetColorAndOpacity(FSlateColor(bSelected ? FLinearColor::White : FLinearColor(0.34f, 0.34f, 0.34f, 1.f)));
		}
		if (StoryItemBorders.IsValidIndex(Index))
		{
			if (UBorder* StoryBorder = StoryItemBorders[Index])
			{
				StoryBorder->SetBrushColor(Index == HoveredStoryItemIndex ? FLinearColor(1.f, 1.f, 1.f, 0.18f) : FLinearColor(1.f, 1.f, 1.f, 0.f));
			}
		}
	}
}

void UMainMenuWidget::ConfirmStorySelection()
{
	PlayConfirmClick();
	if (SelectedStoryItemIndex == 0 || SelectedStoryItemIndex == 1)
	{
		StopBackgroundVideo();
		StopBackgroundLoop();
		if (APlayerController* PC = GetOwningPlayer())
		{
			ULoadingScreenWidget* LoadingScreen = CreateWidget<ULoadingScreenWidget>(PC, ULoadingScreenWidget::StaticClass());
			if (LoadingScreen)
			{
				LoadingScreen->AddToViewport(100);
				LoadingScreen->BeginLoadingToLevel(StoryLevelName);
				return;
			}
		}
		UGameplayStatics::OpenLevel(this, StoryLevelName, true, TEXT("game=/Script/Project_UI_Menu.ProjectUIGameMode"));
	}
}

void UMainMenuWidget::SetOptionCategory(int32 CategoryIndex, bool bPlayHoverSound)
{
	if (!OptionCategoryTexts.IsValidIndex(CategoryIndex))
	{
		return;
	}

	if (bPlayHoverSound && HoverSound && CategoryIndex != SelectedOptionCategoryIndex)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}

	SelectedOptionCategoryIndex = CategoryIndex;
	HoveredOptionCategoryIndex = bPlayHoverSound ? CategoryIndex : INDEX_NONE;
	UpdateOptionCategoryVisuals();
}

void UMainMenuWidget::UpdateOptionCategoryVisuals()
{
	for (int32 Index = 0; Index < OptionCategoryTexts.Num(); ++Index)
	{
		const bool bSelected = Index == SelectedOptionCategoryIndex;
		if (UTextBlock* CategoryText = OptionCategoryTexts[Index])
		{
			CategoryText->SetColorAndOpacity(FSlateColor(bSelected ? FLinearColor::White : FLinearColor(0.22f, 0.22f, 0.22f, 1.f)));
		}
		if (OptionCategoryBorders.IsValidIndex(Index))
		{
			if (UBorder* CategoryBorder = OptionCategoryBorders[Index])
			{
				CategoryBorder->SetBrushColor(Index == HoveredOptionCategoryIndex ? FLinearColor(1.f, 1.f, 1.f, 0.18f) : FLinearColor(1.f, 1.f, 1.f, 0.f));
			}
		}
	}
}

void UMainMenuWidget::SetQuitChoice(EQuitConfirmChoice Choice)
{
	QuitChoice = Choice;
	if (T_QuitYes)
	{
		T_QuitYes->SetColorAndOpacity(FSlateColor(Choice == EQuitConfirmChoice::Yes ? FLinearColor::White : FLinearColor(0.48f, 0.48f, 0.48f, 1.f)));
	}
	if (T_QuitNo)
	{
		T_QuitNo->SetColorAndOpacity(FSlateColor(Choice == EQuitConfirmChoice::No ? FLinearColor::White : FLinearColor(0.48f, 0.48f, 0.48f, 1.f)));
	}
}

void UMainMenuWidget::ConfirmCurrentSelection()
{
	if (ActiveScreen == EMainMenuScreen::Story)
	{
		ConfirmStorySelection();
	}
	else if (ActiveScreen == EMainMenuScreen::Options)
	{
		PlayConfirmClick();
	}
	else if (ActiveScreen == EMainMenuScreen::QuitConfirm)
	{
		if (QuitChoice == EQuitConfirmChoice::Yes)
		{
			PlayConfirmClick();
			UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
		}
		else
		{
			CloseSubMenu();
		}
	}
}

void UMainMenuWidget::SetWidgetCollapsed(UWidget* Widget, bool bCollapsed) const
{
	if (Widget)
	{
		Widget->SetVisibility(bCollapsed ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

FText UMainMenuWidget::GetDescriptionForItem(EMainMenuItem Item) const
{
	switch (Item)
	{
	case EMainMenuItem::NoReturn:
		return NoReturnDescription;
	case EMainMenuItem::Options:
		return OptionsDescription;
	case EMainMenuItem::Extras:
		return ExtrasDescription;
	case EMainMenuItem::Behind:
		return BehindDescription;
	case EMainMenuItem::Login:
		return LoginDescription;
	case EMainMenuItem::Quit:
		return QuitDescription;
	case EMainMenuItem::Story:
	case EMainMenuItem::None:
	default:
		return StoryDescription;
	}
}

void UMainMenuWidget::StartBackgroundLoop()
{
	if (!BackgroundLoopSound || BackgroundLoopAudioComponent)
	{
		return;
	}

	if (USoundWave* BackgroundLoopWave = Cast<USoundWave>(BackgroundLoopSound))
	{
		BackgroundLoopWave->bLooping = true;
	}

	BackgroundLoopAudioComponent = UGameplayStatics::SpawnSound2D(this, BackgroundLoopSound, BackgroundLoopVolume, 1.f, 0.f, nullptr, true, false);
	if (BackgroundLoopAudioComponent)
	{
		BackgroundLoopAudioComponent->bAutoDestroy = false;
		UE_LOG(LogTemp, Display, TEXT("Main menu background loop started: %s"), *BackgroundLoopSound->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Main menu background loop failed to start."));
	}
}

void UMainMenuWidget::StopBackgroundLoop()
{
	if (!BackgroundLoopAudioComponent)
	{
		return;
	}

	BackgroundLoopAudioComponent->FadeOut(BackgroundFadeOutDuration, 0.f);
	BackgroundLoopAudioComponent = nullptr;
}

void UMainMenuWidget::StartBackgroundVideo()
{
	if (!Background || BackgroundMediaPlayer)
	{
		return;
	}

	const FString VideoPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), BackgroundVideoPath));
	if (!FPaths::FileExists(VideoPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Main menu background video missing: %s"), *VideoPath);
		return;
	}

	BackgroundMediaPlayer = NewObject<UMediaPlayer>(this);
	BackgroundMediaTexture = NewObject<UMediaTexture>(this);

	if (!BackgroundMediaPlayer || !BackgroundMediaTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("Main menu background video objects failed to initialize."));
		return;
	}

	BackgroundMediaPlayer->SetLooping(true);
	BackgroundMediaTexture->SetMediaPlayer(BackgroundMediaPlayer);
	BackgroundMediaTexture->UpdateResource();

	FSlateBrush VideoBrush = Background->GetBrush();
	VideoBrush.SetResourceObject(BackgroundMediaTexture);
	Background->SetBrush(VideoBrush);

	if (BackgroundMediaPlayer->OpenFile(VideoPath))
	{
		BackgroundMediaPlayer->Play();
		UE_LOG(LogTemp, Display, TEXT("Main menu background video started: %s"), *VideoPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Main menu background video failed to open: %s"), *VideoPath);
	}
}

void UMainMenuWidget::StopBackgroundVideo()
{
	if (BackgroundMediaPlayer)
	{
		BackgroundMediaPlayer->Close();
	}

	BackgroundMediaTexture = nullptr;
	BackgroundMediaPlayer = nullptr;
}

void UMainMenuWidget::HandleStoryHovered()
{
	SetMenuFocus(EMainMenuItem::Story);
}

void UMainMenuWidget::HandleNoReturnHovered()
{
	SetMenuFocus(EMainMenuItem::NoReturn);
}

void UMainMenuWidget::HandleOptionsHovered()
{
	SetMenuFocus(EMainMenuItem::Options);
}

void UMainMenuWidget::HandleExtrasHovered()
{
	SetMenuFocus(EMainMenuItem::Extras);
}

void UMainMenuWidget::HandleBehindHovered()
{
	SetMenuFocus(EMainMenuItem::Behind);
}

void UMainMenuWidget::HandleLoginHovered()
{
	SetMenuFocus(EMainMenuItem::Login);
}

void UMainMenuWidget::HandleQuitHovered()
{
	SetMenuFocus(EMainMenuItem::Quit);
}

void UMainMenuWidget::HandleMenuUnhovered()
{
	if (ActiveScreen == EMainMenuScreen::Main)
	{
		ClearMenuHover();
	}
}

void UMainMenuWidget::HandleOptionsClicked()
{
	OpenOptionsMenu();
}

void UMainMenuWidget::HandleQuitClicked()
{
	OpenQuitConfirm();
}

void UMainMenuWidget::HandleMainMenuConfirmClicked()
{
	if (CurrentMenuItem == EMainMenuItem::Story)
	{
		OpenStoryMenu();
	}
	else
	{
		PlayConfirmClick();
	}
}

void UMainMenuWidget::HandleStoryContinueHovered()
{
	SetStoryItem(0, true);
}

void UMainMenuWidget::HandleStoryNewGameHovered()
{
	SetStoryItem(1, true);
}

void UMainMenuWidget::HandleStoryLoadGameHovered()
{
	SetStoryItem(2, true);
}

void UMainMenuWidget::HandleStoryChapterHovered()
{
	SetStoryItem(3, true);
}

void UMainMenuWidget::HandleStoryMovieHovered()
{
	SetStoryItem(4, true);
}

void UMainMenuWidget::HandleStoryItemClicked()
{
	ConfirmStorySelection();
}

void UMainMenuWidget::HandleStoryItemUnhovered()
{
	HoveredStoryItemIndex = INDEX_NONE;
	UpdateStoryItemVisuals();
}

void UMainMenuWidget::HandleOptionCategoryControlHovered()
{
	SetOptionCategory(0, true);
}

void UMainMenuWidget::HandleOptionCategoryHapticsHovered()
{
	SetOptionCategory(1, true);
}

void UMainMenuWidget::HandleOptionCategoryHudHovered()
{
	SetOptionCategory(2, true);
}

void UMainMenuWidget::HandleOptionCategorySubtitlesHovered()
{
	SetOptionCategory(3, true);
}

void UMainMenuWidget::HandleOptionCategoryDisplayHovered()
{
	SetOptionCategory(4, true);
}

void UMainMenuWidget::HandleOptionCategoryGraphicsHovered()
{
	SetOptionCategory(5, true);
}

void UMainMenuWidget::HandleOptionCategorySoundHovered()
{
	SetOptionCategory(6, true);
}

void UMainMenuWidget::HandleOptionCategoryLanguageHovered()
{
	SetOptionCategory(7, true);
}

void UMainMenuWidget::HandleOptionCategorySystemHovered()
{
	SetOptionCategory(8, true);
}

void UMainMenuWidget::HandleOptionCategoryAccessibilityHovered()
{
	SetOptionCategory(9, true);
}

void UMainMenuWidget::HandleOptionCategoryClicked()
{
	PlayConfirmClick();
}

void UMainMenuWidget::HandleOptionCategoryUnhovered()
{
	HoveredOptionCategoryIndex = INDEX_NONE;
	UpdateOptionCategoryVisuals();
}

void UMainMenuWidget::HandleQuitYesHovered()
{
	if (HoverSound && QuitChoice != EQuitConfirmChoice::Yes)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	SetQuitChoice(EQuitConfirmChoice::Yes);
}

void UMainMenuWidget::HandleQuitNoHovered()
{
	if (HoverSound && QuitChoice != EQuitConfirmChoice::No)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	SetQuitChoice(EQuitConfirmChoice::No);
}

void UMainMenuWidget::HandleQuitYesClicked()
{
	SetQuitChoice(EQuitConfirmChoice::Yes);
	ConfirmCurrentSelection();
}

void UMainMenuWidget::HandleQuitNoClicked()
{
	SetQuitChoice(EQuitConfirmChoice::No);
	PlayConfirmClick();
	CloseSubMenu();
}

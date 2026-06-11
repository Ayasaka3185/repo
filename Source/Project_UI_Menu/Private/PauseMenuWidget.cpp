#include "PauseMenuWidget.h"

#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundBase.h"
#include "ProjectUISaveGame.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
UTextBlock* CreatePauseText(UWidgetTree* Tree, const FString& Text, float FontSize, const FLinearColor& Color)
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

UBorder* CreatePauseLine(UWidgetTree* Tree, float Alpha)
{
	UBorder* Line = Tree->ConstructWidget<UBorder>();
	Line->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, Alpha));
	Line->SetPadding(FMargin(0.f));
	return Line;
}

void AddPauseCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment = FVector2D::ZeroVector, int32 ZOrder = 0)
{
	UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
	Slot->SetAnchors(Anchors);
	Slot->SetOffsets(Offsets);
	Slot->SetAlignment(Alignment);
	Slot->SetZOrder(ZOrder);
}

UButton* CreatePauseTextButton(UWidgetTree* Tree, const FString& Text, TObjectPtr<UTextBlock>& OutText)
{
	UButton* Button = Tree->ConstructWidget<UButton>();
	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
	ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
	ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
	Button->SetStyle(ButtonStyle);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	OutText = CreatePauseText(Tree, Text, 28.f, FLinearColor(0.48f, 0.48f, 0.48f, 1.f));
	Button->SetContent(OutText);
	return Button;
}
}

UPauseMenuWidget::UPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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
}

TSharedRef<SWidget> UPauseMenuWidget::RebuildWidget()
{
	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PauseMenu_Root"));
	WidgetTree->RootWidget = RootCanvas;

	UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("PauseMenu_BackgroundBlur"));
	Blur->SetBlurStrength(8.f);
	Blur->SetBlurRadius(14);
	AddPauseCanvasChild(RootCanvas, Blur, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 0);

	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PauseMenu_DimMask"));
	Dim->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.58f));
	AddPauseCanvasChild(RootCanvas, Dim, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 1);

	BuildMainMenu();
	BuildOptionsMenu();
	BuildSaveListMenu();
	BuildConfirmMenu(ReturnMainConfirmPanel, TEXT("Pause_ReturnMainConfirm"), TEXT("\u8FD4\u56DE\u4E3B\u83DC\u5355"), TEXT("\u662F\u5426\u8FD4\u56DE\u4E3B\u83DC\u5355\uFF1F"), true);
	BuildConfirmMenu(QuitConfirmPanel, TEXT("Pause_QuitConfirm"), TEXT("\u9000\u51FA\u5230\u684C\u9762"), TEXT("\u662F\u5426\u8981\u9000\u51FA\u5230\u684C\u9762\uFF1F"), false);
	RebuildShortcutHints(false);
	SetScreen(EPauseMenuScreen::Main);
	SetIsFocusable(true);
	SetKeyboardFocus();

	return Super::RebuildWidget();
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape)
	{
		if (ActiveScreen == EPauseMenuScreen::Main)
		{
			ClosePauseMenu();
		}
		else
		{
			PlayConfirmClick();
			SetScreen(EPauseMenuScreen::Main);
		}
		return FReply::Handled();
	}

	if (Key == EKeys::SpaceBar)
	{
		ConfirmCurrentSelection();
		return FReply::Handled();
	}

	if (Key == EKeys::R && ActiveScreen == EPauseMenuScreen::SaveList)
	{
		DeleteSelectedSaveRecord();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenuWidget::BuildMainMenu()
{
	MainPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Pause_MainPanel"));
	AddPauseCanvasChild(RootCanvas, MainPanel, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 2);

	UVerticalBox* Items = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Pause_ItemList"));
	const TArray<FString> ItemTexts = {
		TEXT("\u9009\u9879"),
		TEXT("\u76F8\u7247\u6A21\u5F0F"),
		TEXT("\u9644\u52A0\u5185\u5BB9"),
		TEXT("\u6559\u7A0B"),
		TEXT("\u4FDD\u5B58\u6E38\u620F"),
		TEXT("\u8F7D\u5165\u6E38\u620F"),
		TEXT("\u4ECE\u8BB0\u5F55\u70B9\u91CD\u65B0\u5F00\u59CB    \u5C11\u4E8E 1 \u5206\u949F"),
		TEXT("\u8FD4\u56DE\u4E3B\u83DC\u5355"),
		TEXT("\u9000\u51FA\u5230\u684C\u9762")
	};

	for (int32 Index = 0; Index < ItemTexts.Num(); ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>();
		FButtonStyle ButtonStyle = Button->GetStyle();
		ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
		Button->SetStyle(ButtonStyle);
		Button->SetBackgroundColor(FLinearColor::Transparent);

		UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
		Border->SetPadding(FMargin(36.f, 11.f, 36.f, 12.f));
		Border->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		UTextBlock* Text = CreatePauseText(WidgetTree, ItemTexts[Index], 23.f, FLinearColor(0.34f, 0.34f, 0.34f, 1.f));
		Border->SetContent(Text);
		Button->SetContent(Border);
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Border->Slot))
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Left);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
		}

		PauseItemTexts.Add(Text);
		PauseItemBorders.Add(Border);

		switch (Index)
		{
		case 0: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseOptionHovered); break;
		case 1: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePausePhotoHovered); break;
		case 2: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseExtrasHovered); break;
		case 3: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseTutorialHovered); break;
		case 4: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseSaveHovered); break;
		case 5: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseLoadHovered); break;
		case 6: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseRestartHovered); break;
		case 7: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseReturnMainHovered); break;
		case 8: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseQuitHovered); break;
		default: break;
		}
		Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandlePauseItemClicked);
		Button->OnUnhovered.AddDynamic(this, &UPauseMenuWidget::HandlePauseItemUnhovered);

		UVerticalBoxSlot* ItemSlot = Items->AddChildToVerticalBox(Button);
		ItemSlot->SetHorizontalAlignment(HAlign_Left);
		ItemSlot->SetPadding(FMargin(0.f, 0.f, 0.f, Index == 3 || Index == 6 ? 28.f : 8.f));
	}

	AddPauseCanvasChild(MainPanel, Items, FAnchors(0.f, 0.f), FMargin(80.f, 190.f, 560.f, 720.f));
	SetPauseItem(0, false);
}

void UPauseMenuWidget::BuildOptionsMenu()
{
	OptionsPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Pause_OptionsPanel"));
	AddPauseCanvasChild(RootCanvas, OptionsPanel, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 3);

	UTextBlock* Title = CreatePauseText(WidgetTree, TEXT("\u9009\u9879"), 30.f, FLinearColor::White);
	AddPauseCanvasChild(OptionsPanel, Title, FAnchors(0.f, 0.f), FMargin(74.f, 62.f, 140.f, 44.f));
	AddPauseCanvasChild(OptionsPanel, CreatePauseLine(WidgetTree, 0.10f), FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(172.f, 80.f, 70.f, 1.f));

	UVerticalBox* Categories = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Pause_OptionsCategoryList"));
	const TArray<FString> CategoryTexts = {
		TEXT("\u63A7\u5236"), TEXT("\u89E6\u89C9\u53CD\u9988"), TEXT("HUD"), TEXT("\u5B57\u5E55"), TEXT("\u663E\u793A"),
		TEXT("\u56FE\u50CF"), TEXT("\u58F0\u97F3"), TEXT("\u8BED\u8A00"), TEXT("\u7CFB\u7EDF"), TEXT("\u8F85\u52A9\u529F\u80FD")
	};
	for (int32 Index = 0; Index < CategoryTexts.Num(); ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>();
		FButtonStyle ButtonStyle = Button->GetStyle();
		ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
		Button->SetStyle(ButtonStyle);
		Button->SetBackgroundColor(FLinearColor::Transparent);

		UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
		Border->SetPadding(FMargin(36.f, 11.f, 36.f, 12.f));
		Border->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		UTextBlock* Text = CreatePauseText(WidgetTree, CategoryTexts[Index], 22.f, FLinearColor(0.22f, 0.22f, 0.22f, 1.f));
		Border->SetContent(Text);
		Button->SetContent(Border);
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Border->Slot))
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Left);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
		}

		OptionCategoryTexts.Add(Text);
		OptionCategoryBorders.Add(Border);
		switch (Index)
		{
		case 0: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory0Hovered); break;
		case 1: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory1Hovered); break;
		case 2: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory2Hovered); break;
		case 3: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory3Hovered); break;
		case 4: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory4Hovered); break;
		case 5: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory5Hovered); break;
		case 6: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory6Hovered); break;
		case 7: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory7Hovered); break;
		case 8: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory8Hovered); break;
		case 9: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategory9Hovered); break;
		default: break;
		}
		Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategoryClicked);
		Button->OnUnhovered.AddDynamic(this, &UPauseMenuWidget::HandleOptionCategoryUnhovered);

		UVerticalBoxSlot* CategorySlot = Categories->AddChildToVerticalBox(Button);
		CategorySlot->SetHorizontalAlignment(HAlign_Left);
		CategorySlot->SetPadding(FMargin(0.f, 0.f, 0.f, 18.f));
	}

	AddPauseCanvasChild(OptionsPanel, Categories, FAnchors(0.f, 0.f), FMargin(100.f, 150.f, 220.f, 620.f));
	AddPauseCanvasChild(OptionsPanel, CreatePauseLine(WidgetTree, 0.10f), FAnchors(0.f, 1.f, 1.f, 1.f), FMargin(70.f, -118.f, 70.f, 1.f));
	SetOptionCategory(0, false);
}

void UPauseMenuWidget::BuildSaveListMenu()
{
	SaveListPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Pause_SaveListPanel"));
	AddPauseCanvasChild(RootCanvas, SaveListPanel, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 3);

	UTextBlock* Title = CreatePauseText(WidgetTree, TEXT("\u4FDD\u5B58\u6E38\u620F"), 30.f, FLinearColor::White);
	AddPauseCanvasChild(SaveListPanel, Title, FAnchors(0.f, 0.f), FMargin(58.f, 62.f, 170.f, 44.f));
	AddPauseCanvasChild(SaveListPanel, CreatePauseLine(WidgetTree, 0.10f), FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(225.f, 80.f, 84.f, 1.f));

	SaveRecordList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Pause_SaveRecordList"));
	AddPauseCanvasChild(SaveListPanel, SaveRecordList, FAnchors(0.f, 0.f), FMargin(56.f, 178.f, 860.f, 620.f));

	UBorder* DetailImage = WidgetTree->ConstructWidget<UBorder>();
	DetailImage->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.10f));
	AddPauseCanvasChild(SaveListPanel, DetailImage, FAnchors(0.f, 0.f), FMargin(1030.f, 252.f, 770.f, 330.f));

	SaveDetailTitle = CreatePauseText(WidgetTree, TEXT(""), 26.f, FLinearColor::White);
	SaveDetailLevel = CreatePauseText(WidgetTree, TEXT(""), 23.f, FLinearColor(0.68f, 0.68f, 0.68f, 1.f));
	SaveDetailPlayTime = CreatePauseText(WidgetTree, TEXT(""), 23.f, FLinearColor(0.68f, 0.68f, 0.68f, 1.f));
	SaveDetailTime = CreatePauseText(WidgetTree, TEXT(""), 23.f, FLinearColor(0.68f, 0.68f, 0.68f, 1.f));
	SaveDetailTitle->SetJustification(ETextJustify::Center);
	SaveDetailLevel->SetJustification(ETextJustify::Center);
	SaveDetailPlayTime->SetJustification(ETextJustify::Center);
	SaveDetailTime->SetJustification(ETextJustify::Center);
	AddPauseCanvasChild(SaveListPanel, SaveDetailTitle, FAnchors(0.f, 0.f), FMargin(1030.f, 172.f, 770.f, 44.f));
	AddPauseCanvasChild(SaveListPanel, SaveDetailLevel, FAnchors(0.f, 0.f), FMargin(1030.f, 628.f, 770.f, 36.f));
	AddPauseCanvasChild(SaveListPanel, SaveDetailPlayTime, FAnchors(0.f, 0.f), FMargin(1030.f, 690.f, 770.f, 36.f));
	AddPauseCanvasChild(SaveListPanel, SaveDetailTime, FAnchors(0.f, 0.f), FMargin(1030.f, 742.f, 770.f, 36.f));

	AddPauseCanvasChild(SaveListPanel, CreatePauseLine(WidgetTree, 0.10f), FAnchors(0.f, 1.f, 1.f, 1.f), FMargin(56.f, -118.f, 70.f, 1.f));
}

void UPauseMenuWidget::BuildConfirmMenu(TObjectPtr<UCanvasPanel>& OutPanel, const FName& PanelName, const FString& Title, const FString& Question, bool bReturnMain)
{
	OutPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), PanelName);
	AddPauseCanvasChild(RootCanvas, OutPanel, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 4);

	AddPauseCanvasChild(OutPanel, CreatePauseText(WidgetTree, Title, 28.f, FLinearColor::White), FAnchors(0.f, 0.f), FMargin(484.f, 420.f, 260.f, 40.f));
	AddPauseCanvasChild(OutPanel, CreatePauseLine(WidgetTree, 0.10f), FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(665.f, 436.f, 485.f, 1.f));
	AddPauseCanvasChild(OutPanel, CreatePauseText(WidgetTree, Question, 26.f, FLinearColor(0.48f, 0.48f, 0.48f, 1.f)), FAnchors(0.f, 0.f), FMargin(516.f, 520.f, 520.f, 42.f));
	AddPauseCanvasChild(OutPanel, CreatePauseLine(WidgetTree, 0.10f), FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(484.f, 650.f, 485.f, 1.f));

	TObjectPtr<UTextBlock>& YesText = bReturnMain ? T_ReturnYes : T_QuitYes;
	TObjectPtr<UTextBlock>& NoText = bReturnMain ? T_ReturnNo : T_QuitNo;
	UButton* YesButton = CreatePauseTextButton(WidgetTree, TEXT("\u662F"), YesText);
	UButton* NoButton = CreatePauseTextButton(WidgetTree, TEXT("\u5426"), NoText);
	AddPauseCanvasChild(OutPanel, YesButton, FAnchors(1.f, 0.f), FMargin(-578.f, 667.f, 70.f, 48.f));
	AddPauseCanvasChild(OutPanel, NoButton, FAnchors(1.f, 0.f), FMargin(-504.f, 667.f, 70.f, 48.f));
	YesButton->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleConfirmYesHovered);
	NoButton->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleConfirmNoHovered);
	YesButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleConfirmYesClicked);
	NoButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleConfirmNoClicked);
}

void UPauseMenuWidget::RebuildShortcutHints(bool bIncludeReset, const FString& SpaceLabel, bool bIncludeDelete)
{
	if (ShortcutHint_Panel && ShortcutHint_Panel->GetParent())
	{
		ShortcutHint_Panel->RemoveFromParent();
	}

	ShortcutHint_Panel = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ShortcutHint_Panel"));
	AddPauseCanvasChild(RootCanvas, ShortcutHint_Panel, FAnchors(1.f, 1.f), FMargin(-540.f, -100.f, 60.f, 44.f), FVector2D::ZeroVector, 10);

	auto AddHint = [this](const FString& Key, const FString& Label)
	{
		UBorder* Frame = WidgetTree->ConstructWidget<UBorder>();
		Frame->SetBrushColor(FLinearColor(0.42f, 0.42f, 0.42f, 0.72f));
		Frame->SetPadding(FMargin(1.f));
		UBorder* Inner = WidgetTree->ConstructWidget<UBorder>();
		Inner->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.64f));
		Inner->SetPadding(FMargin(8.f, 4.f));
		UHorizontalBox* Hint = WidgetTree->ConstructWidget<UHorizontalBox>();
		UBorder* KeyBox = WidgetTree->ConstructWidget<UBorder>();
		KeyBox->SetBrushColor(FLinearColor::White);
		KeyBox->SetPadding(FMargin(5.f, 2.f));
		KeyBox->SetContent(CreatePauseText(WidgetTree, Key, 9.f, FLinearColor::Black));
		Hint->AddChildToHorizontalBox(KeyBox);
		UHorizontalBoxSlot* LabelSlot = Hint->AddChildToHorizontalBox(CreatePauseText(WidgetTree, Label, 20.f, FLinearColor(0.78f, 0.78f, 0.78f, 1.f)));
		LabelSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
		Inner->SetContent(Hint);
		Frame->SetContent(Inner);
		UHorizontalBoxSlot* HintSlot = ShortcutHint_Panel->AddChildToHorizontalBox(Frame);
		HintSlot->SetPadding(FMargin(14.f, 0.f, 0.f, 0.f));
	};

	if (bIncludeDelete)
	{
		AddHint(TEXT("R"), TEXT("\u5220\u9664"));
	}
	AddHint(TEXT("SPACE"), SpaceLabel);
	if (bIncludeReset)
	{
		AddHint(TEXT("R"), TEXT("\u91CD\u7F6E\u4E3A\u9ED8\u8BA4\u8BBE\u7F6E"));
	}
	AddHint(TEXT("ESC"), TEXT("\u8FD4\u56DE"));
}

void UPauseMenuWidget::SetScreen(EPauseMenuScreen Screen)
{
	ActiveScreen = Screen;
	if (MainPanel) MainPanel->SetVisibility(Screen == EPauseMenuScreen::Main ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (OptionsPanel) OptionsPanel->SetVisibility(Screen == EPauseMenuScreen::Options ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (SaveListPanel) SaveListPanel->SetVisibility(Screen == EPauseMenuScreen::SaveList ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (ReturnMainConfirmPanel) ReturnMainConfirmPanel->SetVisibility(Screen == EPauseMenuScreen::ReturnMainConfirm ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (QuitConfirmPanel) QuitConfirmPanel->SetVisibility(Screen == EPauseMenuScreen::QuitConfirm ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (Screen == EPauseMenuScreen::SaveList)
	{
		RefreshSaveList();
		RebuildShortcutHints(false, bSaveListIsSaving ? TEXT("\u4FDD\u5B58") : TEXT("\u8F7D\u5165"), true);
	}
	else
	{
		RebuildShortcutHints(Screen == EPauseMenuScreen::Options);
	}
	SetKeyboardFocus();
}

void UPauseMenuWidget::SetPauseItem(int32 ItemIndex, bool bPlayHoverSound)
{
	if (!PauseItemTexts.IsValidIndex(ItemIndex)) return;
	if (bPlayHoverSound && HoverSound && ItemIndex != SelectedPauseItemIndex)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	SelectedPauseItemIndex = ItemIndex;
	HoveredPauseItemIndex = bPlayHoverSound ? ItemIndex : INDEX_NONE;
	UpdatePauseItemVisuals();
}

void UPauseMenuWidget::UpdatePauseItemVisuals()
{
	for (int32 Index = 0; Index < PauseItemTexts.Num(); ++Index)
	{
		const bool bSelected = Index == SelectedPauseItemIndex;
		PauseItemTexts[Index]->SetColorAndOpacity(FSlateColor(bSelected ? FLinearColor::White : FLinearColor(0.34f, 0.34f, 0.34f, 1.f)));
		PauseItemBorders[Index]->SetBrushColor(Index == HoveredPauseItemIndex ? FLinearColor(1.f, 1.f, 1.f, 0.18f) : FLinearColor(1.f, 1.f, 1.f, 0.f));
	}
}

void UPauseMenuWidget::SetOptionCategory(int32 CategoryIndex, bool bPlayHoverSound)
{
	if (!OptionCategoryTexts.IsValidIndex(CategoryIndex)) return;
	if (bPlayHoverSound && HoverSound && CategoryIndex != SelectedOptionCategoryIndex)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	SelectedOptionCategoryIndex = CategoryIndex;
	HoveredOptionCategoryIndex = bPlayHoverSound ? CategoryIndex : INDEX_NONE;
	UpdateOptionCategoryVisuals();
}

void UPauseMenuWidget::UpdateOptionCategoryVisuals()
{
	for (int32 Index = 0; Index < OptionCategoryTexts.Num(); ++Index)
	{
		const bool bSelected = Index == SelectedOptionCategoryIndex;
		OptionCategoryTexts[Index]->SetColorAndOpacity(FSlateColor(bSelected ? FLinearColor::White : FLinearColor(0.22f, 0.22f, 0.22f, 1.f)));
		OptionCategoryBorders[Index]->SetBrushColor(Index == HoveredOptionCategoryIndex ? FLinearColor(1.f, 1.f, 1.f, 0.18f) : FLinearColor(1.f, 1.f, 1.f, 0.f));
	}
}

void UPauseMenuWidget::SetSaveRecord(int32 RecordIndex, bool bPlayHoverSound)
{
	if (!SaveRecordTexts.IsValidIndex(RecordIndex)) return;
	if (bPlayHoverSound && HoverSound && RecordIndex != SelectedSaveRecordIndex)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	SelectedSaveRecordIndex = RecordIndex;
	HoveredSaveRecordIndex = bPlayHoverSound ? RecordIndex : INDEX_NONE;
	UpdateSaveRecordVisuals();
}

void UPauseMenuWidget::UpdateSaveRecordVisuals()
{
	for (int32 Index = 0; Index < SaveRecordTexts.Num(); ++Index)
	{
		const bool bSelected = Index == SelectedSaveRecordIndex;
		const FLinearColor MainColor = bSelected ? FLinearColor::White : FLinearColor(0.36f, 0.36f, 0.36f, 1.f);
		const FLinearColor SubColor = bSelected ? FLinearColor(0.82f, 0.82f, 0.82f, 1.f) : FLinearColor(0.30f, 0.30f, 0.30f, 1.f);
		SaveRecordTexts[Index]->SetColorAndOpacity(FSlateColor(MainColor));
		if (SaveRecordSubtexts.IsValidIndex(Index))
		{
			SaveRecordSubtexts[Index]->SetColorAndOpacity(FSlateColor(SubColor));
		}
		SaveRecordBorders[Index]->SetBrushColor(Index == HoveredSaveRecordIndex ? FLinearColor(1.f, 1.f, 1.f, 0.18f) : FLinearColor(1.f, 1.f, 1.f, 0.f));
	}

	UProjectUISaveGame* SaveGame = Cast<UProjectUISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (SaveGame && SaveGame->Records.IsEmpty() && !SaveGame->SavedLevelName.IsEmpty())
	{
		FProjectUISaveRecord MigratedRecord;
		MigratedRecord.SavedLevelName = SaveGame->SavedLevelName;
		MigratedRecord.SavedAtText = SaveGame->SavedAtText.IsEmpty() ? FDateTime::Now().ToString(TEXT("%Y/%m/%d    %H:%M:%S")) : SaveGame->SavedAtText;
		MigratedRecord.SaveTitle = TEXT("UI Demo - \u624B\u52A8\u5B58\u6863");
		MigratedRecord.PlayTimeText = TEXT("\u5C11\u4E8E 1 \u5206\u949F");
		SaveGame->Records.Add(MigratedRecord);
	}
	const int32 RecordIndex = bSaveListIsSaving ? SelectedSaveRecordIndex - 1 : SelectedSaveRecordIndex;
	if (SaveGame && SaveGame->Records.IsValidIndex(RecordIndex))
	{
		const FProjectUISaveRecord& Record = SaveGame->Records[RecordIndex];
		SaveDetailTitle->SetText(FText::FromString(Record.SaveTitle));
		SaveDetailLevel->SetText(FText::FromString(TEXT("\u5173\u5361  ") + Record.SavedLevelName));
		SaveDetailPlayTime->SetText(FText::FromString(Record.PlayTimeText));
		SaveDetailTime->SetText(FText::FromString(Record.SavedAtText));
	}
	else
	{
		SaveDetailTitle->SetText(FText::FromString(bSaveListIsSaving ? TEXT("\u65B0\u5B58\u6863\u6570\u636E") : TEXT("\u6CA1\u6709\u53EF\u8F7D\u5165\u7684\u5B58\u6863")));
		SaveDetailLevel->SetText(FText::FromString(GetWorld() ? GetWorld()->GetOutermost()->GetName() : TEXT("")));
		SaveDetailPlayTime->SetText(FText::FromString(TEXT("\u5C11\u4E8E 1 \u5206\u949F")));
		SaveDetailTime->SetText(FText::FromString(FDateTime::Now().ToString(TEXT("%Y/%m/%d    %H:%M:%S"))));
	}
}

void UPauseMenuWidget::RefreshSaveList()
{
	if (!SaveRecordList) return;

	SaveRecordList->ClearChildren();
	SaveRecordTexts.Reset();
	SaveRecordSubtexts.Reset();
	SaveRecordBorders.Reset();

	UProjectUISaveGame* SaveGame = Cast<UProjectUISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (SaveGame && SaveGame->Records.IsEmpty() && !SaveGame->SavedLevelName.IsEmpty())
	{
		FProjectUISaveRecord MigratedRecord;
		MigratedRecord.SavedLevelName = SaveGame->SavedLevelName;
		MigratedRecord.SavedAtText = SaveGame->SavedAtText.IsEmpty() ? FDateTime::Now().ToString(TEXT("%Y/%m/%d    %H:%M:%S")) : SaveGame->SavedAtText;
		MigratedRecord.SaveTitle = TEXT("UI Demo - \u624B\u52A8\u5B58\u6863");
		MigratedRecord.PlayTimeText = TEXT("\u5C11\u4E8E 1 \u5206\u949F");
		SaveGame->Records.Add(MigratedRecord);
		UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
	}
	const int32 ExistingCount = SaveGame ? SaveGame->Records.Num() : 0;
	int32 DisplayCount = bSaveListIsSaving ? ExistingCount + 1 : ExistingCount;
	DisplayCount = FMath::Clamp(DisplayCount, 1, 6);

	for (int32 Index = 0; Index < DisplayCount; ++Index)
	{
		const int32 RecordIndex = bSaveListIsSaving ? Index - 1 : Index;
		const bool bHasRecord = SaveGame && SaveGame->Records.IsValidIndex(RecordIndex);
		const FString Title = bHasRecord ? SaveGame->Records[RecordIndex].SaveTitle : (bSaveListIsSaving && Index == 0 ? TEXT("\u65B0\u5B58\u6863\u6570\u636E") : TEXT("\u6CA1\u6709\u5B58\u6863"));
		const FString Detail = bHasRecord
			? FString::Printf(TEXT("%s    %s"), *SaveGame->Records[RecordIndex].SavedAtText, *SaveGame->Records[RecordIndex].PlayTimeText)
			: (bSaveListIsSaving ? TEXT("\u4ECE\u5F53\u524D\u8FDB\u5EA6\u521B\u5EFA\u65B0\u5B58\u6863") : TEXT("\u8BF7\u5148\u4FDD\u5B58\u6E38\u620F"));

		UButton* Button = WidgetTree->ConstructWidget<UButton>();
		FButtonStyle ButtonStyle = Button->GetStyle();
		ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
		Button->SetStyle(ButtonStyle);
		Button->SetBackgroundColor(FLinearColor::Transparent);

		UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
		Border->SetPadding(FMargin(0.f));
		Border->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		UBorder* Thumbnail = WidgetTree->ConstructWidget<UBorder>();
		Thumbnail->SetBrushColor(bHasRecord ? FLinearColor(1.f, 1.f, 1.f, 0.12f) : FLinearColor(0.f, 0.f, 0.f, 0.36f));
		USizeBox* ThumbnailSize = WidgetTree->ConstructWidget<USizeBox>();
		ThumbnailSize->SetWidthOverride(220.f);
		ThumbnailSize->SetHeightOverride(126.f);
		ThumbnailSize->SetContent(Thumbnail);
		Row->AddChildToHorizontalBox(ThumbnailSize);

		UVerticalBox* Texts = WidgetTree->ConstructWidget<UVerticalBox>();
		UTextBlock* MainText = CreatePauseText(WidgetTree, Title, 23.f, FLinearColor(0.36f, 0.36f, 0.36f, 1.f));
		UTextBlock* SubText = CreatePauseText(WidgetTree, Detail, 21.f, FLinearColor(0.30f, 0.30f, 0.30f, 1.f));
		Texts->AddChildToVerticalBox(MainText);
		UVerticalBoxSlot* SubSlot = Texts->AddChildToVerticalBox(SubText);
		SubSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
		UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(Texts);
		TextSlot->SetPadding(FMargin(32.f, 18.f, 0.f, 0.f));

		Border->SetContent(Row);
		Button->SetContent(Border);
		SaveRecordTexts.Add(MainText);
		SaveRecordSubtexts.Add(SubText);
		SaveRecordBorders.Add(Border);

		switch (Index)
		{
		case 0: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecord0Hovered); break;
		case 1: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecord1Hovered); break;
		case 2: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecord2Hovered); break;
		case 3: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecord3Hovered); break;
		case 4: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecord4Hovered); break;
		case 5: Button->OnHovered.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecord5Hovered); break;
		default: break;
		}
		Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecordClicked);
		Button->OnUnhovered.AddDynamic(this, &UPauseMenuWidget::HandleSaveRecordUnhovered);

		UVerticalBoxSlot* RowSlot = SaveRecordList->AddChildToVerticalBox(Button);
		RowSlot->SetHorizontalAlignment(HAlign_Left);
		RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 24.f));
	}

	SelectedSaveRecordIndex = 0;
	HoveredSaveRecordIndex = INDEX_NONE;
	UpdateSaveRecordVisuals();
}

void UPauseMenuWidget::DeleteSelectedSaveRecord()
{
	UProjectUISaveGame* SaveGame = Cast<UProjectUISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveGame)
	{
		return;
	}

	const int32 RecordIndex = bSaveListIsSaving ? SelectedSaveRecordIndex - 1 : SelectedSaveRecordIndex;
	if (!SaveGame->Records.IsValidIndex(RecordIndex))
	{
		return;
	}

	PlayConfirmClick();
	SaveGame->Records.RemoveAt(RecordIndex);
	if (SaveGame->Records.Num() > 0)
	{
		SaveGame->SavedLevelName = SaveGame->Records[0].SavedLevelName;
		SaveGame->SavedAtText = SaveGame->Records[0].SavedAtText;
	}
	else
	{
		SaveGame->SavedLevelName.Reset();
		SaveGame->SavedAtText.Reset();
	}
	UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
	RefreshSaveList();
}

void UPauseMenuWidget::SetConfirmChoice(EPauseConfirmChoice Choice)
{
	ConfirmChoice = Choice;
	auto Apply = [Choice](UTextBlock* Yes, UTextBlock* No)
	{
		if (Yes) Yes->SetColorAndOpacity(FSlateColor(Choice == EPauseConfirmChoice::Yes ? FLinearColor::White : FLinearColor(0.48f, 0.48f, 0.48f, 1.f)));
		if (No) No->SetColorAndOpacity(FSlateColor(Choice == EPauseConfirmChoice::No ? FLinearColor::White : FLinearColor(0.48f, 0.48f, 0.48f, 1.f)));
	};
	Apply(T_ReturnYes, T_ReturnNo);
	Apply(T_QuitYes, T_QuitNo);
}

void UPauseMenuWidget::ConfirmCurrentSelection()
{
	if (ActiveScreen == EPauseMenuScreen::Main)
	{
		PlayConfirmClick();
		if (SelectedPauseItemIndex == 0)
		{
			SetOptionCategory(0, false);
			SetScreen(EPauseMenuScreen::Options);
		}
		else if (SelectedPauseItemIndex == 4)
		{
			bSaveListIsSaving = true;
			SetScreen(EPauseMenuScreen::SaveList);
		}
		else if (SelectedPauseItemIndex == 5)
		{
			bSaveListIsSaving = false;
			SetScreen(EPauseMenuScreen::SaveList);
		}
		else if (SelectedPauseItemIndex == 6)
		{
			LoadSavedGame(0);
		}
		else if (SelectedPauseItemIndex == 7)
		{
			bConfirmTargetsReturnMain = true;
			SetConfirmChoice(EPauseConfirmChoice::No);
			SetScreen(EPauseMenuScreen::ReturnMainConfirm);
		}
		else if (SelectedPauseItemIndex == 8)
		{
			bConfirmTargetsReturnMain = false;
			SetConfirmChoice(EPauseConfirmChoice::No);
			SetScreen(EPauseMenuScreen::QuitConfirm);
		}
	}
	else if (ActiveScreen == EPauseMenuScreen::Options)
	{
		PlayConfirmClick();
	}
	else if (ActiveScreen == EPauseMenuScreen::SaveList)
	{
		PlayConfirmClick();
		if (bSaveListIsSaving)
		{
			SaveCurrentGame();
			RefreshSaveList();
		}
		else
		{
			LoadSavedGame(SelectedSaveRecordIndex);
		}
	}
	else if (ActiveScreen == EPauseMenuScreen::ReturnMainConfirm || ActiveScreen == EPauseMenuScreen::QuitConfirm)
	{
		if (ConfirmChoice == EPauseConfirmChoice::No)
		{
			PlayConfirmClick();
			SetScreen(EPauseMenuScreen::Main);
		}
		else if (ActiveScreen == EPauseMenuScreen::ReturnMainConfirm)
		{
			PlayConfirmClick();
			UGameplayStatics::OpenLevel(this, MainMenuLevelName);
		}
		else
		{
			PlayConfirmClick();
			UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
		}
	}
}

void UPauseMenuWidget::SaveCurrentGame()
{
	UProjectUISaveGame* SaveGame = Cast<UProjectUISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveGame)
	{
		SaveGame = Cast<UProjectUISaveGame>(UGameplayStatics::CreateSaveGameObject(UProjectUISaveGame::StaticClass()));
	}
	if (!SaveGame || !GetWorld())
	{
		return;
	}

	FProjectUISaveRecord NewRecord;
	NewRecord.SavedLevelName = GetWorld()->GetOutermost()->GetName();
	NewRecord.SavedAtText = FDateTime::Now().ToString(TEXT("%Y/%m/%d    %H:%M:%S"));
	NewRecord.SaveTitle = TEXT("UI Demo - \u624B\u52A8\u5B58\u6863");
	NewRecord.PlayTimeText = TEXT("\u5C11\u4E8E 1 \u5206\u949F");

	const int32 ExistingRecordIndex = SelectedSaveRecordIndex - 1;
	if (SaveGame->Records.IsValidIndex(ExistingRecordIndex))
	{
		SaveGame->Records.RemoveAt(ExistingRecordIndex);
	}
	SaveGame->Records.Insert(NewRecord, 0);
	if (SaveGame->Records.Num() > 12)
	{
		SaveGame->Records.SetNum(12);
	}

	SaveGame->SavedLevelName = NewRecord.SavedLevelName;
	SaveGame->SavedAtText = NewRecord.SavedAtText;
	UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
}

void UPauseMenuWidget::LoadSavedGame(int32 RecordIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return;
	}

	UProjectUISaveGame* SaveGame = Cast<UProjectUISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveGame)
	{
		return;
	}

	FString LevelToOpen;
	if (SaveGame->Records.IsValidIndex(RecordIndex))
	{
		LevelToOpen = SaveGame->Records[RecordIndex].SavedLevelName;
	}
	else
	{
		LevelToOpen = SaveGame->SavedLevelName;
	}
	if (LevelToOpen.IsEmpty())
	{
		return;
	}

	UGameplayStatics::OpenLevel(this, FName(*LevelToOpen), true, TEXT("game=/Script/Project_UI_Menu.ProjectUIGameMode"));
}

void UPauseMenuWidget::ClosePauseMenu()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
	RemoveFromParent();
}

void UPauseMenuWidget::PlayConfirmClick() const
{
	if (ConfirmClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ConfirmClickSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
}

void UPauseMenuWidget::HandlePauseOptionHovered() { SetPauseItem(0, true); }
void UPauseMenuWidget::HandlePausePhotoHovered() { SetPauseItem(1, true); }
void UPauseMenuWidget::HandlePauseExtrasHovered() { SetPauseItem(2, true); }
void UPauseMenuWidget::HandlePauseTutorialHovered() { SetPauseItem(3, true); }
void UPauseMenuWidget::HandlePauseSaveHovered() { SetPauseItem(4, true); }
void UPauseMenuWidget::HandlePauseLoadHovered() { SetPauseItem(5, true); }
void UPauseMenuWidget::HandlePauseRestartHovered() { SetPauseItem(6, true); }
void UPauseMenuWidget::HandlePauseReturnMainHovered() { SetPauseItem(7, true); }
void UPauseMenuWidget::HandlePauseQuitHovered() { SetPauseItem(8, true); }
void UPauseMenuWidget::HandlePauseItemClicked() { ConfirmCurrentSelection(); }
void UPauseMenuWidget::HandlePauseItemUnhovered() { HoveredPauseItemIndex = INDEX_NONE; UpdatePauseItemVisuals(); }

void UPauseMenuWidget::HandleOptionCategory0Hovered() { SetOptionCategory(0, true); }
void UPauseMenuWidget::HandleOptionCategory1Hovered() { SetOptionCategory(1, true); }
void UPauseMenuWidget::HandleOptionCategory2Hovered() { SetOptionCategory(2, true); }
void UPauseMenuWidget::HandleOptionCategory3Hovered() { SetOptionCategory(3, true); }
void UPauseMenuWidget::HandleOptionCategory4Hovered() { SetOptionCategory(4, true); }
void UPauseMenuWidget::HandleOptionCategory5Hovered() { SetOptionCategory(5, true); }
void UPauseMenuWidget::HandleOptionCategory6Hovered() { SetOptionCategory(6, true); }
void UPauseMenuWidget::HandleOptionCategory7Hovered() { SetOptionCategory(7, true); }
void UPauseMenuWidget::HandleOptionCategory8Hovered() { SetOptionCategory(8, true); }
void UPauseMenuWidget::HandleOptionCategory9Hovered() { SetOptionCategory(9, true); }
void UPauseMenuWidget::HandleOptionCategoryClicked() { PlayConfirmClick(); }
void UPauseMenuWidget::HandleOptionCategoryUnhovered() { HoveredOptionCategoryIndex = INDEX_NONE; UpdateOptionCategoryVisuals(); }

void UPauseMenuWidget::HandleSaveRecord0Hovered() { SetSaveRecord(0, true); }
void UPauseMenuWidget::HandleSaveRecord1Hovered() { SetSaveRecord(1, true); }
void UPauseMenuWidget::HandleSaveRecord2Hovered() { SetSaveRecord(2, true); }
void UPauseMenuWidget::HandleSaveRecord3Hovered() { SetSaveRecord(3, true); }
void UPauseMenuWidget::HandleSaveRecord4Hovered() { SetSaveRecord(4, true); }
void UPauseMenuWidget::HandleSaveRecord5Hovered() { SetSaveRecord(5, true); }
void UPauseMenuWidget::HandleSaveRecordClicked() { ConfirmCurrentSelection(); }
void UPauseMenuWidget::HandleSaveRecordUnhovered() { HoveredSaveRecordIndex = INDEX_NONE; UpdateSaveRecordVisuals(); }

void UPauseMenuWidget::HandleConfirmYesHovered()
{
	if (HoverSound && ConfirmChoice != EPauseConfirmChoice::Yes)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	SetConfirmChoice(EPauseConfirmChoice::Yes);
}

void UPauseMenuWidget::HandleConfirmNoHovered()
{
	if (HoverSound && ConfirmChoice != EPauseConfirmChoice::No)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	SetConfirmChoice(EPauseConfirmChoice::No);
}
void UPauseMenuWidget::HandleConfirmYesClicked() { SetConfirmChoice(EPauseConfirmChoice::Yes); ConfirmCurrentSelection(); }
void UPauseMenuWidget::HandleConfirmNoClicked() { SetConfirmChoice(EPauseConfirmChoice::No); ConfirmCurrentSelection(); }

#include "Dialogue/DialogueWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Dialogue/DialogueComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
void AddDialogueCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment = FVector2D::ZeroVector, int32 ZOrder = 0)
{
	UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
	Slot->SetAnchors(Anchors);
	Slot->SetOffsets(Offsets);
	Slot->SetAlignment(Alignment);
	Slot->SetZOrder(ZOrder);
}

UTextBlock* CreateDialogueText(UWidgetTree* Tree, const FString& Text, float FontSize, const FLinearColor& Color)
{
	UTextBlock* TextBlock = Tree->ConstructWidget<UTextBlock>();
	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	return TextBlock;
}
}

UDialogueWidget::UDialogueWidget(const FObjectInitializer& ObjectInitializer)
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

TSharedRef<SWidget> UDialogueWidget::RebuildWidget()
{
	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Dialogue_Root"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* PanelFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Dialogue_PanelFrame"));
	PanelFrame->SetBrushColor(FLinearColor(0.11f, 0.115f, 0.115f, 0.72f));
	PanelFrame->SetPadding(FMargin(74.f, 34.f, 74.f, 34.f));
	AddDialogueCanvasChild(RootCanvas, PanelFrame, FAnchors(0.f, 1.f, 1.f, 1.f), FMargin(0.f, -360.f, 0.f, 0.f), FVector2D::ZeroVector, 0);

	UVerticalBox* TextStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Dialogue_TextStack"));
	PanelFrame->SetContent(TextStack);

	SpeakerText = CreateDialogueText(WidgetTree, TEXT(""), 24.f, FLinearColor::White);
	TextStack->AddChildToVerticalBox(SpeakerText);

	BodyText = CreateDialogueText(WidgetTree, TEXT(""), 21.f, FLinearColor(0.82f, 0.82f, 0.82f, 1.f));
	BodyText->SetAutoWrapText(true);
	UVerticalBoxSlot* BodySlot = TextStack->AddChildToVerticalBox(BodyText);
	BodySlot->SetPadding(FMargin(0.f, 14.f, 0.f, 0.f));

	ChoiceList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Dialogue_ChoiceList"));
	AddDialogueCanvasChild(RootCanvas, ChoiceList, FAnchors(0.56f, 1.f, 1.f, 1.f), FMargin(0.f, -248.f, 120.f, 130.f), FVector2D::ZeroVector, 2);

	ShortcutHint_Panel = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Dialogue_ShortcutHint_Panel"));
	AddDialogueCanvasChild(RootCanvas, ShortcutHint_Panel, FAnchors(1.f, 1.f), FMargin(-380.f, -72.f, 70.f, 44.f), FVector2D::ZeroVector, 4);

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
		KeyBox->SetContent(CreateDialogueText(WidgetTree, Key, 9.f, FLinearColor::Black));
		Hint->AddChildToHorizontalBox(KeyBox);
		UHorizontalBoxSlot* LabelSlot = Hint->AddChildToHorizontalBox(CreateDialogueText(WidgetTree, Label, 20.f, FLinearColor(0.78f, 0.78f, 0.78f, 1.f)));
		LabelSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
		Inner->SetContent(Hint);
		Frame->SetContent(Inner);
		UHorizontalBoxSlot* HintSlot = ShortcutHint_Panel->AddChildToHorizontalBox(Frame);
		HintSlot->SetPadding(FMargin(14.f, 0.f, 0.f, 0.f));
	};

	AddHint(TEXT("SPACE"), TEXT("继续"));
	AddHint(TEXT("ESC"), TEXT("退出"));

	SetIsFocusable(true);
	return Super::RebuildWidget();
}

void UDialogueWidget::StartDialogue(UDialogueComponent* InDialogueComponent, const FUIDialogueNode& FirstNode)
{
	DialogueComponent = InDialogueComponent;
	ShowNode(FirstNode);
	SetKeyboardFocus();
}

void UDialogueWidget::ShowNode(const FUIDialogueNode& Node)
{
	CurrentNode = Node;
	SelectedChoiceIndex = 0;
	HoveredChoiceIndex = INDEX_NONE;

	if (SpeakerText)
	{
		SpeakerText->SetText(Node.SpeakerName);
	}
	if (BodyText)
	{
		BodyText->SetText(Node.DialogueText);
	}

	ChoiceList->ClearChildren();
	ChoiceTexts.Reset();
	ChoiceBorders.Reset();

	for (int32 Index = 0; Index < FMath::Min(Node.Choices.Num(), 6); ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>();
		FButtonStyle Style = Button->GetStyle();
		Style.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
		Button->SetStyle(Style);
		Button->SetBackgroundColor(FLinearColor::Transparent);

		UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
		Border->SetPadding(FMargin(30.f, 9.f, 30.f, 10.f));
		Border->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		UTextBlock* ChoiceText = CreateDialogueText(WidgetTree, Node.Choices[Index].ChoiceText.ToString(), 21.f, FLinearColor(0.42f, 0.42f, 0.42f, 1.f));
		Border->SetContent(ChoiceText);
		Button->SetContent(Border);
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Border->Slot))
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Left);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
		}

		switch (Index)
		{
		case 0: Button->OnHovered.AddDynamic(this, &UDialogueWidget::HandleChoice0Hovered); break;
		case 1: Button->OnHovered.AddDynamic(this, &UDialogueWidget::HandleChoice1Hovered); break;
		case 2: Button->OnHovered.AddDynamic(this, &UDialogueWidget::HandleChoice2Hovered); break;
		case 3: Button->OnHovered.AddDynamic(this, &UDialogueWidget::HandleChoice3Hovered); break;
		case 4: Button->OnHovered.AddDynamic(this, &UDialogueWidget::HandleChoice4Hovered); break;
		case 5: Button->OnHovered.AddDynamic(this, &UDialogueWidget::HandleChoice5Hovered); break;
		default: break;
		}
		Button->OnClicked.AddDynamic(this, &UDialogueWidget::HandleChoiceClicked);
		Button->OnUnhovered.AddDynamic(this, &UDialogueWidget::HandleChoiceUnhovered);

		ChoiceTexts.Add(ChoiceText);
		ChoiceBorders.Add(Border);
		UVerticalBoxSlot* ChoiceSlot = ChoiceList->AddChildToVerticalBox(Button);
		ChoiceSlot->SetHorizontalAlignment(HAlign_Left);
		ChoiceSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
	}

	UpdateChoiceVisuals();
	SetKeyboardFocus();
}

FReply UDialogueWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		PlayConfirmClick();
		EndDialogue();
		return FReply::Handled();
	}

	if (InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		ChooseCurrentSelection();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDialogueWidget::AdvanceDefault()
{
	if (DialogueComponent)
	{
		DialogueComponent->AdvanceToNode(CurrentNode.NextNodeId);
	}
}

void UDialogueWidget::ChooseCurrentSelection()
{
	PlayConfirmClick();
	if (CurrentNode.Choices.IsValidIndex(SelectedChoiceIndex))
	{
		if (DialogueComponent)
		{
			DialogueComponent->AdvanceToNode(CurrentNode.Choices[SelectedChoiceIndex].NextNodeId);
		}
	}
	else
	{
		AdvanceDefault();
	}
}

void UDialogueWidget::SetChoiceIndex(int32 ChoiceIndex, bool bPlayHoverSound)
{
	if (!ChoiceTexts.IsValidIndex(ChoiceIndex))
	{
		return;
	}
	if (bPlayHoverSound && ChoiceIndex != SelectedChoiceIndex)
	{
		PlayHoverClick();
	}
	SelectedChoiceIndex = ChoiceIndex;
	HoveredChoiceIndex = bPlayHoverSound ? ChoiceIndex : INDEX_NONE;
	UpdateChoiceVisuals();
}

void UDialogueWidget::UpdateChoiceVisuals()
{
	for (int32 Index = 0; Index < ChoiceTexts.Num(); ++Index)
	{
		const bool bSelected = Index == SelectedChoiceIndex;
		const FString ChoiceLabel = CurrentNode.Choices.IsValidIndex(Index)
			? FString::Printf(TEXT("%s%s"), bSelected ? TEXT("\u25B6  ") : TEXT("    "), *CurrentNode.Choices[Index].ChoiceText.ToString())
			: FString();
		ChoiceTexts[Index]->SetText(FText::FromString(ChoiceLabel));
		ChoiceTexts[Index]->SetColorAndOpacity(FSlateColor(bSelected ? FLinearColor(0.95f, 0.72f, 0.22f, 1.f) : FLinearColor(0.48f, 0.48f, 0.48f, 1.f)));
		ChoiceBorders[Index]->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
	}
}

void UDialogueWidget::EndDialogue()
{
	if (DialogueComponent)
	{
		DialogueComponent->EndDialogue();
	}
	else
	{
		RemoveFromParent();
	}
}

void UDialogueWidget::PlayConfirmClick() const
{
	if (ConfirmClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ConfirmClickSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
}

void UDialogueWidget::PlayHoverClick() const
{
	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
}

void UDialogueWidget::HandleChoice0Hovered() { SetChoiceIndex(0, true); }
void UDialogueWidget::HandleChoice1Hovered() { SetChoiceIndex(1, true); }
void UDialogueWidget::HandleChoice2Hovered() { SetChoiceIndex(2, true); }
void UDialogueWidget::HandleChoice3Hovered() { SetChoiceIndex(3, true); }
void UDialogueWidget::HandleChoice4Hovered() { SetChoiceIndex(4, true); }
void UDialogueWidget::HandleChoice5Hovered() { SetChoiceIndex(5, true); }
void UDialogueWidget::HandleChoiceClicked() { ChooseCurrentSelection(); }
void UDialogueWidget::HandleChoiceUnhovered() { HoveredChoiceIndex = INDEX_NONE; UpdateChoiceVisuals(); }

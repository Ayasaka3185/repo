#include "Dialogue/WorldMarkerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

TSharedRef<SWidget> UWorldMarkerWidget::RebuildWidget()
{
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("WorldMarker_Root"));
	WidgetTree->RootWidget = Root;

	IconText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Marker_Icon"));
	IconText->SetText(FText::FromString(TEXT("\u25C6")));
	IconText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.78f, 0.08f, 1.f)));
	IconText->SetJustification(ETextJustify::Center);
	FSlateFontInfo IconFont = IconText->GetFont();
	IconFont.Size = 22;
	IconText->SetFont(IconFont);
	Root->AddChildToVerticalBox(IconText);

	DistanceText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Marker_Distance"));
	DistanceText->SetText(FText::FromString(TEXT("-- m")));
	DistanceText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	DistanceText->SetJustification(ETextJustify::Center);
	FSlateFontInfo DistanceFont = DistanceText->GetFont();
	DistanceFont.Size = 18;
	DistanceText->SetFont(DistanceFont);
	UVerticalBoxSlot* DistanceSlot = Root->AddChildToVerticalBox(DistanceText);
	DistanceSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));

	PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Marker_Prompt"));
	PromptText->SetText(FText::FromString(TEXT("E \u4EA4\u8C08")));
	PromptText->SetColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.86f, 0.86f, 1.f)));
	PromptText->SetJustification(ETextJustify::Center);
	FSlateFontInfo PromptFont = PromptText->GetFont();
	PromptFont.Size = 16;
	PromptText->SetFont(PromptFont);
	UVerticalBoxSlot* PromptSlot = Root->AddChildToVerticalBox(PromptText);
	PromptSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));

	return Super::RebuildWidget();
}

void UWorldMarkerWidget::SetMarkerDistance(float DistanceMeters, bool bCanInteract)
{
	if (DistanceText)
	{
		DistanceText->SetText(FText::FromString(FString::Printf(TEXT("%.0f m"), DistanceMeters)));
	}
	if (PromptText)
	{
		PromptText->SetVisibility(bCanInteract ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

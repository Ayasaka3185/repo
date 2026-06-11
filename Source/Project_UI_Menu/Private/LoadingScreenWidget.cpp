#include "LoadingScreenWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Texture2D.h"

namespace
{
void AddLoadingCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment = FVector2D::ZeroVector, int32 ZOrder = 0)
{
	UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
	Slot->SetAnchors(Anchors);
	Slot->SetOffsets(Offsets);
	Slot->SetAlignment(Alignment);
	Slot->SetZOrder(ZOrder);
}

UTextBlock* CreateLoadingText(UWidgetTree* Tree, const FString& Text, float FontSize, const FLinearColor& Color)
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

ULoadingScreenWidget::ULoadingScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> ULoadingScreenWidget::RebuildWidget()
{
	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("LoadingScreen_Root"));
	WidgetTree->RootWidget = RootCanvas;

	BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Loading_Background"));
	BackgroundImage->SetColorAndOpacity(FLinearColor::Black);
	BackgroundTexture = UKismetRenderingLibrary::ImportFileAsTexture2D(this, LoadingImagePath);
	if (BackgroundTexture)
	{
		BackgroundImage->SetBrushFromTexture(BackgroundTexture, true);
		BackgroundImage->SetColorAndOpacity(FLinearColor::White);
	}
	AddLoadingCanvasChild(RootCanvas, BackgroundImage, FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, 0);

	UBorder* BottomShade = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Loading_BottomShade"));
	BottomShade->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.38f));
	AddLoadingCanvasChild(RootCanvas, BottomShade, FAnchors(0.f, 1.f, 1.f, 1.f), FMargin(0.f, -245.f, 0.f, 245.f), FVector2D::ZeroVector, 1);

	UTextBlock* LoreTitle = CreateLoadingText(WidgetTree, TEXT("\u4E16\u754C\u89C21"), 24.f, FLinearColor::White);
	AddLoadingCanvasChild(RootCanvas, LoreTitle, FAnchors(0.f, 1.f), FMargin(58.f, -168.f, 900.f, 32.f), FVector2D::ZeroVector, 2);

	UTextBlock* LoreLineA = CreateLoadingText(WidgetTree, TEXT("\u6000\u5FF5\u805A\u7206\u961F\u6700\u597D\u7684\u4E09\u4E2A\u4EBA\uFF1A\u7231\u5F25\u65AF\uFF08\u5DF2\u590D\u6D3B\uFF09\u3001\u8FBE\u59AE\u5A05\uFF08\u5DF2\u7D2B\u7802\uFF09\u3001\u5343\u54B2\uFF08\u5DF2\u9690\u8EAB\uFF09"), 20.f, FLinearColor(0.82f, 0.82f, 0.82f, 1.f));
	UTextBlock* LoreLineB = CreateLoadingText(WidgetTree, TEXT(""), 20.f, FLinearColor(0.82f, 0.82f, 0.82f, 1.f));
	AddLoadingCanvasChild(RootCanvas, LoreLineA, FAnchors(0.f, 1.f), FMargin(58.f, -126.f, 1120.f, 28.f), FVector2D::ZeroVector, 2);
	AddLoadingCanvasChild(RootCanvas, LoreLineB, FAnchors(0.f, 1.f), FMargin(58.f, -92.f, 1120.f, 28.f), FVector2D::ZeroVector, 2);

	LoadingBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("Loading_ProgressBar"));
	FProgressBarStyle BarStyle;
	BarStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor::Black);
	BarStyle.FillImage.TintColor = FSlateColor(FLinearColor::White);
	LoadingBar->SetWidgetStyle(BarStyle);
	LoadingBar->SetBarFillType(EProgressBarFillType::LeftToRight);
	LoadingBar->SetPercent(0.f);
	AddLoadingCanvasChild(RootCanvas, LoadingBar, FAnchors(0.f, 1.f, 1.f, 1.f), FMargin(58.f, -48.f, 138.f, 4.f), FVector2D::ZeroVector, 2);

	PercentText = CreateLoadingText(WidgetTree, TEXT("0%"), 24.f, FLinearColor::White);
	PercentText->SetJustification(ETextJustify::Right);
	AddLoadingCanvasChild(RootCanvas, PercentText, FAnchors(1.f, 1.f), FMargin(-126.f, -62.f, 76.f, 34.f), FVector2D::ZeroVector, 2);

	return Super::RebuildWidget();
}

void ULoadingScreenWidget::BeginLoadingToLevel(FName InTargetLevelName)
{
	TargetLevelName = InTargetLevelName;
	LoadingElapsed = 0.f;
	bIsLoading = true;
	bHasOpenedLevel = false;
	if (LoadingBar)
	{
		LoadingBar->SetPercent(0.f);
	}
	if (PercentText)
	{
		PercentText->SetText(FText::FromString(TEXT("0%")));
	}
}

void ULoadingScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsLoading || bHasOpenedLevel)
	{
		return;
	}

	LoadingElapsed += InDeltaTime;
	const float Alpha = FMath::Clamp(LoadingElapsed / FakeLoadingDuration, 0.f, 1.f);
	const float DisplayProgress = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 1.6f);
	if (LoadingBar)
	{
		LoadingBar->SetPercent(DisplayProgress);
	}
	if (PercentText)
	{
		PercentText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(DisplayProgress * 100.f))));
	}

	if (Alpha >= 1.f)
	{
		OpenTargetLevel();
	}
}

void ULoadingScreenWidget::OpenTargetLevel()
{
	if (TargetLevelName.IsNone())
	{
		return;
	}

	bHasOpenedLevel = true;
	bIsLoading = false;
	UGameplayStatics::OpenLevel(this, TargetLevelName, true, TEXT("game=/Script/Project_UI_Menu.ProjectUIGameMode"));
}

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;

UCLASS()
class PROJECT_UI_MENU_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	ULoadingScreenWidget(const FObjectInitializer& ObjectInitializer);

	void BeginLoadingToLevel(FName InTargetLevelName);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	FString LoadingImagePath = TEXT("D:/UE5_UI_appendix/loading.jpg");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading", meta = (ClampMin = "0.1"))
	float FakeLoadingDuration = 2.4f;

private:
	void OpenTargetLevel();

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> LoadingBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PercentText;

	FName TargetLevelName;
	float LoadingElapsed = 0.f;
	bool bIsLoading = false;
	bool bHasOpenedLevel = false;
};

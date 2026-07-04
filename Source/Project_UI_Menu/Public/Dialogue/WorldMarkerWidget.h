#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorldMarkerWidget.generated.h"

class UTextBlock;
class UVerticalBox;

UCLASS()
class PROJECT_UI_MENU_API UWorldMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMarkerDistance(float DistanceMeters, bool bCanInteract);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> IconText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DistanceText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PromptText;
};

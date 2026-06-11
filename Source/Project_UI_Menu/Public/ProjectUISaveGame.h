#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ProjectUISaveGame.generated.h"

USTRUCT()
struct FProjectUISaveRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FString SavedLevelName;

	UPROPERTY()
	FString SavedAtText;

	UPROPERTY()
	FString SaveTitle;

	UPROPERTY()
	FString PlayTimeText;
};

UCLASS()
class PROJECT_UI_MENU_API UProjectUISaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString SavedLevelName;

	UPROPERTY()
	FString SavedAtText;

	UPROPERTY()
	TArray<FProjectUISaveRecord> Records;
};

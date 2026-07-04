#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/UIDialogueTypes.h"
#include "DialogueComponent.generated.h"

class UDataTable;
class UDialogueWidget;
class APlayerController;

UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class PROJECT_UI_MENU_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName StartNodeId = TEXT("Start");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TObjectPtr<UDataTable> DialogueTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FUIDialogueNode> InlineNodes;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(APlayerController* PlayerController);

	bool GetNode(FName NodeId, FUIDialogueNode& OutNode) const;
	void AdvanceToNode(FName NodeId);
	void EndDialogue();

protected:
	virtual void BeginPlay() override;

private:
	void BuildDemoDialogueIfEmpty();
	void CacheInlineNodes();

	UPROPERTY(Transient)
	TMap<FName, FUIDialogueNode> NodeMap;

	UPROPERTY(Transient)
	TObjectPtr<UDialogueWidget> ActiveDialogueWidget;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> ActivePlayerController;
};

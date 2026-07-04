#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UIDemoDialogueNPC.generated.h"

class UCapsuleComponent;
class UDialogueComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UWorldMarkerWidget;

UCLASS()
class PROJECT_UI_MENU_API AUIDemoDialogueNPC : public AActor
{
	GENERATED_BODY()

public:
	AUIDemoDialogueNPC();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float InteractionRangeCm = 300.f;

	UDialogueComponent* GetDialogueComponent() const { return DialogueComponent; }
	bool IsPlayerInInteractionRange(APawn* PlayerPawn) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateMarker();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> MarkerWidgetComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDialogueComponent> DialogueComponent;
};

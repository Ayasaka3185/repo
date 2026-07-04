#include "Dialogue/UIDemoDialogueNPC.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Dialogue/DialogueComponent.h"
#include "Dialogue/WorldMarkerWidget.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AUIDemoDialogueNPC::AUIDemoDialogueNPC()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(42.f, 96.f);
	RootComponent = Capsule;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
	BodyMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.9f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (MeshFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(MeshFinder.Object);
	}

	MarkerWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("MarkerWidget"));
	MarkerWidgetComponent->SetupAttachment(RootComponent);
	MarkerWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 105.f));
	MarkerWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	MarkerWidgetComponent->SetDrawSize(FVector2D(160.f, 120.f));
	MarkerWidgetComponent->SetWidgetClass(UWorldMarkerWidget::StaticClass());

	DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));
}

void AUIDemoDialogueNPC::BeginPlay()
{
	Super::BeginPlay();
	UpdateMarker();
}

void AUIDemoDialogueNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateMarker();
}

bool AUIDemoDialogueNPC::IsPlayerInInteractionRange(APawn* PlayerPawn) const
{
	if (!PlayerPawn)
	{
		return false;
	}
	return FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation()) <= InteractionRangeCm;
}

void AUIDemoDialogueNPC::UpdateMarker()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	UWorldMarkerWidget* MarkerWidget = Cast<UWorldMarkerWidget>(MarkerWidgetComponent->GetUserWidgetObject());
	if (!MarkerWidget)
	{
		return;
	}

	const float DistanceMeters = FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation()) / 100.f;
	MarkerWidget->SetMarkerDistance(DistanceMeters, IsPlayerInInteractionRange(PlayerPawn));
}

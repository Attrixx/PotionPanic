#include "StationActorBase.h"
#include "SocketComponent.h"

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// TODO(Nath): Setup socket component attachment
	ItemSocket = CreateDefaultSubobject<USocketComponent>(TEXT("ItemSocket"));
	ItemSocket->SetupAttachment(RootComponent);
}

void AStationActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AStationActorBase::Interact(APlayerController& InInstigator)
{
	// Station interaction is handled by external manager
	// This method satisfies the IInteractable interface
	UE_LOG(LogTemp, Log, TEXT("Station '%s' interacted by player"), *GetName());
}

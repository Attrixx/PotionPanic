#include "StationActorBase.h"
#include "HolderComponent.h"

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// TODO(Nath): Setup socket component attachment
	ItemHolder = CreateDefaultSubobject<UHolderComponent>(TEXT("Item Holder"));
	ItemHolder->SetupAttachment(RootComponent);
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

#include "Core/DeliveryStation.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "ScoreSystem/ScoreWorldSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ADeliveryStation::ADeliveryStation()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	SocketComponent = CreateDefaultSubobject<USocketComponent>(TEXT("SocketComponent"));
	SocketComponent->SetupAttachment(RootComponent);
}

void ADeliveryStation::BeginPlay()
{
	Super::BeginPlay();

	SocketComponent->OnHeldChanged.AddUObject(this, &ADeliveryStation::Deliver);
}

void ADeliveryStation::Deliver(USocketableComponent* OldHeld, USocketableComponent* NewHeld)
{
	if (OldHeld || !NewHeld) // Only fire on put
		return;

	// TODO FRANCOIS NATH // Vérifier si l'objet est aceptable pour ce niveau ici
	if (false)
		return;

	GetWorld()->GetSubsystem<UScoreWorldSubsystem>()->AddScore(1);

	SocketComponent->Take();
	NewHeld->GetOwner()->Destroy();
}

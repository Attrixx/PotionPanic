#include "Core/BinStation.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "ScoreSystem/ScoreWorldSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ABinStation::ABinStation()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	SocketComponent = CreateDefaultSubobject<USocketComponent>(TEXT("SocketComponent"));
	SocketComponent->SetupAttachment(RootComponent);
}

void ABinStation::BeginPlay()
{
	Super::BeginPlay();

	SocketComponent->OnHeldChanged.AddUObject(this, &ABinStation::ThrowAway);
}

void ABinStation::ThrowAway(USocketableComponent* OldHeld, USocketableComponent* NewHeld)
{
	if (OldHeld || !NewHeld) // Only fire on put
		return;

	SocketComponent->Take();
	NewHeld->GetOwner()->Destroy();
}

#include "Core/ObjectSpawner.h"
#include "Core/SpawnableObject.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AObjectSpawner::AObjectSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
	SpawnerMesh->SetupAttachment(RootComponent);
	
	
	SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(RootComponent);
}

void AObjectSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AObjectSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AObjectSpawner::SpawnObject()
{
	if (!ObjectToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectSpawner: ObjectToSpawn n'est pas défini sur %s"), *GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectSpawner: Impossible d'obtenir le World"));
		return;
	}

	FVector SpawnPos = SpawnLocation->GetComponentLocation();
	FRotator SpawnRot = SpawnLocation->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASpawnableObject* SpawnedObject = World->SpawnActor<ASpawnableObject>(
		ObjectToSpawn,
		SpawnPos,
		SpawnRot,
		SpawnParams
	);

	if (SpawnedObject)
	{
		UE_LOG(LogTemp, Log, TEXT("ObjectSpawner: Objet spawné avec succès à %s"), *SpawnPos.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectSpawner: Échec du spawn de l'objet"));
	}
}
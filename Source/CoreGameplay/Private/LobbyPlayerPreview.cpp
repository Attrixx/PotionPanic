// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerPreview.h"

// Sets default values
ALobbyPlayerPreview::ALobbyPlayerPreview()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Player Mesh"));
	RootComponent = PlayerMesh;
	
}

// Called when the game starts or when spawned
void ALobbyPlayerPreview::BeginPlay()
{
	Super::BeginPlay();
	//SetPlayerColor(FColor::White);
}

// Called every frame
void ALobbyPlayerPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALobbyPlayerPreview::SetPlayerColor(FColor Color)
{
	PlayerMesh->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(FName("Color"), Color);
}


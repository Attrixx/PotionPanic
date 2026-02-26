// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerPreview.h"

#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

ALobbyPlayerPreview::ALobbyPlayerPreview()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	RootComponent = CapsuleComponent;

	PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Player Mesh"));
	PlayerMesh->SetupAttachment(RootComponent);
}

void ALobbyPlayerPreview::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
}

void ALobbyPlayerPreview::BeginPlay()
{
	Super::BeginPlay();
	/*FColor TargetColor(UKismetMathLibrary::RandomIntegerInRange(0, 255), UKismetMathLibrary::RandomIntegerInRange(0, 255), UKismetMathLibrary::RandomIntegerInRange(0, 255));
	SetPlayerColor(TargetColor);*/
}

void ALobbyPlayerPreview::OnRep_PreviewColor()
{
	PlayerMesh->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(FName("Color"), PreviewColor);
}

void ALobbyPlayerPreview::SetPlayerColor_Implementation(FColor Color)
{
	PreviewColor = Color;
	if (HasAuthority())
	{
		OnRep_PreviewColor();
	}
}


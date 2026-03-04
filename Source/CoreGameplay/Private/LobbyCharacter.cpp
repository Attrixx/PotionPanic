// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyCharacter.h"
#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"

ALobbyCharacter::ALobbyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Player Mesh"));
	PlayerMesh->SetupAttachment(RootComponent);
}

void ALobbyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	OnPlayerStateReady();
}

void ALobbyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	}
}

void ALobbyCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	OnPlayerStateReady();
}

void ALobbyCharacter::OnPlayerStateReady()
{
	if (ALobbyPlayerState* LobbyPlayerState = GetPlayerState<ALobbyPlayerState>())
	{
		LobbyPlayerState->OnPlayerColorChanged.RemoveDynamic(this, &ALobbyCharacter::SetPlayerColor);
		LobbyPlayerState->OnPlayerColorChanged.AddDynamic(this, &ALobbyCharacter::SetPlayerColor);

		SetPlayerColor(LobbyPlayerState->GetPlayerColor());
	}
}

void ALobbyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyCharacter, CharacterColor);
}

void ALobbyCharacter::SetPlayerColor(FColor Color)
{
	if (HasAuthority())
	{
		CharacterColor = Color;
	}
	PlayerMesh->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(FName("Color"), Color);
}

void ALobbyCharacter::OnRep_CharacterColor()
{
	SetPlayerColor(CharacterColor);
}

void ALobbyCharacter::Move(const FInputActionValue& Value)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!IsValid(PC)) return;
	FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();

	FVector MoveDirection = UKismetMathLibrary::GetForwardVector(CameraRotation) * Value.Get<FVector2D>().Y + UKismetMathLibrary::GetRightVector(CameraRotation) * Value.Get<FVector2D>().X;
	AddMovementInput(MoveDirection);
}


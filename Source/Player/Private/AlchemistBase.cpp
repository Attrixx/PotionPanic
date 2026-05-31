// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistBase.h"
#include "AlchemistMovementComponent.h"
#include "HolderComponent.h"
#include "RangeComponent.h"
#include "Interactable.h"
#include "InputBindable.h"
#include "Carriable.h"
#include "ActorFilters/InteractableActorFilter.h"
#include "ActorFilters/InterfaceActorFilter.h"
#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>
#include "PotionPanicKeybindSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemistBase, Log, All);

AAlchemistBase::AAlchemistBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAlchemistMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	
	HolderComponent = CreateDefaultSubobject<UHolderComponent>(TEXT("Holder Component"));
	HolderComponent->SetupAttachment(GetMesh());

	RangeComponent = CreateDefaultSubobject<URangeComponent>(TEXT("Range Component"));
	RangeComponent->SetupAttachment(RootComponent);

	InteractableFilter = CreateDefaultSubobject<UInteractableActorFilter>(TEXT("Interactable Filter"));
	InteractableFilter->Instigator = this;

	CarriableFilter = CreateDefaultSubobject<UInterfaceActorFilter>(TEXT("Carriable Filter"));
	CarriableFilter->Interface = UCarriable::StaticClass();
}

void AAlchemistBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (HolderComponent && GetMesh() && GetMesh()->DoesSocketExist(HolderParentSocket))
	{
		HolderComponent->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::KeepRelativeTransform,
			HolderParentSocket);
	}
}

void AAlchemistBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAlchemistBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAlchemistBase::NotifyControllerChanged()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(PreviousController))
	{
		ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			UInputMappingContext* ContextToRemove = MovementMappingContext;
			if (auto* KeybindSubsystem = LocalPlayer->GetSubsystem<UPotionPanicKeybindSubsystem>())
			{
				ContextToRemove = KeybindSubsystem->GetRuntimeContext(MovementMappingContext);
			}
			Subsystem->RemoveMappingContext(ContextToRemove);
		}
	}

	Super::NotifyControllerChanged(); // Updates PreviousController

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			UInputMappingContext* ContextToAdd = MovementMappingContext;
			if (auto* KeybindSubsystem = LocalPlayer->GetSubsystem<UPotionPanicKeybindSubsystem>())
			{
				ContextToAdd = KeybindSubsystem->GetRuntimeContext(MovementMappingContext);
			}
			Subsystem->AddMappingContext(ContextToAdd, 0);
		}
	}
}

void AAlchemistBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	auto* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	for (auto* Component : GetComponents())
	{
		if (Component && Component->Implements<UInputBindable>())
		{
			IInputBindable::Execute_SetupInputComponent(Component, EIC);
		}
	}

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAlchemistBase::Input_Move);
	EIC->BindAction(DashAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_Dash);
	EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_Interact);
	EIC->BindAction(PickupOrDropAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_PickupOrDrop);
	EIC->BindAction(ThrowAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_Throw);
}

void AAlchemistBase::Input_Move(const FInputActionValue& Value)
{
	auto Axis2D = Value.Get<FInputActionValue::Axis2D>();

	FRotator CamRotation;
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (PlayerController->PlayerCameraManager)
		{
			CamRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
		}
	}

	// Movement is based on camera forward and right vectors
	FVector RotatedInput = CamRotation.RotateVector({Axis2D.Y, Axis2D.X, 0});
	FVector MovementInput = FVector::VectorPlaneProject(RotatedInput, GetActorUpVector()).GetSafeNormal();
	AddMovementInput(MovementInput);
}

void AAlchemistBase::Input_Dash()
{
	if (auto* AMC = Cast<UAlchemistMovementComponent>(GetCharacterMovement()))
	{
		AMC->Dash();
	}
}

void AAlchemistBase::Input_Interact()
{
	if (AActor* Interactable = RangeComponent->FindBestMatchingActor(InteractableFilter))
		Server_Interact(Interactable);
}

void AAlchemistBase::Input_PickupOrDrop()
{
	if (HolderComponent->GetCarriable())
		Server_Drop();
	else if (AActor* Carriable = RangeComponent->FindBestMatchingActor(CarriableFilter))
		Server_Pickup(Carriable);
}

void AAlchemistBase::Input_Throw()
{
	if (HolderComponent->GetCarriable())
		Server_Throw(GetActorForwardVector());
}

void AAlchemistBase::Server_Interact_Implementation(AActor* Interactable)
{
	if (Interactable && Interactable->Implements<UInteractable>() && RangeComponent->IsActorInRange(Interactable))
	{
		IInteractable::Execute_Interact(Interactable, this);
	}
}

void AAlchemistBase::Server_Pickup_Implementation(AActor* Carriable)
{
	if (Carriable && Carriable->Implements<UCarriable>() && RangeComponent->IsActorInRange(Carriable))
	{
		HolderComponent->TryPickup(Carriable);
	}
}

void AAlchemistBase::Server_Drop_Implementation()
{
	HolderComponent->Release();
}

void AAlchemistBase::Server_Throw_Implementation(FVector Direction)
{
	HolderComponent->Release(Direction.GetSafeNormal2D() * ThrowForce);
}

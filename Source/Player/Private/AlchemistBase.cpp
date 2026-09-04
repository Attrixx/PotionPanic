// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistBase.h"
#include "AlchemistMovementComponent.h"
#include "HolderComponent.h"
#include "RangeComponent.h"
#include "Interactable.h"
#include "InputBindable.h"
#include "Carriable.h"
#include "ActorFilters/InteractableActorFilter.h"
#include "ActorFilters/CarriableActorFilter.h"
#include "ActorFilters/FreeHolderActorFilter.h"
#include "CoreGameplayLibrary.h"
#include "Components/QTEComponent.h"
#include "Components/QTEDisplayComponent.h"
#include "NetworkSoundComponent.h"
#include "NetworkSoundSubsystem.h"
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

	CarriableFilter = CreateDefaultSubobject<UCarriableActorFilter>(TEXT("Carriable Filter"));
	CarriableFilter->Ignored = this;

	FreeHolderFilter = CreateDefaultSubobject<UFreeHolderActorFilter>(TEXT("Free Holder Filter"));
	FreeHolderFilter->Ignored = this;
	
	PhysicalAnimationComponent = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("Physical Animation Component"));
	PhysicalAnimationComponent->StrengthMultiplyer = 5.f;

	// Useful for physical animation (ragdoll)
	GetMesh()->SetCollisionProfileName(TEXT("Pawn"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("IgnoreOnlyPawn"));

	// Enable CustomDepth to have player color and outline when behing walls
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(1);

	QTEComponent = CreateDefaultSubobject<UQTEComponent>(TEXT("QTE Component"));

	QTEDisplayComponent = CreateDefaultSubobject<UQTEDisplayComponent>(TEXT("QTE Display"));
	QTEDisplayComponent->SetupAttachment(RootComponent);

	NetworkSoundComponent = CreateDefaultSubobject<UNetworkSoundComponent>(TEXT("Network Sound"));
}

bool AAlchemistBase::IsCarrying() const
{
	return HolderComponent->GetCarriable() != nullptr;
}

void AAlchemistBase::ApplyStunRagdoll()
{
	// TODO: Temporary for testing
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);

		GetMesh()->AddImpulse(FVector(0.f, 0.f, -1000.f), NAME_None, true);
	}
}

void AAlchemistBase::SetPlayerStencilIndex(int32 StencilValue)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetRenderCustomDepth(StencilValue > 0);
		MeshComp->SetCustomDepthStencilValue(StencilValue);
	}
}

void AAlchemistBase::ApplyCustomization(USkeletalMesh* NewMesh, FColor NewColor)
{
	if (NewMesh)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			MeshComp->SetSkeletalMesh(NewMesh);
		}
	}
	SetColor(NewColor);
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

	// Setup physical animation for ragdolling
	if (!RagdollRootBoneName.IsValid() || RagdollRootBoneName.IsNone())
	{
		UE_LOGFMT(MS_AlchemistBase, Warning, "RagdollRootBoneName is not set for {0}. Physical animation will not be applied.", *GetName());
	}
	else
	{
		PhysicalAnimationComponent->SetSkeletalMeshComponent(GetMesh());
		PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(RagdollRootBoneName, PhysicalAnimationData);
		GetMesh()->SetAllBodiesBelowSimulatePhysics(RagdollRootBoneName, true, false);
	}

	// TODO: Register SetActorCustomDepthEnabled on RangeComponent

#if WITH_EDITORONLY_DATA
	if (bDebugTrackActorFilters)
	{
		RangeComponent->TrackFilter(InteractableFilter);
		RangeComponent->TrackFilter(CarriableFilter);
		RangeComponent->TrackFilter(FreeHolderFilter);
	}
#endif
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

void AAlchemistBase::SetActorCustomDepthEnabled(AActor* TargetActor, bool bEnabled, int32 StencilValue)
{
	if (!TargetActor)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
	{
		if (PrimComp)
		{
			PrimComp->SetRenderCustomDepth(bEnabled);
			PrimComp->SetCustomDepthStencilValue(StencilValue);
			PrimComp->MarkRenderStateDirty();
		}
	}

	TArray<UChildActorComponent*> ChildActorComponents;
	TargetActor->GetComponents<UChildActorComponent>(ChildActorComponents);

	for (UChildActorComponent* ChildActorComp : ChildActorComponents)
	{
		if (ChildActorComp)
		{
			if (AActor* ChildActor = ChildActorComp->GetChildActor())
			{
				SetActorCustomDepthEnabled(ChildActor, bEnabled, StencilValue);
			}
		}
	}
}

UObject* AAlchemistBase::GetQTESourceObject_Implementation() const
{
	return RangeComponent->FindBestMatchingActor(InteractableFilter);
}

void AAlchemistBase::ShowQTEActivityStep_Implementation(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass)
{
	QTEDisplayComponent->ShowQTEActivityStep(InQTEComponent, InWidgetClass);
}

void AAlchemistBase::HideQTEActivityStep_Implementation()
{
	QTEDisplayComponent->HideQTEActivityStep();
}

bool AAlchemistBase::ShouldBlockGameplayInput() const
{
	return QTEComponent && QTEComponent->IsQTERunning();
}

int32 AAlchemistBase::PlayNetworkedSound(USoundBase* Sound)
{
	if (Sound)
	{
		if (UNetworkSoundSubsystem* SoundSys = GetGameInstance()->GetSubsystem<UNetworkSoundSubsystem>())
		{
			// The handle can be stored and passed to StopNetworkedSound() for looping sounds.
			return SoundSys->PlayNetworkedSound(Sound, GetActorLocation(), this);
		}
	}
	return -1;
}

void AAlchemistBase::Input_Move(const FInputActionValue& Value)
{
	// TODO: Disable Mapping context instead of guarding
	if (ShouldBlockGameplayInput())
	{
		return;
	}

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
	// TODO: Disable Mapping context instead of guarding
	if (ShouldBlockGameplayInput())
	{
		return;
	}

	if (auto* AMC = Cast<UAlchemistMovementComponent>(GetCharacterMovement()))
	{
		if (!AMC->CanDash())
		{
			return;
		}

		AMC->Dash();

		PlayNetworkedSound(DashSound);
	}
}

void AAlchemistBase::Input_Interact()
{
	// TODO: Disable Mapping context instead of guarding
	if (ShouldBlockGameplayInput())
	{
		return;
	}

	if (AActor* Interactable = RangeComponent->FindBestMatchingActor(InteractableFilter))
		Server_Interact(Interactable);
}

void AAlchemistBase::Input_PickupOrDrop()
{
	// TODO: Disable Mapping context instead of guarding
	if (ShouldBlockGameplayInput())
	{
		return;
	}

	if (HolderComponent->GetCarriable())
	{
		// Items are never put on the ground: they go from holder to holder, or they get thrown.
		if (AActor* Receiver = RangeComponent->FindBestMatchingActor(FreeHolderFilter))
		{
			Server_Place(Receiver);
			PlayNetworkedSound(DropSound);
		}
	}
	else if (AActor* Candidate = RangeComponent->FindBestMatchingActor(CarriableFilter))
	{
		Server_Pickup(Candidate);
		PlayNetworkedSound(PickupSound);
	}
}

void AAlchemistBase::Input_Throw()
{
	// TODO: Disable Mapping context instead of guarding
	if (ShouldBlockGameplayInput())
	{
		return;
	}

	if (HolderComponent->GetCarriable())
	{
		Server_Throw(GetActorForwardVector());
		PlayNetworkedSound(ThrowSound);
	}
}

void AAlchemistBase::Server_Interact_Implementation(AActor* Interactable)
{
	if (ShouldBlockGameplayInput())
	{
		return;
	}

	if (Interactable && Interactable->Implements<UInteractable>() && RangeComponent->IsActorInRange(Interactable))
	{
		IInteractable::Execute_Interact(Interactable, this);
	}
}

void AAlchemistBase::Server_Pickup_Implementation(AActor* Candidate)
{
	if (ShouldBlockGameplayInput())
	{
		return;
	}

	if (!Candidate || !RangeComponent->IsActorInRange(Candidate))
	{
		return;
	}

	// Candidate is either a loose Carriable itself, or an actor whose occupied holder is offering
	// one up (a station, or another player). Resolve to the actual object either way, and let
	// TryPickup decide whether taking it from there is allowed: same bAllowStealing rule as any
	// other holder-to-holder steal.
	UObject* Target = Candidate->Implements<UCarriable>() ? Candidate : nullptr;

	if (!Target)
	{
		if (UHolderComponent* SourceHolder = UCoreGameplayLibrary::FindComponentInAttachChain<UHolderComponent>(Candidate))
		{
			Target = SourceHolder->GetCarriable();
		}
	}

	if (Target)
	{
		HolderComponent->TryPickup(Target);
	}
}

void AAlchemistBase::Server_Place_Implementation(AActor* Receiver)
{
	if (ShouldBlockGameplayInput())
	{
		return;
	}

	if (!Receiver || !RangeComponent->IsActorInRange(Receiver))
	{
		return;
	}

	UHolderComponent* TargetHolder = UCoreGameplayLibrary::FindComponentInAttachChain<UHolderComponent>(Receiver);
	if (!TargetHolder || TargetHolder == HolderComponent)
	{
		return;
	}

	HolderComponent->TransferTo(TargetHolder);
}

void AAlchemistBase::Server_Throw_Implementation(FVector Direction)
{
	if (!ShouldBlockGameplayInput())
	{
		HolderComponent->Release(Direction.GetSafeNormal2D() * ThrowForce);
	}
}

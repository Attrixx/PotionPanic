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
#include "ActivityExecutor.h"
#include "NetworkSoundComponent.h"
#include "NetworkSoundSubsystem.h"
#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>
#include <InputMappingContext.h>
#include <Net/UnrealNetwork.h>
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

	NetworkSoundComponent = CreateDefaultSubobject<UNetworkSoundComponent>(TEXT("Network Sound"));
}

void AAlchemistBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AAlchemistBase, bActivityInputCaptured, COND_OwnerOnly);
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
	if (const APlayerController* PlayerController = Cast<APlayerController>(PreviousController))
	{
		SetMappingContextActive(PlayerController->GetLocalPlayer(), MovementMappingContext, false, 0);
	}

	Super::NotifyControllerChanged(); // Updates PreviousController

	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		SetMappingContextActive(PlayerController->GetLocalPlayer(), MovementMappingContext, true, 0);
	}

	// Repossess in the middle of a step should not allow movement
	OnRep_ActivityInputCaptured();
}

ULocalPlayer* AAlchemistBase::GetInputLocalPlayer() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	return PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
}

void AAlchemistBase::SetMappingContextActive(ULocalPlayer* LocalPlayer, UInputMappingContext* Context, bool bActive, int32 Priority)
{
	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!Subsystem || !Context)
	{
		return;
	}

	if (auto* KeybindSubsystem = LocalPlayer->GetSubsystem<UPotionPanicKeybindSubsystem>())
	{
		Context = KeybindSubsystem->GetRuntimeContext(Context);
	}

	if (bActive)
	{
		Subsystem->AddMappingContext(Context, Priority);
	}
	else
	{
		Subsystem->RemoveMappingContext(Context);
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

	EIC->BindAction(SlotUpAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_ActivityUp);
	EIC->BindAction(SlotLeftAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_ActivityLeft);
	EIC->BindAction(SlotDownAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_ActivityDown);
	EIC->BindAction(SlotRightAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_ActivityRight);
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

void AAlchemistBase::BeginActivityInputCapture_Implementation(UActivityExecutor* Executor)
{
	CapturingExecutor = Executor;
	SetActivityInputCaptured(true);
}

void AAlchemistBase::EndActivityInputCapture_Implementation()
{
	CapturingExecutor.Reset();
	SetActivityInputCaptured(false);
}

void AAlchemistBase::SetActivityInputCaptured(bool bCaptured)
{
	check(HasAuthority());
	if (bActivityInputCaptured != bCaptured)
	{
		bActivityInputCaptured = bCaptured;
		OnRep_ActivityInputCaptured();
	}
}

void AAlchemistBase::OnRep_ActivityInputCaptured()
{
	if (IsLocallyControlled())
	{
		ULocalPlayer* LocalPlayer = GetInputLocalPlayer();
		SetMappingContextActive(LocalPlayer, MovementMappingContext, !bActivityInputCaptured, 0);
		SetMappingContextActive(LocalPlayer, ComboMappingContext, bActivityInputCaptured, 0);
	}
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

UObject* AAlchemistBase::ResolveCarriable(AActor* Candidate)
{
	// Either a loose Carriable, or an actor whose holder is offering one up (a station, a player).
	if (!Candidate)
	{
		return nullptr;
	}

	if (Candidate->Implements<UCarriable>())
	{
		return Candidate;
	}

	if (UHolderComponent* SourceHolder = UCoreGameplayLibrary::FindComponentInAttachChain<UHolderComponent>(Candidate))
	{
		return SourceHolder->GetCarriable();
	}

	return nullptr;
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
	if (AActor* Interactable = RangeComponent->FindBestMatchingActor(InteractableFilter))
		Server_Interact(Interactable);
}

void AAlchemistBase::Input_PickupOrDrop()
{
	if (HolderComponent->GetCarriable())
	{
		// Items are never put on the ground: they go from holder to holder, or they get thrown.
		if (AActor* Receiver = RangeComponent->FindBestMatchingActor(FreeHolderFilter))
		{
			// Applied before the RPC: the decision itself on the authority, a prediction on a client.
			UHolderComponent* TargetHolder = UCoreGameplayLibrary::FindComponentInAttachChain<UHolderComponent>(Receiver);
			if (TargetHolder && HolderComponent->TransferTo(TargetHolder))
			{
				PlayNetworkedSound(DropSound);
			}

			Server_Place(Receiver);
		}
	}
	else if (AActor* Candidate = RangeComponent->FindBestMatchingActor(CarriableFilter))
	{
		if (HolderComponent->TryPickup(ResolveCarriable(Candidate)))
		{
			PlayNetworkedSound(PickupSound);
		}

		Server_Pickup(Candidate);
	}
}

void AAlchemistBase::Input_Throw()
{
	UObject* Carriable = HolderComponent->GetCarriable();
	if (Carriable && ICarriable::Execute_CanBeThrown(Carriable))
	{
		const FVector Direction = GetActorForwardVector();

		if (HolderComponent->Release(Direction.GetSafeNormal2D() * ThrowForce))
		{
			PlayNetworkedSound(ThrowSound);
		}

		Server_Throw(Direction);
	}
}

void AAlchemistBase::Input_ActivityUp() { Server_ActivityInput(EActivityInputSlot::Up); }
void AAlchemistBase::Input_ActivityLeft() { Server_ActivityInput(EActivityInputSlot::Left); }
void AAlchemistBase::Input_ActivityDown() { Server_ActivityInput(EActivityInputSlot::Down); }
void AAlchemistBase::Input_ActivityRight() { Server_ActivityInput(EActivityInputSlot::Right); }
void AAlchemistBase::Input_ActivityCancel() { Server_ActivityCancel(); }

void AAlchemistBase::Server_Interact_Implementation(AActor* Interactable)
{
	if (bActivityInputCaptured)
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
	if (bActivityInputCaptured)
	{
		return;
	}

	if (!Candidate || !RangeComponent->IsActorInRange(Candidate))
	{
		return;
	}

	// A no-op for the listen server's own player, which already applied this on the key press.
	HolderComponent->TryPickup(ResolveCarriable(Candidate));
}

void AAlchemistBase::Server_Place_Implementation(AActor* Receiver)
{
	if (bActivityInputCaptured)
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
	if (bActivityInputCaptured)
	{
		return;
	}

	UObject* Carriable = HolderComponent->GetCarriable();
	if (Carriable && ICarriable::Execute_CanBeThrown(Carriable))
	{
		HolderComponent->Release(Direction.GetSafeNormal2D() * ThrowForce);
	}
}

void AAlchemistBase::Server_ActivityInput_Implementation(EActivityInputSlot Slot)
{
	if (UActivityExecutor* Executor = CapturingExecutor.Get())
	{
		Executor->ReceiveActivityInput(this, Slot);
	}
}

void AAlchemistBase::Server_ActivityCancel_Implementation()
{
	if (UActivityExecutor* Executor = CapturingExecutor.Get())
	{
		Executor->RequestStepCancel(this);
	}
}

#include "Core/PotionPanicCharacter.h"
#include "Core/PotionPanicPlayerController.h"
#include "Core/PotionPanicPlayerState.h"
#include "Core/CamTargetComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/FlyingSocket.h"
#include "Core/InteractionInterface.h"
#include "Core/GameplayAbilitySystem/PotionPanicTags.h"

#include <Components/SphereComponent.h>
#include <Components/CapsuleComponent.h>
#include <Engine/OverlapResult.h>
#include <Logging/StructuredLog.h>
#include <limits>
#include <AbilitySystemComponent.h>

DEFINE_LOG_CATEGORY_STATIC(MS_PotionPanicCharacter, Log, All);

APotionPanicCharacter::APotionPanicCharacter()
{
	PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
	PickupRange->SetupAttachment(RootComponent);

	CamTargetComponent = CreateDefaultSubobject<UCamTargetComponent>(TEXT("CamTargetComponent"));

	Socket = CreateDefaultSubobject<USocketComponent>(TEXT("Socket"));
	Socket->SetupAttachment(RootComponent);
}

void APotionPanicCharacter::BeginPlay()
{
	Super::BeginPlay();

	PickupRange->OnComponentBeginOverlap.AddDynamic(this, &APotionPanicCharacter::OnComponentBeginOverlap);
	PickupRange->OnComponentEndOverlap.AddDynamic(this, &APotionPanicCharacter::OnComponentEndOverlap);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APotionPanicCharacter::OnHit);
	Socket->OnHeldChanged.AddUObject(this, &APotionPanicCharacter::OnHeldChanged);
}

void APotionPanicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InteractableActorsInRange.Num() >= 2)
	{
		SortInteractablesInRange();
	}

	if (SocketableComponentsInRange.Num() >= 2 && !IsHolding())
	{
		SortSocketablesInRange();
	}

	if (SocketComponentsInRange.Num() >= 2 && IsHolding())
	{
		SortSocketsInRange();
	}
}

void APotionPanicCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	PickupRange->OnComponentBeginOverlap.RemoveAll(this);
	PickupRange->OnComponentEndOverlap.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* APotionPanicCharacter::GetAbilitySystemComponent() const
{
	APotionPanicPlayerState* CurrentPlayerState = GetPlayerState<APotionPanicPlayerState>();
	if (!IsValid(CurrentPlayerState)) return nullptr;
	
	return CurrentPlayerState->GetAbilitySystemComponent();
}

void APotionPanicCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize the Ability System Component when possessed by a controller
	if (!IsValid(GetAbilitySystemComponent()) || !HasAuthority()) return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
	GiveStartupAbilities();
}

void APotionPanicCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Initialize the Ability System Component when the PlayerState is valid
	if (!IsValid(GetAbilitySystemComponent())) return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
}

void APotionPanicCharacter::GiveStartupAbilities()
{
	if (!IsValid(GetAbilitySystemComponent())) return;

	for (const auto& Ability : StartupAbilities)
	{
		FGameplayAbilitySpec Spec = FGameplayAbilitySpec(Ability);
		GetAbilitySystemComponent()->GiveAbility(Spec);
	}
}

void APotionPanicCharacter::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!IsValid(GetAbilitySystemComponent())) return;

	FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(EffectClass, 1.f, EffectContext);

	if (SpecHandle.IsValid())
	{
		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(MS_PotionPanicCharacter, Warning, TEXT("Failed to apply effect to self: SpecHandle is invalid"));
	}
}

void APotionPanicCharacter::RemoveEffectByGrantedTag(const FGameplayTag& TagToRemove)
{
	if (!IsValid(GetAbilitySystemComponent())) return;

	GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TagToRemove));
}

void APotionPanicCharacter::OnHeldChanged(USocketableComponent* OldHeld, USocketableComponent* NewHeld)
{
	if (OldHeld == nullptr && NewHeld != nullptr)
	{
		// Picked up an item
		ApplyEffectToSelf(PickUpEffect);
		ApplyEffectToSelf(CarryingEffect);
	}
}

void APotionPanicCharacter::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->GetOwner() == this)
		return;

	// Only consider actors that implement the interaction interface or are socketable
	// TODO FRANCOIS - For each on all socketables instead
	if (auto* SocketableComponent = OtherActor->GetComponentByClass<USocketableComponent>())
	{
		++SocketableComponentsInRange.FindOrAdd(SocketableComponent);

		if (SocketableComponentsInRange.Num() == 1)
		{
			SetBestSocketable(SocketableComponent);
		}
	}
	// TODO FRANCOIS - Better group of information here
	else if (auto* SocketComponent = OtherActor->GetComponentByClass<USocketComponent>())
	{
		++SocketComponentsInRange.FindOrAdd(SocketComponent);
		if (SocketComponentsInRange.Num() == 1)
		{
			BestSocket = SocketComponent;
		}
	}

	for (auto* Component : OtherActor->GetComponents())
	{
		if (Component->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{
			++InteractableActorsInRange.FindOrAdd(Component);

			if (InteractableActorsInRange.Num() == 1)
			{
				BestInteractableComponent = Component;
				//BestInteractableComponent->SetDistinguish(true);
				ApplyEffectToSelf(CanInteractEffect);
			}
		}
	}
}

void APotionPanicCharacter::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto* SocketableComponent = OtherActor->GetComponentByClass<USocketableComponent>())
	{
		int32* CountPtr = SocketableComponentsInRange.Find(SocketableComponent);
		if (!CountPtr)
			return;
		if (--(*CountPtr) <= 0)
		{
			SocketableComponentsInRange.Remove(SocketableComponent);
			SetBestSocketable(nullptr);
		}

		if (SocketableComponentsInRange.Num() == 1 && !IsHolding())
		{
			SortSocketablesInRange();
		}
	}
	else if (auto* SocketComponent = OtherActor->GetComponentByClass<USocketComponent>())
	{
		int32* CountPtr = SocketComponentsInRange.Find(SocketComponent);
		if (!CountPtr)
			return;
		if (--(*CountPtr) <= 0)
		{
			SocketComponentsInRange.Remove(SocketComponent);
			BestSocket = nullptr;
		}
		if (SocketComponentsInRange.Num() == 1)
		{
			SortSocketsInRange();
		}
	}

	for (auto* Component : OtherActor->GetComponents())
	{
		if (Component->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{
			int32* CountPtr = InteractableActorsInRange.Find(Component);
			if (!CountPtr)
				return;
			if (--(*CountPtr) <= 0)
			{
				InteractableActorsInRange.Remove(Component);
				//if (BestInteractableComponent)
					//BestInteractableComponent->SetDistinguish(false);
				BestInteractableComponent = nullptr;
				RemoveEffectByGrantedTag(PotionPanicTags::Character::State::CanInteract);
			}

			if (SocketableComponentsInRange.Num() == 1)
			{
				SortInteractablesInRange();
				//BestInteractableComponent->SetDistinguish(true);
			}
		}
	}
}

void APotionPanicCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!GetAbilitySystemComponent()) return;
	if (!GetAbilitySystemComponent()->HasMatchingGameplayTag(PotionPanicTags::Character::State::Dashing)) return;
	if (IsHolding())
	{
		APotionPanicPlayerController* PlayerController = Cast<APotionPanicPlayerController>(GetController());
		if (!PlayerController) return;

		PlayerController->ForceDropOnHit();
	}

	if (APotionPanicCharacter* OtherCharacter = Cast<APotionPanicCharacter>(OtherActor))
	{
		if (!OtherCharacter->IsHolding()) return;

		APotionPanicPlayerController* OtherPlayerController = Cast<APotionPanicPlayerController>(OtherCharacter->GetController());
		if (!OtherPlayerController) return;
		OtherPlayerController->ForceDropOnHit();
	}
}

void APotionPanicCharacter::SortInteractablesInRange()
{
	// Sort Interractable always
	TArray<UActorComponent*> IntarractableInRange;
	InteractableActorsInRange.GetKeys(IntarractableInRange);
	float MaxScore = -std::numeric_limits<float>::infinity();

	for (auto* Component : IntarractableInRange)
	{
		float Score = ComputeLocationScore(Component->GetOwner()->GetActorLocation());
		if (Score > MaxScore)
		{
			MaxScore = Score;
			BestInteractableComponent = Component;
			//BestInteractableComponent->SetDistinguish(true);
			ApplyEffectToSelf(CanInteractEffect);
		}
	}
}

void APotionPanicCharacter::SortSocketablesInRange()
{
	TArray<USocketableComponent*> SocketableInRange;
	SocketableComponentsInRange.GetKeys(SocketableInRange);
	float MaxScore = std::numeric_limits<float>::lowest();

	for (auto* Socketable : SocketableInRange)
	{
		float Score = ComputeLocationScore(Socketable->GetComponentLocation());
		if (Score >= MaxScore)
		{
			MaxScore = Score;
			SetBestSocketable(Socketable);
		}
	}
}

void APotionPanicCharacter::SortSocketsInRange()
{
	TArray<USocketComponent*> SocketsInRange;
	SocketComponentsInRange.GetKeys(SocketsInRange);
	float MaxScore = std::numeric_limits<float>::lowest();

	for (auto* SocketInRange : SocketsInRange)
	{
		float Score = ComputeLocationScore(SocketInRange->GetComponentLocation());
		if (Score >= MaxScore)
		{
			MaxScore = Score;
			BestSocket = SocketInRange;
		}
	}
}

float APotionPanicCharacter::ComputeLocationScore(FVector Location)
{
	// Get Angle between forward vector and direction to component
	FVector ToActor = (Location - GetActorLocation());
	float Dot = FVector::DotProduct(GetActorForwardVector(), ToActor.GetSafeNormal());
	
	return Dot - ToActor.Length() / PickupRange->GetScaledSphereRadius();
}

void APotionPanicCharacter::ThrowHeldObject()
{
	auto* Socketable = Socket->Take();
	if (!Socketable)
		return;

	FTransform FlyingSocketTransform =
	{
		GetActorRotation(),
		Socket->GetComponentLocation()
	};

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* FlyingSocket = GetWorld()->SpawnActor<AFlyingSocket>(FlyingSocketClass, FlyingSocketTransform, SpawnParameters);
	if (!FlyingSocket)
	{
		UE_LOGFMT(MS_PotionPanicCharacter, Error, "Could not spawn {0}.",
			FlyingSocketClass ? FlyingSocketClass->GetName() : "NULL");
		return;
	}

	FlyingSocket->IgnoreActor(this);
	FlyingSocket->Launch(*Socketable, GetActorForwardVector() * ObjectThrowSpeed);
	ApplyEffectToSelf(ThrowEffect);
}

void APotionPanicCharacter::Interact()
{	
	if (BestInteractableComponent == nullptr) return;

	if (IInteractionInterface* Interaction = Cast<IInteractionInterface>(BestInteractableComponent))
	{
		Interaction->Interact(this);
	}
}

void APotionPanicCharacter::OnDash()
{
	ApplyEffectToSelf(DashEffect);
	ApplyEffectToSelf(DashingEffect);
}

void APotionPanicCharacter::OnStartUsingStation()
{
	ApplyEffectToSelf(UsingStationEffect);
}

AActor* APotionPanicCharacter::GetBestInteractableActor() const
{
	if (BestInteractableComponent == nullptr) return nullptr;
	return BestInteractableComponent->GetOwner();
}

void APotionPanicCharacter::DropObject()
{
	// TODO FRANCOIS
	auto* Socketable = Socket->Take();
	if (!Socketable)
		return;

	if (BestSocket)
	{
		BestSocket->Put(*Socketable);
	}
	else
	{
		Socketable->SnapToGround();
	}

	SortSocketablesInRange();
	ApplyEffectToSelf(DropEffect);
}

void APotionPanicCharacter::PickupObject(USocketableComponent* ForceSocketable)
{
	USocketableComponent* TargetSocketable = IsValid(ForceSocketable) ? ForceSocketable : BestSocketable;
	if (TargetSocketable)
	{
		Socket->Put(*TargetSocketable);
		SetBestSocketable(nullptr);
	}

	SortInteractablesInRange();
}

void APotionPanicCharacter::SetBestSocketable(USocketableComponent* NewBestSocketable)
{
	if (BestSocketable == NewBestSocketable)
		return;
	if (BestSocketable)
		BestSocketable->SetDistinguish(false);
	BestSocketable = NewBestSocketable;
	if (BestSocketable)
		BestSocketable->SetDistinguish(true);

	if (IsValid(BestSocketable))
	{
		ApplyEffectToSelf(CanPickUpItemEffect);
	}
	else
	{
		RemoveEffectByGrantedTag(PotionPanicTags::Character::State::CanPickUpItem);
	}
}

bool APotionPanicCharacter::IsHolding() const
{
	return Socket->IsHolding();
}

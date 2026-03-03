// Fill out your copyright notice in the Description page of Project Settings.

#include "CarriableComponent.h"
#include <Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY_STATIC(MS_CarriableComponent, Log, All);

UCarriableComponent::UCarriableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCarriableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCarriableComponent, Holder);
}

void UCarriableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		RootPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	}

	if (RootPrimitive.IsValid())
	{
		RootPrimitive->OnComponentHit.AddDynamic(this, &UCarriableComponent::OnRootPrimitiveHit);
	}
	else
	{
		UE_LOGFMT(MS_CarriableComponent, Warning, "Object {0} does not have a primitive root. Carriable Component will not work as expected.",
			GetOwner() ? GetOwner()->GetName() : "NULL");
	}
}

void UCarriableComponent::Throw(const FVector& Impulse)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOGFMT(MS_CarriableComponent, Warning, "Throw must execute on authority. Call ignored.");
		return;
	}

	if (Holder)
	{
		UE_LOGFMT(MS_CarriableComponent, Error, "Cannot throw while being held.");
		return;
	}

	if (!RootPrimitive.IsValid())
	{
		UE_LOGFMT(MS_CarriableComponent, Error, "Invalid throw call on an object without a primitive root.");
		return;
	}

	RootPrimitive->SetSimulatePhysics(true);
	RootPrimitive->SetPhysicsLinearVelocity(Impulse);
}

void UCarriableComponent::SnapToGround()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOGFMT(MS_CarriableComponent, Warning, "SnapToGround must execute on authority. Call ignored.");
		return;
	}

	if (Holder)
	{
		UE_LOGFMT(MS_CarriableComponent, Error, "Cannot snap while being held.");
		return;
	}

	if (!RootPrimitive.IsValid())
	{
		UE_LOGFMT(MS_CarriableComponent, Error, "Invalid snap call on an object without a primitive root.");
		return;
	}

	FHitResult HitResult;
	FVector Start = RootPrimitive->GetComponentLocation();
	FVector End = Start + FVector::DownVector * SnapToGroundMaxDistance;
	FQuat Rot = RootPrimitive->GetComponentQuat();
	auto ProfileName = RootPrimitive->GetCollisionProfileName();
	auto CollisionShape = RootPrimitive->GetCollisionShape();
	FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
	QueryParams.AddIgnoredActor(GetOwner());
	if (!GetWorld()->SweepSingleByProfile(HitResult, Start, End, Rot, ProfileName, CollisionShape, QueryParams))
	{
		UE_LOGFMT(MS_CarriableComponent, Error, "Impossible to snap: ground not found.");
		return;
	}

	RootPrimitive->SetWorldLocation(HitResult.Location);
}

void UCarriableComponent::SetHolder(UHolderComponent* NewHolder)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOGFMT(MS_CarriableComponent, Warning, "SetHolder must execute on authority. Call ignored.");
		return;
	}

	if (NewHolder && RootPrimitive.IsValid())
	{
		RootPrimitive->SetSimulatePhysics(false);
	}

	UHolderComponent* OldHolder = std::exchange(Holder, NewHolder);
	OnHolderChanged(OldHolder, NewHolder);
}

void UCarriableComponent::OnRep_Holder(UHolderComponent* OldHolder)
{
	OnHolderChanged(OldHolder, Holder);
}

void UCarriableComponent::OnRootPrimitiveHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                             FVector NormalImpulse, const FHitResult& Hit)
{
	check(HitComp == RootPrimitive)

	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (Holder)
		return;

	// collision with ground
	if (Hit.Normal.Dot(FVector::UpVector) >= GroundCollisionThreshold)
	{
		RootPrimitive->SetSimulatePhysics(false);
		SnapToGround();
	}
}

void UCarriableComponent::OnHolderChanged_Implementation(UHolderComponent* OldHolder, UHolderComponent* NewHolder)
{
	// Nothing to do by default, can be overriden in bp or cpp.
}

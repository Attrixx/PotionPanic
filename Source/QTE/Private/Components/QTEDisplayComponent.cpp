#include "Components/QTEDisplayComponent.h"

#include "Components/QTEComponent.h"
#include "Widgets/QTEWidgetBase.h"

DEFINE_LOG_CATEGORY_STATIC(MS_QTEDisplayComponent, Log, All);

UQTEDisplayComponent::UQTEDisplayComponent()
{
	SetIsReplicatedByDefault(true);

	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetVisibility(false);
	SetRelativeLocation(OwnerRelativeLocation);
}

void UQTEDisplayComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreOwnerAnchor();
	Super::EndPlay(EndPlayReason);
}

void UQTEDisplayComponent::ShowQTEActivityStep(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass)
{
	Client_ShowQTEActivityStep(InQTEComponent, InWidgetClass);
}

void UQTEDisplayComponent::HideQTEActivityStep()
{
	Client_HideQTEActivityStep();
}

UQTEWidgetBase* UQTEDisplayComponent::GetQTEWidget() const
{
	return Cast<UQTEWidgetBase>(GetUserWidgetObject());
}

AActor* UQTEDisplayComponent::ResolveAnchorActor(UQTEComponent* InQTEComponent) const
{
	if (InQTEComponent)
	{
		// Filled by StartQTEInternal; on a remote client the mirror has already started
		// because Client_StartAuthorityQTE is a reliable RPC sent before this one.
		if (AActor* SourceActor = InQTEComponent->GetCurrentQTEState().SourceActor)
		{
			return SourceActor;
		}
	}

	return GetOwner();
}

void UQTEDisplayComponent::AnchorAboveActor(AActor* AnchorActor)
{
	USceneComponent* AnchorRoot = AnchorActor ? AnchorActor->GetRootComponent() : nullptr;
	if (!AnchorRoot || AnchorActor == GetOwner())
	{
		RestoreOwnerAnchor();
		return;
	}

	AttachToComponent(AnchorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	// Station visuals live in a UChildActorComponent, so the bounds must include child
	// actors - otherwise the medallion would sink into the station mesh.
	FVector Origin = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	AnchorActor->GetActorBounds(false, Origin, Extent, true);

	SetWorldLocation(FVector(Origin.X, Origin.Y, Origin.Z + Extent.Z + AnchorHeightClearance));
}

void UQTEDisplayComponent::RestoreOwnerAnchor()
{
	AActor* Owner = GetOwner();
	USceneComponent* OwnerRoot = Owner ? Owner->GetRootComponent() : nullptr;
	if (!OwnerRoot)
	{
		return;
	}

	if (GetAttachParent() != OwnerRoot)
	{
		AttachToComponent(OwnerRoot, FAttachmentTransformRules::KeepRelativeTransform);
	}

	SetRelativeLocation(OwnerRelativeLocation);
}

void UQTEDisplayComponent::Client_ShowQTEActivityStep_Implementation(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass)
{
	if (InWidgetClass && GetWidgetClass() != InWidgetClass)
	{
		SetWidgetClass(InWidgetClass);
		InitWidget();
	}
	else if (!GetUserWidgetObject() && GetWidgetClass())
	{
		// The widget can be missing on the first show when this component initialised
		// before the local player existed. Retry instead of silently showing nothing.
		InitWidget();
	}

	UQTEWidgetBase* QTEWidget = GetQTEWidget();
	if (!QTEWidget)
	{
		UE_LOG(MS_QTEDisplayComponent,
			Warning,
			TEXT("Cannot show QTE widget on '%s': requested='%s' current='%s'. Set QTEWidgetClass on the QTE activity step and make sure it derives from UQTEWidgetBase."),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(InWidgetClass.Get()),
			*GetNameSafe(GetWidgetClass().Get()));
		return;
	}

	UQTEComponent* ComponentToBind = InQTEComponent ? InQTEComponent : (GetOwner() ? GetOwner()->FindComponentByClass<UQTEComponent>() : nullptr);

	// Client-side only: this is pure presentation, and the server has no UI. The server
	// never touches AttachParent, so the replicated value cannot undo this attachment.
	AnchorAboveActor(ResolveAnchorActor(ComponentToBind));

	QTEWidget->BindToQTEComponent(ComponentToBind);
	SetVisibility(true);
}

void UQTEDisplayComponent::Client_HideQTEActivityStep_Implementation()
{
	if (UQTEWidgetBase* QTEWidget = GetQTEWidget())
	{
		QTEWidget->BindToQTEComponent(nullptr);
	}

	RestoreOwnerAnchor();
	SetVisibility(false);
}

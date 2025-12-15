#include "OrderSystem/OrderClient.h"
#include "Components/TextRenderComponent.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ScoreSystem/ScoreWorldSubsystem.h"
#include "OrderSystem/CommandeManagerWorldSubsystem.h"
#include "Net/UnrealNetwork.h"

#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Math/RotationMatrix.h"

#if WITH_EDITOR
#include "Engine/Blueprint.h"
#endif

AOrderClient::AOrderClient()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(false);

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    OrderTextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("OrderTextComp"));
    OrderTextComp->SetupAttachment(RootComponent);

    OrderTextComp->SetMobility(EComponentMobility::Movable);

    OrderTextComp->SetUsingAbsoluteRotation(true);

    OrderTextComp->SetHorizontalAlignment(EHTA_Center);
    OrderTextComp->SetVerticalAlignment(EVRTA_TextCenter);
    OrderTextComp->SetWorldSize(50.f);
    OrderTextComp->SetTextRenderColor(FColor::White);
    OrderTextComp->SetText(FText::GetEmpty());
}

void AOrderClient::BeginPlay()
{
    Super::BeginPlay();

    bCachedHadActiveOrder = bHasActiveOrder;

    if (OrderTextComp)
    {
        OrderTextComp->SetHiddenInGame(!bUseWorldText);
        OrderTextComp->SetVisibility(bUseWorldText);
        OrderTextComp->SetComponentTickEnabled(bUseWorldText);
    }

    SetActorTickEnabled(bUseWorldText);

    if (UCommandeManagerWorldSubsystem* CmdSub = GetWorld()->GetSubsystem<UCommandeManagerWorldSubsystem>())
    {
        CmdSub->RegisterClient(this);
    }
}

void AOrderClient::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bUseWorldText || !OrderTextComp) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->PlayerCameraManager) return;

    if (!PC->IsLocalController()) return;

    const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    const FVector TextLoc = OrderTextComp->GetComponentLocation();

    FVector ToCam = CamLoc - TextLoc;
    ToCam.Z = 0.f;

    if (ToCam.IsNearlyZero()) return;

    ToCam.Normalize();

    FRotator Rot = FRotationMatrix::MakeFromX(ToCam).Rotator();
    Rot.Pitch = 0.f;
    Rot.Roll = 0.f;

    OrderTextComp->SetWorldRotation(Rot);
}

void AOrderClient::UpdateText3D(const FText& NewText)
{
    if (OrderTextComp)
    {
        OrderTextComp->SetText(NewText);
    }
}

void AOrderClient::BeginOrder(const FClientOrderEntry& NewOrder)
{
    if (!HasAuthority())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(OrderTimerHandle);

    CurrentOrder = NewOrder;
    bHasActiveOrder = true;
    bLastOrderSuccess = false;
    RemainingTime = CurrentOrder.Duration;

    const FText OrderName = CurrentOrder.DisplayText.IsEmpty()
        ? FText::FromName(CurrentOrder.OrderId)
        : CurrentOrder.DisplayText;

    FText FormattedText = FText::Format(
        NSLOCTEXT("Order", "OrderInitialText", "Commande : {0}\nTemps : {1}s"),
        OrderName,
        FText::AsNumber(FMath::CeilToInt(RemainingTime))
    );
    UpdateText3D(FormattedText);

    OnOrderStarted.Broadcast(CurrentOrder);
    BroadcastOrderUpdated();

    GetWorldTimerManager().SetTimer(
        OrderTimerHandle,
        this,
        &AOrderClient::OrderTimerTick,
        1.0f,
        true
    );

    bCachedHadActiveOrder = bHasActiveOrder;
    ForceNetUpdate();
}

void AOrderClient::OrderTimerTick()
{
    if (!HasAuthority() || !bHasActiveOrder) return;

    RemainingTime -= 1.f;

    if (RemainingTime <= 0.f)
    {
        CancelOrder();
        return;
    }

    const FText OrderName = CurrentOrder.DisplayText.IsEmpty()
        ? FText::FromName(CurrentOrder.OrderId)
        : CurrentOrder.DisplayText;

    FText FormattedText = FText::Format(
        NSLOCTEXT("Order", "OrderTickText", "Commande : {0}\nTemps : {1}s"),
        OrderName,
        FText::AsNumber(FMath::CeilToInt(RemainingTime))
    );
    UpdateText3D(FormattedText);
    BroadcastOrderUpdated();
    ForceNetUpdate();
}

void AOrderClient::CancelOrder()
{
    if (!HasAuthority())
    {
        return;
    }

    bHasActiveOrder = false;
    bLastOrderSuccess = false;
    RemainingTime = 0.f;

    GetWorldTimerManager().ClearTimer(OrderTimerHandle);

    UpdateText3D(NSLOCTEXT("Order", "OrderFailed", "Commande ratee"));
    BroadcastOrderUpdated();

    OnOrderFinished.Broadcast(this, CurrentOrder, false);
    ForceNetUpdate();
}

void AOrderClient::CompleteOrder(AActor* DishActor)
{
    if (!HasAuthority())
    {
        return;
    }

    bHasActiveOrder = false;
    bLastOrderSuccess = true;
    GetWorldTimerManager().ClearTimer(OrderTimerHandle);

    UpdateText3D(NSLOCTEXT("Order", "OrderSuccess", "Commande validee !"));
    BroadcastOrderUpdated();

    if (UWorld* World = GetWorld())
    {
        if (UScoreWorldSubsystem* ScoreSubsystem = World->GetSubsystem<UScoreWorldSubsystem>())
        {
            ScoreSubsystem->AddScore(1);
        }
    }

    if (DishActor)
    {
        DishActor->Destroy();
    }

    OnOrderFinished.Broadcast(this, CurrentOrder, true);
    ForceNetUpdate();
}

void AOrderClient::TryServeDish(AActor* DishActor)
{
    if (!bHasActiveOrder || !DishActor)
    {
        return;
    }

    if (!HasAuthority())
    {
        return;
    }

    bool bValid = false;

    if (UCommandeManagerWorldSubsystem* CmdSub = GetWorld()->GetSubsystem<UCommandeManagerWorldSubsystem>())
    {
        bValid = CmdSub->ValidateDishForClient(this, DishActor);
    }
    else
    {
        bValid = CheckDishMatchesCurrentOrder(DishActor);
    }

    if (!bValid)
    {
        UpdateText3D(NSLOCTEXT("Order", "OrderWrongDish", "Ce n'est pas la bonne commande !"));
        return;
    }

    CompleteOrder(DishActor);
}

bool AOrderClient::CheckDishMatchesCurrentOrder(AActor* DishActor) const
{
    if (!DishActor || !bHasActiveOrder)
    {
        return false;
    }

    if (UObject* PayloadObject = CurrentOrder.Payload.Get())
    {
        if (const UClass* ExpectedClass = Cast<UClass>(PayloadObject))
        {
            if (DishActor->IsA(ExpectedClass))
            {
                return true;
            }
        }

#if WITH_EDITOR
        if (const UBlueprint* BlueprintAsset = Cast<UBlueprint>(PayloadObject))
        {
            if (BlueprintAsset->GeneratedClass && DishActor->IsA(BlueprintAsset->GeneratedClass))
            {
                return true;
            }
        }
#endif

        if (const AActor* ActorTemplate = Cast<AActor>(PayloadObject))
        {
            if (DishActor->IsA(ActorTemplate->GetClass()))
            {
                return true;
            }
        }
    }

    if (CurrentOrder.OrderId != NAME_None)
    {
        if (DishActor->ActorHasTag(CurrentOrder.OrderId))
        {
            return true;
        }

        if (DishActor->GetClass() && DishActor->GetClass()->GetFName() == CurrentOrder.OrderId)
        {
            return true;
        }
    }

    return CurrentOrder.Payload == nullptr && CurrentOrder.OrderId == NAME_None;
}

void AOrderClient::BroadcastOrderUpdated()
{
    OnOrderUpdated.Broadcast(CurrentOrder, RemainingTime, bHasActiveOrder);
}

void AOrderClient::OnRep_OrderState()
{
    // Refresh 3D text for late joiners or clients without authority.
    if (bUseWorldText)
    {
        if (bHasActiveOrder)
        {
            const FText OrderName = CurrentOrder.DisplayText.IsEmpty()
                ? FText::FromName(CurrentOrder.OrderId)
                : CurrentOrder.DisplayText;

            FText FormattedText = FText::Format(
                NSLOCTEXT("Order", "OrderTickText_Replicated", "Commande : {0}\nTemps : {1}s"),
                OrderName,
                FText::AsNumber(FMath::CeilToInt(RemainingTime))
            );
            UpdateText3D(FormattedText);
        }
        else
        {
            UpdateText3D(bLastOrderSuccess
                ? NSLOCTEXT("Order", "OrderSuccess_Replicated", "Commande validee !")
                : NSLOCTEXT("Order", "OrderFailed_Replicated", "Commande ratee"));
        }
    }

    BroadcastOrderUpdated();

    if (bHasActiveOrder && !bCachedHadActiveOrder)
    {
        OnOrderStarted.Broadcast(CurrentOrder);
    }

    if (!bHasActiveOrder && bCachedHadActiveOrder)
    {
        OnOrderFinished.Broadcast(this, CurrentOrder, bLastOrderSuccess);
    }

    bCachedHadActiveOrder = bHasActiveOrder;
}

void AOrderClient::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AOrderClient, CurrentOrder);
    DOREPLIFETIME(AOrderClient, RemainingTime);
    DOREPLIFETIME(AOrderClient, bHasActiveOrder);
    DOREPLIFETIME(AOrderClient, bLastOrderSuccess);
}

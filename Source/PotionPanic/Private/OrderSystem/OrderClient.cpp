#include "OrderSystem/OrderClient.h"
#include "Components/TextRenderComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ScoreSystem/ScoreWorldSubsystem.h"
#include "OrderSystem/CommandeManagerWorldSubsystem.h"

#if WITH_EDITOR
#include "Engine/Blueprint.h"
#endif

AOrderClient::AOrderClient()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    OrderTextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("OrderTextComp"));
    OrderTextComp->SetupAttachment(RootComponent);

    OrderTextComp->SetHorizontalAlignment(EHTA_Center);
    OrderTextComp->SetVerticalAlignment(EVRTA_TextCenter);
    OrderTextComp->SetWorldSize(50.f);
    OrderTextComp->SetTextRenderColor(FColor::White);
    OrderTextComp->SetText(FText::GetEmpty());
}

void AOrderClient::BeginPlay()
{
    Super::BeginPlay();

    if (UCommandeManagerWorldSubsystem* CmdSub = GetWorld()->GetSubsystem<UCommandeManagerWorldSubsystem>())
    {
        CmdSub->RegisterClient(this);
    }
}

void AOrderClient::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!OrderTextComp) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    FVector CamLocation;
    FRotator CamRotation;
    PC->GetPlayerViewPoint(CamLocation, CamRotation);

    const FVector TextLocation = OrderTextComp->GetComponentLocation();
    FVector ToCam = CamLocation - TextLocation;
    ToCam.Z = 0.f;

    if (!ToCam.IsNearlyZero())
    {
        FRotator LookAtRot = FRotationMatrix::MakeFromX(ToCam).Rotator();
        OrderTextComp->SetWorldRotation(LookAtRot);
    }
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
    GetWorldTimerManager().ClearTimer(OrderTimerHandle);

    CurrentOrder = NewOrder;
    bHasActiveOrder = true;
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

    GetWorldTimerManager().SetTimer(
        OrderTimerHandle,
        this,
        &AOrderClient::OrderTimerTick,
        1.0f,
        true
    );
}

void AOrderClient::OrderTimerTick()
{
    if (!bHasActiveOrder) return;

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
}

void AOrderClient::CancelOrder()
{
    bHasActiveOrder = false;
    RemainingTime = 0.f;

    GetWorldTimerManager().ClearTimer(OrderTimerHandle);

    UpdateText3D(NSLOCTEXT("Order", "OrderFailed", "Commande ratée"));

    OnOrderFinished.Broadcast(this, CurrentOrder, false);
}

void AOrderClient::CompleteOrder(AActor* DishActor)
{
    bHasActiveOrder = false;
    GetWorldTimerManager().ClearTimer(OrderTimerHandle);

    UpdateText3D(NSLOCTEXT("Order", "OrderSuccess", "Commande validée !"));

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
}

void AOrderClient::TryServeDish(AActor* DishActor)
{
    if (!bHasActiveOrder || !DishActor)
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

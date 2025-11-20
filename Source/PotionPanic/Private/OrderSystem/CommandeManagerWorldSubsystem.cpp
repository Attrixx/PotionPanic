#include "OrderSystem/CommandeManagerWorldSubsystem.h"
#include "OrderSystem/OrderSettings.h"
#include "OrderSystem/OrdersDataAsset.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UCommandeManagerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UOrderSettings* Settings = GetDefault<UOrderSettings>();

    OrdersPerRound = Settings ? FMath::Max(1, Settings->OrdersPerRound) : 2;
    NextOrderDelay = Settings ? FMath::Max(0.f, Settings->NextOrderDelay) : 2.f;
    RoundRestartDelay = Settings ? FMath::Max(0.f, Settings->RoundRestartDelay) : 3.f;
    bAutoStartOnRegister = Settings ? Settings->bAutoStartOnRegister : true;
    bAvoidSameOrderTwice = Settings ? Settings->bAvoidSameOrderTwice : true;
    bReloadLevelOnRoundEnd = Settings ? Settings->bReloadLevelOnRoundEnd : false;

    CachedOrders.Reset();

    if (Settings && !Settings->OrdersAsset.IsNull())
    {
        UOrdersDataAsset* Asset = Settings->OrdersAsset.LoadSynchronous();
        if (Asset)
        {
            CachedOrders = Asset->Orders;
        }
    }
}

void UCommandeManagerWorldSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        for (auto& Pair : States)
        {
            FRoundState& State = Pair.Value;
            World->GetTimerManager().ClearTimer(State.NextOrderHandle);
            World->GetTimerManager().ClearTimer(State.RestartHandle);
        }
    }

    States.Reset();
    CachedOrders.Reset();
    Super::Deinitialize();
}

void UCommandeManagerWorldSubsystem::RegisterClient(AOrderClient* Client)
{
    if (!Client) return;

    States.FindOrAdd(Client);

    Client->OnOrderFinished.AddUniqueDynamic(
        this,
        &UCommandeManagerWorldSubsystem::HandleOrderFinished
    );

    if (bAutoStartOnRegister)
    {
        StartRound(Client);
    }
}

void UCommandeManagerWorldSubsystem::StartRound(AOrderClient* Client)
{
    if (!Client) return;
    if (CachedOrders.Num() == 0) return;

    FRoundState& State = States.FindOrAdd(Client);
    State.Served = 0;
    State.Success = 0;
    State.bInRound = true;
    State.LastIndex = INDEX_NONE;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(State.NextOrderHandle);
        World->GetTimerManager().ClearTimer(State.RestartHandle);
    }

    RefillAndShufflePool(State);
    IssueNextOrder(Client);
}

bool UCommandeManagerWorldSubsystem::ValidateDishForClient(AOrderClient* Client, AActor* DishActor) const
{
    if (!Client || !DishActor || !Client->HasActiveOrder())
    {
        return false;
    }

    return Client->CheckDishMatchesCurrentOrder(DishActor);
}

void UCommandeManagerWorldSubsystem::HandleOrderFinished(AOrderClient* Client, const FClientOrderEntry&, bool bSuccess)
{
    if (!Client) return;

    FRoundState* StatePtr = States.Find(Client);
    if (!StatePtr || !StatePtr->bInRound) return;

    FRoundState& State = *StatePtr;

    State.Served++;
    if (bSuccess)
    {
        State.Success++;
    }

    if (State.Served < OrdersPerRound)
    {
        ScheduleNextOrder(Client);
    }
    else
    {
        EndRound(Client);
    }
}

void UCommandeManagerWorldSubsystem::IssueNextOrder(AOrderClient* Client)
{
    if (!Client) return;
    if (CachedOrders.Num() == 0) return;

    FRoundState& State = States.FindOrAdd(Client);
    const int32 Index = DrawFromPool(State);
    if (Index == INDEX_NONE) return;

    Client->BeginOrder(CachedOrders[Index]);
}

void UCommandeManagerWorldSubsystem::ScheduleNextOrder(AOrderClient* Client)
{
    if (!Client) return;

    FRoundState& State = States.FindOrAdd(Client);

    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<AOrderClient> WeakClient = Client;

        FTimerDelegate Del;
        Del.BindWeakLambda(this, [this, WeakClient]()
            {
                if (WeakClient.IsValid())
                {
                    IssueNextOrder(WeakClient.Get());
                }
            });

        World->GetTimerManager().SetTimer(
            State.NextOrderHandle,
            Del,
            NextOrderDelay,
            false
        );
    }
}

void UCommandeManagerWorldSubsystem::EndRound(AOrderClient* Client)
{
    if (!Client) return;

    FRoundState& State = States.FindOrAdd(Client);
    State.bInRound = false;

    EOrderRoundResult Result;
    if (State.Success >= OrdersPerRound)
    {
        Result = EOrderRoundResult::PerfectWin;
    }
    else if (State.Success == 1)
    {
        Result = EOrderRoundResult::PartialWin;
    }
    else
    {
        Result = EOrderRoundResult::Lose;
    }

    OnRoundEnded.Broadcast(Client, Result, State.Success);

    if (UWorld* World = GetWorld())
    {
        if (bReloadLevelOnRoundEnd)
        {
            FTimerDelegate Del;
            Del.BindWeakLambda(this, [this]()
                {
                    ReloadCurrentLevel();
                });

            World->GetTimerManager().SetTimer(
                State.RestartHandle,
                Del,
                RoundRestartDelay,
                false
            );
        }
        else
        {
            TWeakObjectPtr<AOrderClient> WeakClient = Client;

            FTimerDelegate Del;
            Del.BindWeakLambda(this, [this, WeakClient]()
                {
                    if (WeakClient.IsValid())
                    {
                        StartRound(WeakClient.Get());
                    }
                });

            World->GetTimerManager().SetTimer(
                State.RestartHandle,
                Del,
                RoundRestartDelay,
                false
            );
        }
    }
}

void UCommandeManagerWorldSubsystem::ReloadCurrentLevel()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const FName LevelName(*World->GetName());
    UGameplayStatics::OpenLevel(this, LevelName, true);
}

void UCommandeManagerWorldSubsystem::RefillAndShufflePool(FRoundState& State)
{
    State.Pool.Reset();
    for (int32 i = 0; i < CachedOrders.Num(); ++i)
    {
        State.Pool.Add(i);
    }

    for (int32 i = State.Pool.Num() - 1; i > 0; --i)
    {
        const int32 j = FMath::RandRange(0, i);
        if (i != j)
        {
            State.Pool.Swap(i, j);
        }
    }
}

int32 UCommandeManagerWorldSubsystem::DrawFromPool(FRoundState& State)
{
    const int32 NumOrders = CachedOrders.Num();
    if (NumOrders <= 0)
        return INDEX_NONE;

    bool bNeedRefill = (State.Pool.Num() == 0);

    if (!bNeedRefill)
    {
        for (int32 V : State.Pool)
        {
            if (V < 0 || V >= NumOrders)
            {
                bNeedRefill = true;
                break;
            }
        }
    }

    if (bNeedRefill)
    {
        RefillAndShufflePool(State);
        if (State.Pool.Num() == 0)
            return INDEX_NONE;
    }

    int32 Index = State.Pool.Pop(EAllowShrinking::No);

    if (bAvoidSameOrderTwice && NumOrders > 1 && Index == State.LastIndex)
    {
        if (State.Pool.Num() == 0)
        {
            RefillAndShufflePool(State);
        }

        if (State.Pool.Num() > 0)
        {
            int32 Other = State.Pool.Pop(EAllowShrinking::No);
            State.Pool.Add(Index);
            Index = Other;
        }
    }

    State.LastIndex = Index;
    return Index;
}

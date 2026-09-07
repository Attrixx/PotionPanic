// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameState.h"

// Console commands, and the round surgery behind them, are development only.
#if !UE_BUILD_SHIPPING

#include <Engine/Engine.h>
#include <Engine/World.h>
#include <HAL/IConsoleManager.h>
#include <Logging/StructuredLog.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemyGameState_Cheats, Log, All);

struct FAlchemyGameStateCheats
{
	/** @return The placed order expiring the soonest, or null when none is placed. */
	static FItemOrder* FindSoonestPlacedOrder(AAlchemyGameState& GameState)
	{
		FItemOrder* Soonest = nullptr;
		for (FItemOrder& Order : GameState.RoundOrders)
		{
			if (Order.State != EOrderState::Placed)
				continue;

			if (!Soonest || Order.StartTime + Order.MaxDuration < Soonest->StartTime + Soonest->MaxDuration)
				Soonest = &Order;
		}

		return Soonest;
	}

	static bool CompleteSoonestOrder(AAlchemyGameState& GameState)
	{
		const FItemOrder* Soonest = FindSoonestPlacedOrder(GameState);
		return Soonest && GameState.DeliverOrder(Soonest->Item);
	}

	static bool FailSoonestOrder(AAlchemyGameState& GameState)
	{
		if (FItemOrder* Soonest = FindSoonestPlacedOrder(GameState))
		{
			GameState.SetOrderState(*Soonest, EOrderState::Cancelled);
			return true;
		}

		return false;
	}

	static bool EndRound(AAlchemyGameState& GameState)
	{
		// Ticking is what tells a running round from a finished one: StartRound turns it on, EndRound off.
		if (!GameState.IsActorTickEnabled())
			return false;

		GameState.EndRound();
		return true;
	}
};

namespace
{
constexpr float ScreenMessageDuration = 5.f;

void Report(const FString& Message, bool bSucceeded)
{
	if (bSucceeded)
		UE_LOGFMT(MS_AlchemyGameState_Cheats, Log, "{0}", Message);
	else
		UE_LOGFMT(MS_AlchemyGameState_Cheats, Warning, "{0}", Message);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, ScreenMessageDuration,
			bSucceeded ? FColor::Green : FColor::Red, Message);
	}
}

void RunCheat(UWorld* World, const TCHAR* Name, TFunctionRef<bool(AAlchemyGameState&)> Cheat)
{
	AAlchemyGameState* GameState = World ? World->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
	{
		Report(FString::Printf(TEXT("CHEAT %s: no AlchemyGameState in this world."), Name), false);
		return;
	}

	if (!GameState->HasAuthority())
	{
		Report(FString::Printf(TEXT("CHEAT %s: refused, host only."), Name), false);
		return;
	}

	const bool bDone = Cheat(*GameState);
	Report(FString::Printf(TEXT("CHEAT %s: %s"), Name, bDone ? TEXT("applied") : TEXT("nothing to apply")), bDone);
}

void CompleteOrder(UWorld* World)
{
	RunCheat(World, TEXT("CompleteOrder"), &FAlchemyGameStateCheats::CompleteSoonestOrder);
}

void FailOrder(UWorld* World)
{
	RunCheat(World, TEXT("FailOrder"), &FAlchemyGameStateCheats::FailSoonestOrder);
}

void EndRound(UWorld* World)
{
	RunCheat(World, TEXT("EndRound"), &FAlchemyGameStateCheats::EndRound);
}
}

static FAutoConsoleCommandWithWorld GCheatCompleteOrder(
	TEXT("PotionPanic.Cheat.CompleteOrder"),
	TEXT("Completes the order expiring the soonest, as delivering its item would. Host only."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&CompleteOrder));

static FAutoConsoleCommandWithWorld GCheatFailOrder(
	TEXT("PotionPanic.Cheat.FailOrder"),
	TEXT("Cancels the order expiring the soonest, as running out of time would. Host only."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&FailOrder));

static FAutoConsoleCommandWithWorld GCheatEndRound(
	TEXT("PotionPanic.Cheat.EndRound"),
	TEXT("Ends the running round now, resolving it as its own timer would. Host only."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&EndRound));

#endif

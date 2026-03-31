#pragma once

#include "CoreMinimal.h"
#include "Core/QTETypes.h"

class UQTEDefinitionDataAsset;

class QTE_API FQTEResolver
{
public:
	static float GetDifficultyMultiplier(const UQTEDefinitionDataAsset* Definition);
	static float GetEffectiveGlobalTimeout(const UQTEDefinitionDataAsset* Definition);
	static float GetEffectiveStepTimeout(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step);
	static float GetEffectiveTolerance(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step);
	static float GetEffectiveHoldTime(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step);
	static int32 GetEffectiveMashTarget(const UQTEDefinitionDataAsset* Definition, const FQTEStepDefinition& Step);
	static const FQTEOutcomeConfiguration* GetOutcomeConfiguration(const UQTEDefinitionDataAsset* Definition, EQTEState FinalState);
	static EQTEGrade ResolveGrade(const FQTERuntimeState& RuntimeState, EQTEState FinalState);
	static FText ResolveOutcomeMessage(const UQTEDefinitionDataAsset* Definition, EQTEState FinalState, const FText& OverrideMessage);
	static FQTEResult BuildResult(
		const UQTEDefinitionDataAsset* Definition,
		UObject* SourceObject,
		AActor* InstigatorActor,
		const FQTERuntimeState& RuntimeState,
		EQTEState FinalState,
		const FText& OverrideMessage);
	static FQTEAuthorityResult BuildAuthorityResult(
		const UQTEDefinitionDataAsset* Definition,
		const FQTERuntimeState& RuntimeState,
		const FQTEResult& LastResult,
		EQTEState FallbackState);
};

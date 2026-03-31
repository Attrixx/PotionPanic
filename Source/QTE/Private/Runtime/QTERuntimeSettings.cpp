#include "Runtime/QTERuntimeSettings.h"

#include "Runtime/QTEChainRuntime.h"
#include "Runtime/QTEHoldRuntime.h"
#include "Runtime/QTEMashRuntime.h"
#include "Runtime/QTEPressRuntime.h"
#include "Runtime/QTESequenceRuntime.h"

TUniquePtr<FQTERuntime> UQTEPressRuntimeSettings::CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const
{
	return MakeUnique<FQTEPressRuntime>(OwnerComponent, Definition);
}

EQTEType UQTEPressRuntimeSettings::GetQTEType() const
{
	return EQTEType::Press;
}

TUniquePtr<FQTERuntime> UQTEHoldRuntimeSettings::CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const
{
	return MakeUnique<FQTEHoldRuntime>(OwnerComponent, Definition);
}

EQTEType UQTEHoldRuntimeSettings::GetQTEType() const
{
	return EQTEType::Hold;
}

TUniquePtr<FQTERuntime> UQTEMashRuntimeSettings::CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const
{
	return MakeUnique<FQTEMashRuntime>(OwnerComponent, Definition);
}

EQTEType UQTEMashRuntimeSettings::GetQTEType() const
{
	return EQTEType::Mash;
}

TUniquePtr<FQTERuntime> UQTESequenceRuntimeSettings::CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const
{
	return MakeUnique<FQTESequenceRuntime>(OwnerComponent, Definition);
}

EQTEType UQTESequenceRuntimeSettings::GetQTEType() const
{
	return EQTEType::Sequence;
}

TUniquePtr<FQTERuntime> UQTEChainRuntimeSettings::CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const
{
	return MakeUnique<FQTEChainRuntime>(OwnerComponent, Definition);
}

EQTEType UQTEChainRuntimeSettings::GetQTEType() const
{
	return EQTEType::Chain;
}

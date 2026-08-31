#pragma once

#include "Runtime/QTERuntime.h"

class QTE_API FQTEMashRuntime final : public FQTERuntime
{
public:
	using FQTERuntime::FQTERuntime;

	void Start() override;
	bool HandlePressed(const UInputAction* InputAction) override;
	bool HandleReleased(const UInputAction* InputAction) override;
	bool HandleTriggered(const UInputAction* InputAction) override;

private:
	void RefreshStepProgress(const FQTEStepDefinition& Step) override;
};

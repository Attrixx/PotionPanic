#pragma once

#include "Modules/ModuleManager.h"

class FOrdersModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};

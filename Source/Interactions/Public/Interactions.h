#pragma once

#include "Modules/ModuleManager.h"

class FInteractionsModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

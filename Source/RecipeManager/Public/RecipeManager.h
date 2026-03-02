#pragma once

#include "Modules/ModuleManager.h"

class FRecipeManagerModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

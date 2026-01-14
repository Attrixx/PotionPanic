#pragma once

#include "Modules/ModuleManager.h"

class FRecipesModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

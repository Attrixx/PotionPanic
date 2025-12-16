#pragma once

#include "Modules/ModuleManager.h"

class FCoreGameplayModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

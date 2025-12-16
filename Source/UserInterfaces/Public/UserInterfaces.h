#pragma once

#include "Modules/ModuleManager.h"

class FUserInterfacesModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

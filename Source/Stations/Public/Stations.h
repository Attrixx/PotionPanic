#pragma once

#include "Modules/ModuleManager.h"

class FStationsModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

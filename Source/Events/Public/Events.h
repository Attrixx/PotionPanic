#pragma once

#include "Modules/ModuleManager.h"

class FEventsModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

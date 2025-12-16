#pragma once

#include "Modules/ModuleManager.h"

class FGameFlowModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

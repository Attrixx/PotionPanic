#pragma once

#include "Modules/ModuleManager.h"

class FLobbyModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

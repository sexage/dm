#include "OfflineConfig.hpp"

static OfflineConfig* g_pOfflineConfig = nullptr;

OfflineConfig::OfflineConfig()
{
	if (!g_pOfflineConfig)
		g_pOfflineConfig = this;
}

OfflineConfig* GetOfflineConfig()
{
	return g_pOfflineConfig;
}

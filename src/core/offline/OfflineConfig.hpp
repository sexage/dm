#pragma once

#include <string>

class OfflineConfig
{
public:
	OfflineConfig();

	// Enable/disable offline mode
	void SetEnabled(bool enabled) { m_bEnabled = enabled; }
	bool IsEnabled() const { return m_bEnabled; }

	// Storage path
	void SetStoragePath(const std::string& path) { m_storagePath = path; }
	const std::string& GetStoragePath() const { return m_storagePath; }

	// Cache settings
	void SetMaxCachedMessages(int max) { m_maxCachedMessages = max; }
	int GetMaxCachedMessages() const { return m_maxCachedMessages; }

	void SetCacheOlderThanDays(int days) { m_cacheOlderThanDays = days; }
	int GetCacheOlderThanDays() const { return m_cacheOlderThanDays; }

	// Auto-sync settings
	void SetAutoSync(bool enable) { m_bAutoSync = enable; }
	bool IsAutoSyncEnabled() const { return m_bAutoSync; }

	void SetSyncInterval(int seconds) { m_syncInterval = seconds; }
	int GetSyncInterval() const { return m_syncInterval; }

	// Cleanup settings
	void SetAutoCleanup(bool enable) { m_bAutoCleanup = enable; }
	bool IsAutoCleanupEnabled() const { return m_bAutoCleanup; }

	void SetCleanupInterval(int seconds) { m_cleanupInterval = seconds; }
	int GetCleanupInterval() const { return m_cleanupInterval; }

private:
	bool m_bEnabled = false;
	std::string m_storagePath = "";
	int m_maxCachedMessages = 10000; // Maximum messages per channel
	int m_cacheOlderThanDays = 30;   // Don't cache older than 30 days
	bool m_bAutoSync = true;
	int m_syncInterval = 300; // 5 minutes
	bool m_bAutoCleanup = true;
	int m_cleanupInterval = 3600; // 1 hour
};

OfflineConfig* GetOfflineConfig();

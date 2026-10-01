#pragma once

#include <string>
#include <vector>
#include <ctime>
#include "../models/Snowflake.hpp"

class SyncManager
{
public:
	enum eSyncStatus
	{
		SYNC_IDLE,
		SYNC_IN_PROGRESS,
		SYNC_COMPLETED,
		SYNC_FAILED,
	};

	SyncManager();
	~SyncManager();

	// Initialize sync manager
	void Init();
	void Kill();

	// Check online status and trigger sync if needed
	void CheckAndSync();

	// Manual sync trigger
	void StartSync(Snowflake channel);
	void StartFullSync();

	// Get current status
	eSyncStatus GetSyncStatus() const { return m_status; }
	bool IsSyncing() const { return m_status == SYNC_IN_PROGRESS; }

	// Callbacks for network operations
	void OnNetworkOnline();
	void OnNetworkOffline();
	bool IsOnline() const { return m_bIsOnline; }

	// Sync configuration
	void SetAutoSync(bool enable) { m_bAutoSync = enable; }
	void SetSyncInterval(int seconds) { m_syncInterval = seconds; }

	// Get sync progress
	float GetSyncProgress() const { return m_syncProgress; }
	int GetSyncedChannels() const { return m_syncedChannels; }

private:
	eSyncStatus m_status = SYNC_IDLE;
	bool m_bIsOnline = true;
	bool m_bAutoSync = true;
	int m_syncInterval = 300; // 5 minutes default
	float m_syncProgress = 0.0f;
	int m_syncedChannels = 0;
	time_t m_lastSyncTime = 0;

	// Sync a single channel
	bool SyncChannel(Snowflake channel);

	// Retry unsent messages
	bool RetryUnsentMessages(Snowflake channel);

	// Merge remote and local messages
	void MergeMessages(Snowflake channel);
};

SyncManager* GetSyncManager();

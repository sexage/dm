#include "SyncManager.hpp"
#include "OfflineMessageCache.hpp"
#include <ctime>

static SyncManager* g_pSyncManager = nullptr;

SyncManager::SyncManager()
{
}

SyncManager::~SyncManager()
{
	Kill();
}

void SyncManager::Init()
{
	m_status = SYNC_IDLE;
	m_bIsOnline = true;
	m_lastSyncTime = std::time(nullptr);
	g_pSyncManager = this;
}

void SyncManager::Kill()
{
	g_pSyncManager = nullptr;
}

void SyncManager::CheckAndSync()
{
	if (!m_bIsOnline || !m_bAutoSync || m_status == SYNC_IN_PROGRESS)
		return;

	time_t now = std::time(nullptr);
	if (now - m_lastSyncTime >= m_syncInterval)
	{
		StartFullSync();
	}
}

void SyncManager::StartSync(Snowflake channel)
{
	if (!m_bIsOnline || m_status == SYNC_IN_PROGRESS)
		return;

	m_status = SYNC_IN_PROGRESS;
	m_syncProgress = 0.0f;

	if (SyncChannel(channel))
	{
		m_status = SYNC_COMPLETED;
		m_lastSyncTime = std::time(nullptr);
	}
	else
	{
		m_status = SYNC_FAILED;
	}
}

void SyncManager::StartFullSync()
{
	if (!m_bIsOnline || m_status == SYNC_IN_PROGRESS)
		return;

	m_status = SYNC_IN_PROGRESS;
	m_syncProgress = 0.0f;
	m_syncedChannels = 0;

	OfflineMessageCache* pCache = GetOfflineMessageCache();
	if (pCache)
	{
		// Get all loaded channels and sync them
		// This will be implemented based on actual Discord API
	}

	m_status = SYNC_COMPLETED;
	m_lastSyncTime = std::time(nullptr);
}

void SyncManager::OnNetworkOnline()
{
	m_bIsOnline = true;
	if (m_bAutoSync)
	{
		StartFullSync();
	}
}

void SyncManager::OnNetworkOffline()
{
	m_bIsOnline = false;
	m_status = SYNC_IDLE;
}

bool SyncManager::SyncChannel(Snowflake channel)
{
	OfflineMessageCache* pCache = GetOfflineMessageCache();
	if (!pCache)
		return false;

	// Retry unsent messages first
	if (!RetryUnsentMessages(channel))
		return false;

	// Fetch new messages from server
	// This will be connected to the Discord API
	// For now, we just update the sync timestamp
	pCache->UpdateLastSync(channel);

	return true;
}

bool SyncManager::RetryUnsentMessages(Snowflake channel)
{
	OfflineMessageCache* pCache = GetOfflineMessageCache();
	if (!pCache)
		return false;

	std::vector<Snowflake> unsentMsgs;
	if (!pCache->GetUnsentMessages(channel, unsentMsgs))
		return true; // No unsent messages

	// Retry sending each unsent message
	// This will be connected to the Discord API
	for (const auto& msgId : unsentMsgs)
	{
		// Attempt to send the message
		// On success: pCache->MarkMessageSent(channel, msgId)
		// On failure: Keep it in unsent list
	}

	return true;
}

void SyncManager::MergeMessages(Snowflake channel)
{
	// Merge remote and local messages
	// Resolve conflicts:
	// - If message exists locally but not remotely, keep local (user created it offline)
	// - If message exists remotely but not locally, add it
	// - If both exist, check timestamps and keep the latest
}

SyncManager* GetSyncManager()
{
	return g_pSyncManager;
}

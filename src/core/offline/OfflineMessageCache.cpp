#include "OfflineMessageCache.hpp"
#include <ctime>

static OfflineMessageCache* g_pOfflineMessageCache = nullptr;

OfflineMessageCache::OfflineMessageCache()
{
	m_pMessageCache = GetMessageCache();
}

OfflineMessageCache::~OfflineMessageCache()
{
}

void OfflineMessageCache::InitOfflineStorage(const std::string& storagePath)
{
	if (!m_pStorage)
	{
		m_pStorage = new OfflineStorage();
		m_pStorage->Init(storagePath);
	}
}

void OfflineMessageCache::EnableOfflineMode(bool enable)
{
	m_bOfflineEnabled = enable;
}

bool OfflineMessageCache::IsOfflineModeEnabled() const
{
	return m_bOfflineEnabled;
}

void OfflineMessageCache::AddMessage(Snowflake channel, const Message& msg)
{
	m_pMessageCache->AddMessage(channel, msg);

	// Save to offline storage if enabled
	if (m_bOfflineEnabled && m_pStorage)
	{
		m_pStorage->SaveMessage(channel, msg);
	}
}

void OfflineMessageCache::EditMessage(Snowflake channel, const Message& msg)
{
	m_pMessageCache->EditMessage(channel, msg);

	// Update in offline storage if enabled
	if (m_bOfflineEnabled && m_pStorage)
	{
		m_pStorage->SaveMessage(channel, msg);
	}
}

void OfflineMessageCache::DeleteMessage(Snowflake channel, Snowflake message)
{
	m_pMessageCache->DeleteMessage(channel, message);

	// Delete from offline storage if enabled
	if (m_bOfflineEnabled && m_pStorage)
	{
		m_pStorage->DeleteMessage(channel, message);
	}
}

void OfflineMessageCache::GetLoadedMessages(Snowflake channel, Snowflake guild, std::list<MessagePtr>& out)
{
	m_pMessageCache->GetLoadedMessages(channel, guild, out);
}

bool OfflineMessageCache::IsMessageLoaded(Snowflake channel, Snowflake message)
{
	return m_pMessageCache->IsMessageLoaded(channel, message);
}

MessagePtr OfflineMessageCache::GetLoadedMessage(Snowflake channel, Snowflake message)
{
	return m_pMessageCache->GetLoadedMessage(channel, message);
}

bool OfflineMessageCache::LoadOfflineMessages(Snowflake channel)
{
	if (!m_pStorage)
		return false;

	std::vector<MessagePtr> messages;
	if (!m_pStorage->LoadMessages(channel, messages))
		return false;

	// Add loaded messages to in-memory cache
	for (const auto& msg : messages)
	{
		m_pMessageCache->AddMessage(channel, *msg);
	}

	return true;
}

void OfflineMessageCache::UpdateLastSync(Snowflake channel)
{
	if (m_pStorage)
	{
		time_t now = std::time(nullptr);
		m_pStorage->SetLastSync(channel, now);
	}
}

time_t OfflineMessageCache::GetLastSync(Snowflake channel) const
{
	if (m_pStorage)
		return m_pStorage->GetLastSync(channel);
	return 0;
}

void OfflineMessageCache::MarkMessageUnsent(Snowflake channel, Snowflake msgId)
{
	if (m_pStorage)
	{
		m_pStorage->MarkAsUnsent(channel, msgId);
	}
}

void OfflineMessageCache::MarkMessageSent(Snowflake channel, Snowflake msgId)
{
	if (m_pStorage)
	{
		m_pStorage->MarkAsSent(channel, msgId);
	}
}

bool OfflineMessageCache::GetUnsentMessages(Snowflake channel, std::vector<Snowflake>& outMsgIds) const
{
	if (m_pStorage)
		return m_pStorage->GetUnsentMessages(channel, outMsgIds);
	return false;
}

OfflineMessageCache* GetOfflineMessageCache()
{
	return g_pOfflineMessageCache;
}

#pragma once

#include "../state/MessageCache.hpp"
#include "OfflineStorage.hpp"
#include <vector>

class OfflineMessageCache
{
public:
	OfflineMessageCache();
	~OfflineMessageCache();

	void InitOfflineStorage(const std::string& storagePath);
	void EnableOfflineMode(bool enable);
	bool IsOfflineModeEnabled() const;

	// Wrapper methods for MessageCache with offline persistence
	void AddMessage(Snowflake channel, const Message& msg);
	void EditMessage(Snowflake channel, const Message& msg);
	void DeleteMessage(Snowflake channel, Snowflake message);
	void GetLoadedMessages(Snowflake channel, Snowflake guild, std::list<MessagePtr>& out);
	bool IsMessageLoaded(Snowflake channel, Snowflake message);
	MessagePtr GetLoadedMessage(Snowflake channel, Snowflake message);

	// Load messages from offline storage
	bool LoadOfflineMessages(Snowflake channel);

	// Sync metadata
	void UpdateLastSync(Snowflake channel);
	time_t GetLastSync(Snowflake channel) const;

	// Unsent messages (created while offline)
	void MarkMessageUnsent(Snowflake channel, Snowflake msgId);
	void MarkMessageSent(Snowflake channel, Snowflake msgId);
	bool GetUnsentMessages(Snowflake channel, std::vector<Snowflake>& outMsgIds) const;

	// Access to underlying cache
	MessageCache* GetInnerCache() { return m_pMessageCache; }

private:
	MessageCache* m_pMessageCache = nullptr;
	OfflineStorage* m_pStorage = nullptr;
	bool m_bOfflineEnabled = false;
};

OfflineMessageCache* GetOfflineMessageCache();

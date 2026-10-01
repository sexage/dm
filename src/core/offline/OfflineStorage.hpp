#pragma once

#include <string>
#include <map>
#include <nlohmann/json.h>
#include "../models/Message.hpp"
#include "../models/Snowflake.hpp"

class OfflineStorage
{
public:
	OfflineStorage();
	~OfflineStorage();

	void Init(const std::string& basePath);
	void Kill();

	// Message operations
	bool SaveMessage(Snowflake channel, const Message& msg);
	bool SaveMessages(Snowflake channel, const std::vector<MessagePtr>& messages);
	bool LoadMessages(Snowflake channel, std::vector<MessagePtr>& outMessages);
	bool DeleteMessage(Snowflake channel, Snowflake msgId);
	bool ClearChannel(Snowflake channel);

	// Metadata
	bool SetLastSync(Snowflake channel, time_t timestamp);
	time_t GetLastSync(Snowflake channel);
	bool IsOfflineAvailable(Snowflake channel) const;

	// Sync tracking
	bool MarkAsUnsent(Snowflake channel, Snowflake msgId);
	bool GetUnsentMessages(Snowflake channel, std::vector<Snowflake>& outMsgIds);
	bool MarkAsSent(Snowflake channel, Snowflake msgId);

private:
	std::string m_basePath;
	std::string m_messageDir;
	std::string m_metadataPath;

	std::string GetChannelPath(Snowflake channel) const;
	std::string GetMessageFile(Snowflake channel, Snowflake msgId) const;
	std::string GetMetadataFile() const;

	nlohmann::json LoadMetadata() const;
	void SaveMetadata(const nlohmann::json& metadata);
	bool EnsureDirectories();
};

OfflineStorage* GetOfflineStorage();

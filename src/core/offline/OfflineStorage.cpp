#include "OfflineStorage.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <ctime>

namespace fs = std::filesystem;
using json = nlohmann::json;

static OfflineStorage* g_pOfflineStorage = nullptr;

OfflineStorage::OfflineStorage()
{
}

OfflineStorage::~OfflineStorage()
{
	Kill();
}

void OfflineStorage::Init(const std::string& basePath)
{
	m_basePath = basePath;
	m_messageDir = basePath + "/messages";
	m_metadataPath = basePath + "/metadata.json";

	if (!EnsureDirectories())
		return;

	g_pOfflineStorage = this;
}

void OfflineStorage::Kill()
{
	g_pOfflineStorage = nullptr;
}

bool OfflineStorage::EnsureDirectories()
{
	try
	{
		if (!fs::exists(m_basePath))
			fs::create_directories(m_basePath);

		if (!fs::exists(m_messageDir))
			fs::create_directories(m_messageDir);

		return true;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

std::string OfflineStorage::GetChannelPath(Snowflake channel) const
{
	return m_messageDir + "/" + std::to_string(channel);
}

std::string OfflineStorage::GetMessageFile(Snowflake channel, Snowflake msgId) const
{
	return GetChannelPath(channel) + "/" + std::to_string(msgId) + ".json";
}

std::string OfflineStorage::GetMetadataFile() const
{
	return m_metadataPath;
}

bool OfflineStorage::SaveMessage(Snowflake channel, const Message& msg)
{
	try
	{
		std::string channelPath = GetChannelPath(channel);
		if (!fs::exists(channelPath))
			fs::create_directories(channelPath);

		json msgJson;
		msgJson["snowflake"] = msg.m_snowflake;
		msgJson["author_snowflake"] = msg.m_author_snowflake;
		msgJson["message"] = msg.m_message;
		msgJson["author"] = msg.m_author;
		msgJson["avatar"] = msg.m_avatar;
		msgJson["date_time"] = msg.m_dateTime;
		msgJson["time_edited"] = msg.m_timeEdited;
		msgJson["is_pinned"] = msg.m_bIsPinned;
		msgJson["type"] = msg.m_type;

		std::string filePath = GetMessageFile(channel, msg.m_snowflake);
		std::ofstream file(filePath);
		file << msgJson.dump(2);
		file.close();

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool OfflineStorage::SaveMessages(Snowflake channel, const std::vector<MessagePtr>& messages)
{
	for (const auto& msg : messages)
	{
		if (!SaveMessage(channel, *msg))
			return false;
	}
	return true;
}

bool OfflineStorage::LoadMessages(Snowflake channel, std::vector<MessagePtr>& outMessages)
{
	try
	{
		std::string channelPath = GetChannelPath(channel);
		if (!fs::exists(channelPath))
			return true;

		for (const auto& entry : fs::directory_iterator(channelPath))
		{
			if (entry.path().extension() != ".json")
				continue;

			std::ifstream file(entry.path());
			json msgJson = json::parse(file);
			file.close();

			auto msg = MakeMessage();
			msg->m_snowflake = msgJson.value("snowflake", 0);
			msg->m_author_snowflake = msgJson.value("author_snowflake", 0);
			msg->m_message = msgJson.value("message", "");
			msg->m_author = msgJson.value("author", "");
			msg->m_avatar = msgJson.value("avatar", "");
			msg->m_dateTime = msgJson.value("date_time", 0);
			msg->m_timeEdited = msgJson.value("time_edited", 0);
			msg->m_bIsPinned = msgJson.value("is_pinned", false);
			msg->m_type = msgJson.value("type", 0);

			outMessages.push_back(msg);
		}

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool OfflineStorage::DeleteMessage(Snowflake channel, Snowflake msgId)
{
	try
	{
		std::string filePath = GetMessageFile(channel, msgId);
		if (fs::exists(filePath))
			fs::remove(filePath);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool OfflineStorage::ClearChannel(Snowflake channel)
{
	try
	{
		std::string channelPath = GetChannelPath(channel);
		if (fs::exists(channelPath))
			fs::remove_all(channelPath);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool OfflineStorage::SetLastSync(Snowflake channel, time_t timestamp)
{
	try
	{
		json metadata = LoadMetadata();
		metadata["last_sync"][std::to_string(channel)] = timestamp;
		SaveMetadata(metadata);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

time_t OfflineStorage::GetLastSync(Snowflake channel)
{
	try
	{
		json metadata = LoadMetadata();
		if (metadata.contains("last_sync") && metadata["last_sync"].contains(std::to_string(channel)))
			return metadata["last_sync"][std::to_string(channel)];
	}
	catch (const std::exception&)
	{
	}
	return 0;
}

bool OfflineStorage::IsOfflineAvailable(Snowflake channel) const
{
	return fs::exists(GetChannelPath(channel));
}

bool OfflineStorage::MarkAsUnsent(Snowflake channel, Snowflake msgId)
{
	try
	{
		json metadata = LoadMetadata();
		metadata["unsent_messages"][std::to_string(channel)].push_back(msgId);
		SaveMetadata(metadata);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool OfflineStorage::GetUnsentMessages(Snowflake channel, std::vector<Snowflake>& outMsgIds)
{
	try
	{
		json metadata = LoadMetadata();
		if (metadata.contains("unsent_messages") && metadata["unsent_messages"].contains(std::to_string(channel)))
		{
			for (const auto& id : metadata["unsent_messages"][std::to_string(channel)])
			{
				outMsgIds.push_back(id.get<Snowflake>());
			}
		}
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool OfflineStorage::MarkAsSent(Snowflake channel, Snowflake msgId)
{
	try
	{
		json metadata = LoadMetadata();
		if (metadata.contains("unsent_messages") && metadata["unsent_messages"].contains(std::to_string(channel)))
		{
			auto& arr = metadata["unsent_messages"][std::to_string(channel)];
			arr.erase(
				std::remove_if(arr.begin(), arr.end(),
					[msgId](const json& j) { return j.get<Snowflake>() == msgId; }),
				arr.end()
			);
		}
		SaveMetadata(metadata);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

json OfflineStorage::LoadMetadata() const
{
	try
	{
		if (fs::exists(m_metadataPath))
		{
			std::ifstream file(m_metadataPath);
			return json::parse(file);
		}
	}
	catch (const std::exception&)
	{
	}
	return json::object();
}

void OfflineStorage::SaveMetadata(const json& metadata)
{
	try
	{
		std::ofstream file(m_metadataPath);
		file << metadata.dump(2);
		file.close();
	}
	catch (const std::exception&)
	{
	}
}

OfflineStorage* GetOfflineStorage()
{
	return g_pOfflineStorage;
}

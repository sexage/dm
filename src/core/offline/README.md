# Offline Mode Module

Discord Messenger'in çevrimdışı (offline) mod desteği.

## Genel Bakış

Offline modu, kullanıcıların internet bağlantısı olmadan Discord mesajlarını okuyabilmelerini sağlar. Bağlantı geri geldiğinde, sistem otomatik olarak yeni mesajları indirir ve unsent (gönderilmemiş) mesajları yeniden göndermeye çalışır.

## Mimarı

### Bileşenler

#### 1. OfflineStorage
- Mesajları JSON dosyaları olarak diskette saklar
- Metadata (son sync zamanı, unsent mesajlar vb.) yönetir
- `AppData/Discord Messenger/offline/` dizininde saklanır

**Dosya Yapısı:**
```
offline/
├── messages/
│   ├── {channel_id}/
│   │   ├── {message_id}.json
│   │   └── ...
│   └── ...
└── metadata.json
```

#### 2. OfflineMessageCache
- OfflineStorage'ı sarmalayan (wrapper) sınıf
- Belleğe yüklü mesajları saklar
- `AddMessage()`, `EditMessage()`, `DeleteMessage()` çağrıldığında otomatik olarak diskette kaydeder

#### 3. SyncManager
- Ağ durumunu takip eder
- Çevrim dışıyken kaydedilen mesajları çevrim içi olduğunda senkronize eder
- Unsent mesajları yeniden göndermeyi dener
- Çakışmaları çözer (conflict resolution)

#### 4. OfflineConfig
- Offline mode ayarlarını yönetir
- Cache boyutu, sync aralığı, vb. yapılandırılabilir

## Kullanım

### Başlatma

```cpp
// DiscordInstance başlatılırken
OfflineConfig* pConfig = new OfflineConfig();
pConfig->SetEnabled(true);
pConfig->SetStoragePath(appDataPath + "/offline");

OfflineMessageCache* pOfflineCache = new OfflineMessageCache();
pOfflineCache->InitOfflineStorage(pConfig->GetStoragePath());
pOfflineCache->EnableOfflineMode(true);

SyncManager* pSyncManager = new SyncManager();
pSyncManager->Init();
pSyncManager->SetAutoSync(true);
pSyncManager->SetSyncInterval(300); // 5 dakika
```

### Mesaj Ekleme (Offline Olurken)

```cpp
Message msg = CreateMessage(...);
pOfflineCache->AddMessage(channel, msg);
// Otomatik olarak dosyaya kaydedilir
```

### Network Durumu Değişikliğinde

```cpp
// İnternet bağlantısı sağlanırsa
pSyncManager->OnNetworkOnline();

// İnternet bağlantısı kesilirse
pSyncManager->OnNetworkOffline();
```

### Unsent Mesajları Yeniden Gönderme

```cpp
std::vector<Snowflake> unsentMsgs;
pOfflineCache->GetUnsentMessages(channel, unsentMsgs);

for (const auto& msgId : unsentMsgs)
{
    Message* msg = pOfflineCache->GetLoadedMessage(channel, msgId);
    // Discord API'ye gönder
    if (successful)
        pOfflineCache->MarkMessageSent(channel, msgId);
}
```

## Ayarlandırılabilir Parametreler

- **enabled**: Offline modu etkinleştir/devre dış bırak
- **maxCachedMessages**: Kanal başına maksimum cache edilecek mesaj sayısı (default: 10000)
- **cacheOlderThanDays**: Belirtilen günden eski mesajları cache etme (default: 30)
- **autoSync**: Ağ geri geldiğinde otomatik senkronize et (default: true)
- **syncInterval**: Otomatik senkronizasyon aralığı (default: 300 saniye)
- **autoCleanup**: Eski cache'leri otomatik temizle (default: true)
- **cleanupInterval**: Cleanup işlemi aralığı (default: 3600 saniye)

## Depo Formatı

### message JSON şeması
```json
{
  "snowflake": "message_id",
  "author_snowflake": "user_id",
  "message": "message content",
  "author": "username",
  "avatar": "avatar url",
  "date_time": 1234567890,
  "time_edited": 0,
  "is_pinned": false,
  "type": 0
}
```

### metadata.json şeması
```json
{
  "last_sync": {
    "channel_id": 1234567890
  },
  "unsent_messages": {
    "channel_id": [message_id, ...]
  }
}
```

## Gelecek İyileştirmeler

1. **SQLite Veritabanı**: JSON dosyaları yerine SQLite kullan (daha hızlı sorgulamalar)
2. **Delta Sync**: Sadece değişen mesajları senkronize et
3. **Compression**: Depolama alanını azaltmak için compression
4. **Encrypted Storage**: Güvenlik için local verileri şifrele
5. **Selective Caching**: Kullanıcı seçebileceği kanalları cache et
6. **Cache Statistics**: Cache boyutu, hit rate vb. istatistikler

## Bilinen Sınırlamalar

- Embed'ler ve attachments'lar şu anda tam olarak saklanmıyor
- Reaksiyon (reactions) cache edilmiyor
- Threads şu anda desteklenmiyor
- Voice/video mesajları cache edilmiyor

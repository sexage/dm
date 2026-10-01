# Offline Mode Integration Guide

Bu belge, Offline Mode modülünün mevcut Discord Messenger codebase'ine nasıl entegre edileceğini açıklar.

## Adım 1: Makefile'a Offline Files Ekleme

`Makefile` açın ve `src/core/` altında bulunan dosyalara offline dosyalarını ekleyin:

```makefile
# src/core/offline/ dosyaları
OFFLINE_SRCS = \
	src/core/offline/OfflineStorage.cpp \
	src/core/offline/OfflineMessageCache.cpp \
	src/core/offline/SyncManager.cpp \
	src/core/offline/OfflineConfig.cpp

# Existing CORE_SRCS'a ekle
CORE_SRCS = $(EXISTING_CORE_SRCS) $(OFFLINE_SRCS)
```

## Adım 2: DiscordInstance'a Entegrasyon

`src/core/DiscordInstance.hpp` dosyasında:

```cpp
#include "offline/OfflineMessageCache.hpp"
#include "offline/SyncManager.hpp"
#include "offline/OfflineConfig.hpp"

class DiscordInstance
{
private:
    OfflineMessageCache* m_pOfflineCache = nullptr;
    SyncManager* m_pSyncManager = nullptr;
    OfflineConfig* m_pOfflineConfig = nullptr;
    
public:
    // Offline mode başlatma
    void InitOfflineMode(const std::string& storagePath);
    void DisableOfflineMode();
    
    OfflineMessageCache* GetOfflineCache() { return m_pOfflineCache; }
};
```

`src/core/DiscordInstance.cpp` dosyasında:

```cpp
void DiscordInstance::InitOfflineMode(const std::string& storagePath)
{
    m_pOfflineConfig = new OfflineConfig();
    m_pOfflineConfig->SetEnabled(true);
    m_pOfflineConfig->SetStoragePath(storagePath);
    
    m_pOfflineCache = new OfflineMessageCache();
    m_pOfflineCache->InitOfflineStorage(storagePath);
    m_pOfflineCache->EnableOfflineMode(true);
    
    m_pSyncManager = new SyncManager();
    m_pSyncManager->Init();
    m_pSyncManager->SetAutoSync(true);
    m_pSyncManager->SetSyncInterval(300);
}

void DiscordInstance::DisableOfflineMode()
{
    if (m_pOfflineCache)
    {
        m_pOfflineCache->EnableOfflineMode(false);
    }
}
```

## Adım 3: MessageCache Kullanımını Güncelle

Mevcut `AddMessage()` çağrılarını offline cache'e yönlendir:

```cpp
// Eski (sadece bellek)
GetMessageCache()->AddMessage(channel, msg);

// Yeni (bellek + disk)
if (GetDiscordInstance()->GetOfflineCache())
    GetDiscordInstance()->GetOfflineCache()->AddMessage(channel, msg);
else
    GetMessageCache()->AddMessage(channel, msg);
```

## Adım 4: Network Durumu Takibi

HTTP/WebSocket connection events'lerine network durumu callback'i ekle:

```cpp
// WebSocket bağlandığında
void OnWebsocketConnect()
{
    if (GetSyncManager())
        GetSyncManager()->OnNetworkOnline();
}

// WebSocket koptuğunda
void OnWebsocketDisconnect()
{
    if (GetSyncManager())
        GetSyncManager()->OnNetworkOffline();
}
```

## Adım 5: Startup'ta Offline Mesajları Yükle

`Main.cpp` ile application başlatılırken:

```cpp
// Discord instance'ı başlat
DiscordInstance* pDiscord = GetDiscordInstance();

// Offline mode'u başlat
std::string appDataPath = GetAppDataPath();
pDiscord->InitOfflineMode(appDataPath + "/offline");

// Önbelleğe alınan mesajları yükle
// (Bu otomatik olarak server'dan mesajlar indirilirken yapılacak)
```

## Adım 6: Settings UI'a Offline Ayarlarını Ekle

`src/windows/SettingsDialog.cpp`:

```cpp
// Settings penceresinde offline mode seçeneği
AddCheckbox(hDialog, "Enable Offline Mode", 
    GetOfflineConfig()->IsEnabled());

// Cache boyutu ayarı
AddSpinner(hDialog, "Max Cached Messages", 
    GetOfflineConfig()->GetMaxCachedMessages(), 1000, 100000);

// Sync aralığı
AddSpinner(hDialog, "Sync Interval (seconds)", 
    GetOfflineConfig()->GetSyncInterval(), 60, 3600);
```

## Adım 7: Test

Offline mode'u test etmek için:

1. **Offline modunu etkinleştir**: Settings'te offline mode'u aç
2. **Mesaj indir**: Bir kanalı aç, birkaç mesajı yükle
3. **İnternet'i kapat**: Network connection'ı kapat
4. **Offline oku**: Önceden yüklenen mesajleri okunabilir olması gerekir
5. **Mesaj gönder**: Offline olurken bir mesaj gönder (unsent olarak işaretlenir)
6. **İnternet'i aç**: Connection'ı geri aç
7. **Sync ol**: Mesajlar senkronize olmalı, gönderme retry edilmelidir

## Kod Dönüştürme Checklistesi

- [ ] Makefile'a offline sources eklendi
- [ ] DiscordInstance.hpp/cpp güncellendi
- [ ] MessageCache çağrılarını offline cache'e yönlendir
- [ ] Network durumu callbacks eklendi
- [ ] Startup'ta offline mesajları yükle
- [ ] Settings UI'a offline seçenekleri eklendi
- [ ] Compilation testi geçti
- [ ] Runtime testi geçti
- [ ] Offline/Online switching test edildi

## Hatırlatmalar

1. **OfflineMessageCache** bellek ve disk arasında senkronize tutuyor
2. **SyncManager** otomatik senkronizasyon yapıyor
3. **OfflineStorage** JSON dosyalarını `AppData` klasöründe saklar
4. Eski cache'leri temizlemek için **cleanup** işlemi yapılıyor
5. **Conflict resolution** strategy: Daha yeni timestamp'e sahip mesaj kazanır

## Performance Notları

- JSON parsing/serialization bellek yoğunudur
- Büyük sayıda mesajda SQLite'a geçiş önerilir
- Disk I/O'yu azaltmak için batch operations kullan
- Büyük channels için incremental loading kullan

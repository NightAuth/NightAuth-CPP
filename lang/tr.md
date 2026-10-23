# NightAuth C++ SDK — GitHub'da Bize Yıldız Verin ⭐

Resmi **NightAuth** C++17 Header-Only SDK ve Referans İstemci Paketi (Modern **Win32 Pure GDI+** Masaüstü Arayüzü ve Etkileşimli Terminal CLI). C++ yazılımlarınızı tersine mühendislik ve korsanlığa karşı dakikalar içinde koruyun.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Video Eğitimleri & Kurulum Kılavuzları
> NightAuth Yönetim Panelinde uygulama, plan, abonelik, bulut değişkenleri, webhook ve lisansların nasıl yapılandırılacağını anlatan resmi video rehberleri ve adım adım kılavuzlar **yakında yayınlanacaktır**.

---

## 📸 Ekran Görüntüleri

### Masaüstü Uygulaması (Yerel Win32 GDI+ Modern Koyu Tema)

| Giriş ve Kimlik Doğrulama Ekranı | Kullanıcı Paneli ve Abonelik Yönetimi |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Konsol Uygulaması (Etkileşimli Terminal CLI)

| Terminal Giriş Ekranı | Etkileşimli Konsol Paneli |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Önerilen Güvenlik Uygulamaları

* **Yerel İkili Dosya Sanallaştırma:** Yerel C++ ikili dosyaları IDA Pro, Ghidra ve Binary Ninja ile statik analize açıktır. Üretim sürümleri için derlenmiş yürütülebilir dosyaları **VMProtect**, **Themida** veya **CodeVirtualizer** gibi ticari kod sanallaştırıcılar ile koruyun.
* **Derleme Zamanı Dize Şifreleme:** `.rdata` bölümünde hassas dizeleri veya API belirteçlerini düz metin olarak bırakmayın. Dizeleri yalnızca gerektiği anda çözmek için derleme zamanı XOR gizleme şablonları (örn. `skCrypter` veya özel constexpr dize kodlayıcıları) kullanın.
* **Bellekte Güvenli Dosya Yürütme:** Korunan yükleri veya DLL'leri `NightAuthApp.downloadFile(fileId, buffer)` aracılığıyla indirirken, **baytları asla fiziksel diske yazmayın**. Yükleri doğrudan RAM'den yükleyin ve çalıştırın (örn. yansıtıcı PE yükleme veya manuel haritalama) ve ardından arabelleği hemen `SecureZeroMemory()` veya `std::fill` ile temizleyin.
* **Donanım ve Bütünlük Doğrulaması:** Çalışma zamanı kancalamasını ve hata ayıklayıcı bağlanmasını azaltmak için üretim sürümlerinde anti-debugging ve bellek bütünlüğü denetimlerini etkin tutun.

---

## 📦 Gereksinimler ve Derleme

**C++17** veya üstü ve **Visual Studio 2022 / 2026** gerektirir. Birlikte verilen `.vcxproj` dosyaları varsayılan olarak MSVC araç seti `v145` kullanır; `v143` de desteklenir.

Windows bağımlılıkları:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Yerel Windows Kriptografi ve Soketler)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Masaüstü Grafik Arayüz Çizimi)
* Pakete dahil `libcurl` ve `OpenSSL` (veya platformun yerel HTTPS soketleri), `deps/curl_win64` konumundan `libcurl.dll.a;libcrypto.a;libssl.a` olarak içe aktarılır; include yolu `deps/curl_win64/curl-8.23.0_5-win64-mingw/include` dizinini gösterir

> [!IMPORTANT]
> Pakete dahil `libcurl` ile bağlanan bir derlemeyi dağıtırken `libcurl-x64.dll` dosyasını çalıştırılabilir dosyanızın yanına koyun.

### Çözümü MSBuild ile Derleme

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Yapılandırma & Başlatma

NightAuth panelindeki uygulama bilgilerinizi tanımlayın:

```cpp
#define NIGHTAUTH_IMPLEMENTATION
#include "sdk/nightauth.hpp"

// Define your application credentials from the Manage Apps tab of the NightAuth dashboard
NightAuth::Api NightAuthApp(
    /* name    */ "",
    /* apikey  */ "",
    /* version */ "1.0",
    /* token   */ ""
);
```

### Kriptografik El Sıkışmayı Başlatma (`init`)

Tüm işlemlerden önce `init()` çağrılmalıdır. Diffie-Hellman (X25519) anahtar değişimi ve şifreli oturumu kurar. `init()` öncesinde bir aktarım yapılandırılmalıdır (bkz. [Özel aktarımlar](#25-custom-transports--server-key-pinning)); en basit seçenek, birlikte verilen libcurl yardımcılarını kullanır:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(NightAuthApp.api_url));
NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));

NightAuthApp.init();

if (!NightAuthApp.response.success) {
    std::cerr << "Initialization failed: " << NightAuthApp.response.message << "\n";
    return 1;
}
```

---

## 🚀 SDK Kullanım Kılavuzu

### 1. Oturum Kontrolü & Kara Liste Durumu

```cpp
NightAuthApp.init();

if (!NightAuthApp.response.success) {
    std::cerr << "Initialization failed: " << NightAuthApp.response.message << "\n";
    return 1;
}
```

### 2. Kullanıcı Adı & Parola ile Giriş (ve 2FA)

```cpp
if (NightAuthApp.login(username, password, totpCode)) {
    std::cout << "Welcome back, " << NightAuthApp.user.username << "!\n";
    std::cout << "Hardware ID: " << NightAuthApp.user.hwid << "\n";
    std::cout << "Client IP:   " << NightAuthApp.user.ip << "\n";

    for (const auto& sub : NightAuthApp.user.subscriptions) {
        std::cout << "Plan: " << sub.subscription << " | Expires: " << sub.expiry << " | Key: " << sub.key << "\n";
    }
} else {
    std::cerr << "Login failed: " << NightAuthApp.response.message << "\n";
}
```

### 3. Lisans Anahtarı ile Kayıt

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Sadece Lisans Anahtarı ile Giriş (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Abonelik Yükseltme / Anahtar Kullanma (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. Donanım Kimliği Sıfırlama (HWID Reset)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. Şifremi Unuttum (Forgot Password & Reset)

```cpp
// Request password reset email
if (NightAuthApp.forgotPassword(email)) {
    std::cout << "Password recovery email sent. Check your inbox.\n";
}

// Complete password reset with token
if (NightAuthApp.resetPassword(token, newPassword)) {
    std::cout << "Password reset successfully.\n";
}
```

### 8. Kullanıcı Profil Bilgileri (`result.ToUserData()`)

```cpp
const NightAuth::UserData& user = NightAuthApp.user;

std::cout << "Username:    " << user.username << "\n";
std::cout << "Client IP:   " << user.ip << "\n";
std::cout << "Hardware ID: " << user.hwid << "\n";
std::cout << "Registered:  " << user.created_at << "\n";
std::cout << "Last Login:  " << user.last_login_at << "\n";

for (const auto& sub : user.subscriptions) {
    std::cout << "Plan: " << sub.subscription << " | Expires: " << sub.expiry << " | Key: " << sub.key << "\n";
}
```

### 9. Bulut Değişkenleri (Genel)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. Kullanıcı Değişkenleri (User Variables)

```cpp
// Set a user variable
bool setOk = NightAuthApp.setUserVar("discord_tag", "user#0001");

// Retrieve a user variable
std::string tag = NightAuthApp.getUserVar("discord_tag");
std::cout << "Stored Discord: " << tag << "\n";

// Retrieve all user variables
std::vector<std::pair<std::string, std::string>> allVars;
if (NightAuthApp.getUserVariables(allVars)) {
    for (const auto& kv : allVars) {
        std::cout << kv.first << " = " << kv.second << "\n";
    }
}
```

### 11. Uygulama Denetim Günlükleri

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Kullanıcı Engelleme & Cihazı Yasaklama

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Sunucu Taraflı Webhook'lar

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

Dosya eklemek için `NightAuth::WebhookOptions` alan aşırı yüklemesini kullanın (dosya multipart/form-data olarak yüklenir):

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Bellekte Güvenli Dosya İndirme

```cpp
std::string fileBuffer;

if (NightAuthApp.downloadFile("385624", fileBuffer)) {
    std::cout << "Downloaded " << fileBuffer.size() << " bytes securely.\n";
    // Process or execute payload directly in memory (Reflective PE / Shellcode)
    // Securely wipe memory buffer after execution
    SecureZeroMemory(fileBuffer.data(), fileBuffer.size()); // Windows-only (Win32)
}
```

> [!NOTE]
> `SecureZeroMemory()` bir Win32 API'sidir. Taşınabilir bir silme için `std::fill(buffer.begin(), buffer.end(), '\0')` veya `OPENSSL_cleanse(buffer.data(), buffer.size())` kullanın (OpenSSL zaten bağlıdır).

### 15. Canlı Sohbet Kanalları

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Gerçek Zamanlı Olay Dinleyicileri (SSE)

Sunucu tarafı olay akışına, bir `NightAuth::EventMessage` alan callback ile abone olun (alanlar: `id`, `type`, `data`). Callback bir `bool` döndürmelidir: dinlemeyi durdurmak için `false` döndürün. Önce bir akış aktarımı yapılandırılmalıdır (bkz. [Özel aktarımlar](#25-custom-transports--server-key-pinning) içindeki `setStreamTransport`):

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

Ayrıca `Api` örneğinde sunulan callback'leri atayarak yaşam döngüsü bildirimlerine tepki verebilirsiniz:

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. İki Faktörlü Kimlik Doğrulama (2FA)

```cpp
// Setup TOTP 2FA
NightAuth::TwoFactorSetupData setup;
if (NightAuthApp.setup2FA(setup)) {
    std::cout << "2FA Secret: " << setup.secret << "\n";
    std::cout << "QR URI:     " << setup.uri << "\n";
}

// Confirm with 6-digit Authenticator code
NightAuth::TwoFactorConfirmData conf;
NightAuthApp.confirm2FA("123456", conf);

// Disable 2FA
NightAuthApp.disable2FA(password, "123456");
```

### 18. Oturum Yaşam Döngüsü & Çıkış

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Kullanıcı Adını Değiştirme

Kimliği doğrulanmış hesabın kullanıcı adını değiştirir (güvenlik için oturum kapatılır):

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Uygulama İstatistikleri ve Çevrimiçi Kullanıcılar

Uygulamanın toplu istatistiklerini ve oturumu açık kullanıcıların listesini getirir:

```cpp
NightAuth::AppStats stats;
if (NightAuthApp.getStats(stats)) {
    std::cout << "Users: " << stats.total_users << " (active " << stats.active_users
              << ", banned " << stats.banned_users << ")\n";
    std::cout << "Licenses: " << stats.total_licenses << " (used " << stats.used_licenses
              << ", unused " << stats.unused_licenses << ")\n";
    std::cout << "Version: " << stats.version << "\n";
}

for (const auto& u : NightAuthApp.getOnlineUsers()) {
    std::cout << "Online: " << u << "\n";
}
```

Uygulama düzeyi sayaçlarına doğrudan örnek üzerinde `NightAuthApp.app` (`total_users`, `online_users`, `total_keys`, `version` alanlarına sahip bir `NightAuth::AppData`) olarak da erişilebilir. `user_data` / `app_data` takma adları aynı `user` / `app` üyelerine referans verir.

### 21. Erişim Biletleri

`downloadFile` ve `subscribeEvents` tarafından dahili olarak kullanılan, kapsamlı bir işlemi yetkilendiren kısa ömürlü bir bilet ister:

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Dosya Hash Doğrulama

Onaltılık bir SHA-256 özetini sunucuda kayıtlı değerlere karşı doğrular:

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Donanım Kimliği (HWID) Yönetimi

SDK, donanım kimliğini otomatik olarak türetir (hostname + kullanıcı adı, hash'lenmiş). Örnek başına geçersiz kılabilir veya temizleyebilirsiniz:

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

`NightAuth::SecurityProtections` üzerindeki statik yardımcılar:

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. İmzalı Otomatik Güncellemeler

İmzasız otomatik güncelleme devre dışıdır: `applyUpdate()` her zaman başarısız olur. İmzalı sürüm bildirimleri (manifest) dağıtın ve bunları `SignedUpdate` / `SignedUpdateInstaller` ile doğrulayın:

```cpp
// One-shot: fetch the manifest, verify its Ed25519 signature against your pinned
// release key, enforce the stored sequence (anti-rollback), download to a temp
// file and verify its SHA-256 digest. Returns the destination path.
std::string path = NightAuthApp.downloadSignedUpdate(
    "https://example.com/releases/manifest.json",
    pinnedReleaseKeyHex,           // 64 hex chars (32 bytes)
    "C:\\Updates\\MyApp.exe");

// Manual pipeline for full control:
NightAuth::VerifiedUpdate v = NightAuth::SignedUpdate::verify(envelopeJson, pinnedReleaseKeyHex, "MyApp");
NightAuth::SignedUpdate::download(v, "C:\\Updates\\MyApp.exe");

// Sequence storage location (per-application .seq file used for rollback protection):
std::string seqPath = NightAuth::SignedUpdateInstaller::getSequencePath("MyApp");
```

`NightAuth::VerifiedUpdate`, `application`, `version`, `sequence`, `url`, `size`, `expiresAt` ve `digest` (SHA-256 baytları) alanlarını sunar.

### 25. Özel Aktarımlar ve Sunucu Anahtarı Sabitleme

Tüm HTTP trafiği değiştirilebilir aktarımlar üzerinden geçer. Birlikte verilen libcurl yardımcıları varsayılan seçimdir; düz HTTP, yalnızca açık onayla tam loopback konakları dışında reddedilir:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

Kendi sunucunuza barındırmak veya anahtarları döndürmek için `init()` öncesinde kendi sunucu imza / sealed-box anahtarlarınızı sabitleyin (her ikisi de 64 onaltılık karakter olmalıdır):

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

Varsayılan değerler sabitler ve statik üyeler olarak sunulur: `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` ve `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Konsol ve Masaüstü İstemcilerini Çalıştırma

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Lisans

Bu proje **Elastic License 2.0** kapsamında lisanslanmıştır.

* Yazılımı üçüncü taraflara barındırılan veya yönetilen bir hizmet olarak sağlayamazsınız.
* Lisans anahtarı işlevselliğini değiştiremez, devre dışı bırakamaz veya atlayamazsınız.
* Lisans veya telif hakkı bildirimlerini kaldıramazsınız.

Tam metin için [LICENSE](../LICENSE) dosyasına bakın.

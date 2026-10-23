# NightAuth C++ SDK — Поставьте звезду на GitHub ⭐

Официальный **NightAuth** C++17 Header-Only SDK и набор эталонных клиентов (Современный графический интерфейс **Win32 Pure GDI+** и интерактивный терминал CLI). Защитите нативное ПО на C++ от реверс-инжиниринга и пиратства за считанные минуты.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Видеоинструкции и руководства
> Официальные видеоуроки и пошаговые руководства по настройке приложений, тарифов, подписок, переменных, вебхуков и ключей в панели NightAuth **будут опубликованы в ближайшее время**.

---

## 📸 Скриншоты

### Десктопное приложение (Нативный Win32 GDI+ современная темная тема)

| Окно входа и авторизации | Панель управления и подписки |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Консольное приложение (Интерактивный CLI)

| Вход через терминал | Панель управления в консоли |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Рекомендуемые практики безопасности

* **Виртуализация нативных бинарных файлов:** Нативные C++ исполняемые файлы подвержены статическому анализу через IDA Pro, Ghidra и Binary Ninja. Для релизов защищайте скомпилированные файлы коммерческими виртуализаторами кода, такими как **VMProtect**, **Themida** или **CodeVirtualizer**.
* **Шифрование строк во время компиляции:** Не оставляйте важные строки или токены API в открытом виде в секции `.rdata`. Используйте шаблоны XOR-обфускации времени компиляции (например, `skCrypter` или пользовательские constexpr-энкодеры), чтобы расшифровывать строки только в момент их вызова.
* **Безопасное выполнение файлов в оперативной памяти:** При потоковой загрузке защищенных полезных нагрузок или DLL через `NightAuthApp.downloadFile(fileId, buffer)`, **никогда не записывайте байты на диск**. Загружайте и выполняйте полезную нагрузку напрямую из RAM (например, reflective PE loading или manual mapping) и сразу очищайте буфер с помощью `SecureZeroMemory()` или `std::fill`.
* **Проверка аппаратного обеспечения и целостности:** Держите активными проверки anti-debugging, целостности памяти и таймингов в продакшене.

---

## 📦 Требования и Сборка

Требуется **C++17** или новее и **Visual Studio 2022 / 2026**. Прилагаемые файлы `.vcxproj` по умолчанию используют набор инструментов MSVC `v145`; `v143` также поддерживается.

Зависимости для Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Нативная криптография и сокеты Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Отрисовка GUI)
* Включенные `libcurl` и `OpenSSL` (или нативные HTTPS-сокеты платформы), импортируемые из `deps/curl_win64` как `libcurl.dll.a;libcrypto.a;libssl.a`, с include-путём на `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> При распространении сборки, связанной с включённым `libcurl`, размещайте `libcurl-x64.dll` рядом с исполняемым файлом.

### Сборка решения с помощью MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Конфигурация и инициализация

Укажите учетные данные вашего приложения из панели управления NightAuth:

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

### Криптографическое рукопожатие (`init`)

Вызовите `init()` перед вызовом других методов. Устанавливает сеанс с обменом ключами Diffie-Hellman (X25519) и сквозным шифрованием. Перед `init()` необходимо настроить транспорт (см. [Пользовательские транспорты](#25-custom-transports--server-key-pinning)); проще всего использовать встроенные помощники libcurl:

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

## 🚀 Руководство по использованию SDK

### 1. Проверка статуса сессии и черный список

```cpp
NightAuthApp.init();

if (!NightAuthApp.response.success) {
    std::cerr << "Initialization failed: " << NightAuthApp.response.message << "\n";
    return 1;
}
```

### 2. Вход по логину и паролю (и 2FA)

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

### 3. Регистрация с лицензионным ключом

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Вход только по ключу лицензии (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Обновление подписки / Активация ключа (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. Сброс привязки железа (HWID Lock)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. Восстановление пароля (Forgot Password & Reset)

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

### 8. Профиль пользователя (`result.ToUserData()`)

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

### 9. Облачные переменные (Глобальные)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. Пользовательские переменные

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

### 11. Журнал аудита и логи

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Бан пользователя и блокировка устройства

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Серверные вебхуки

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

Чтобы прикрепить файл, используйте перегрузку с `NightAuth::WebhookOptions` (файл загружается как multipart/form-data):

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Безопасная загрузка файлов в RAM

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
> `SecureZeroMemory()` — это Win32 API. Для переносимого стирания используйте `std::fill(buffer.begin(), buffer.end(), '\0')` или `OPENSSL_cleanse(buffer.data(), buffer.size())` (OpenSSL уже подключён).

### 15. Онлайн-чат

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Обработчики событий реального времени (SSE)

Подпишитесь на поток серверных событий с помощью callback, получающего `NightAuth::EventMessage` (поля: `id`, `type`, `data`). Callback должен возвращать `bool`: верните `false`, чтобы прекратить прослушивание. Сначала необходимо настроить потоковый транспорт (см. `setStreamTransport` в [Пользовательские транспорты](#25-custom-transports--server-key-pinning)):

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

Вы также можете реагировать на уведомления жизненного цикла, назначая callbacks, доступные на экземпляре `Api`:

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. Двухфакторная аутентификация (2FA)

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

### 18. Жизненный цикл сессии и выход

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Смена Имени Пользователя

Меняет имя пользователя аутентифицированного аккаунта (сессия закрывается в целях безопасности):

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Статистика Приложения и Пользователи Онлайн

Получает агрегированную статистику приложения и список пользователей с активной сессией:

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

Счётчики уровня приложения также доступны прямо на экземпляре как `NightAuthApp.app` (`NightAuth::AppData` с полями `total_users`, `online_users`, `total_keys`, `version`). Псевдонимы `user_data` / `app_data` ссылаются на те же члены `user` / `app`.

### 21. Билеты Доступа

Запрашивает недолговечный билет, авторизующий операцию с заданной областью (используется внутри `downloadFile` и `subscribeEvents`):

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Проверка Хеша Файла

Проверяет шестнадцатеричный SHA-256 дайджест по значениям, зарегистрированным на сервере:

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Управление Hardware ID

SDK автоматически выводит аппаратный идентификатор (hostname + имя пользователя, хешируются). Его можно переопределить или очистить для экземпляра:

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

Статические помощники в `NightAuth::SecurityProtections`:

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. Подписанные Автообновления

Неподписанное автообновление отключено: `applyUpdate()` всегда завершается ошибкой. Распространяйте подписанные манифесты релизов и проверяйте их с помощью `SignedUpdate` / `SignedUpdateInstaller`:

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

`NightAuth::VerifiedUpdate` предоставляет `application`, `version`, `sequence`, `url`, `size`, `expiresAt` и `digest` (байты SHA-256).

### 25. Пользовательские Транспорты и Закрепление Ключей Сервера

Весь HTTP-трафик проходит через сменные транспорты. Встроенные помощники libcurl — выбор по умолчанию; простой HTTP отклоняется, кроме точных loopback-хостов с явным согласием:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

Для самостоятельного хостинга или ротации ключей закрепите собственные ключи подписи / sealed-box сервера перед `init()` (оба должны быть из 64 шестнадцатеричных символов):

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

Значения по умолчанию доступны как константы и статические члены: `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` и `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Запуск приложений консоли и рабочего стола

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Лицензия

Проект распространяется под лицензией **Elastic License 2.0**.

* Запрещено предоставлять ПО третьим лицам в качестве хостинг-сервиса.
* Запрещено изменять или обходить систему лицензирования.
* Запрещено удалять уведомления об авторских правах.

Полный текст лицензии доступен в файле [LICENSE](../LICENSE).

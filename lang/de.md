# NightAuth C++ SDK — Star uns auf GitHub ⭐

Offizielles **NightAuth** C++17 Header-Only SDK & Referenz-Client-Suite (Moderne Desktop-GUI mit **Win32 Pure GDI+** und interaktiver Terminal-CLI). Schützen Sie Ihre native C++-Software in Minuten vor Reverse Engineering, Piraterie und Manipulation.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Video-Tutorials & Einrichtungsanleitungen
> Offizielle Video-Tutorials und Schritt-für-Schritt-Anleitungen zur Konfiguration von Anwendungen, Tarifen, Abonnements, Cloud-Variablen, Webhooks und Lizenzen im NightAuth-Dashboard **werden in Kürze veröffentlicht**.

---

## 📸 Screenshots

### Desktop-Anwendung (Natives Win32 GDI+ Modern Dark Theme)

| Anmelde- / Authentifizierungsfenster | Dashboard & Abonnementverwaltung |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Konsolenanwendung (Interaktives CLI-Terminal)

| Terminal-Anmeldung | Interaktives Konsolen-Dashboard |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Empfohlene Sicherheitspraktiken

* **Native Binärvirtualisierung:** Native C++-Binärdateien unterliegen statischer Analyse über IDA Pro, Ghidra und Binary Ninja. Schützen Sie kompilierte Executables für Produktions-Releases mit kommerziellen Code-Virtualisierern und Protektoren wie **VMProtect**, **Themida** oder **CodeVirtualizer**.
* **String-Verschlüsselung zur Compile-Zeit:** Hinterlassen Sie keine sensiblen Strings oder API-Token im Klartext im `.rdata`-Bereich. Verwenden Sie XOR-Verschleierungs-Templates zur Compile-Zeit (z. B. `skCrypter` oder eigene constexpr-String-Encoder), um Strings erst im Bedarfsfall zu entschlüsseln.
* **Sichere Dateiausführung im Arbeitsspeicher:** Wenn Sie geschützte Payloads oder DLLs über `NightAuthApp.downloadFile(fileId, buffer)` streamen, **schreiben Sie niemals Bytes auf den physischen Speicher oder die Festplatte**. Laden und führen Sie Payloads direkt aus dem RAM aus (z. B. reflective PE-Loading oder manuelles Map-Injection) und überschreiben Sie den Puffer sofort mit `SecureZeroMemory()` oder `std::fill`.
* **Hardware- & Integritätsprüfung:** Halten Sie Anti-Debugging-, Speicherintegritäts- und Timing-Prüfungen in Produktions-Releases aktiv, um Runtime-Hooking und Debugger-Anhänge zu erschweren.

---

## 📦 Anforderungen und Erstellung

Erfordert **C++17** oder neuer und **Visual Studio 2022 / 2026**. Die mitgelieferten `.vcxproj`-Dateien nutzen standardmäßig das MSVC-Toolset `v145`; `v143` wird ebenfalls unterstützt.

Windows-Abhängigkeiten:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Native Windows-Krypto und Sockets)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Desktop-GUI-Rendering)
* Gebündeltes `libcurl` und `OpenSSL` (oder native HTTPS-Sockets der Plattform), importiert aus `deps/curl_win64` als `libcurl.dll.a;libcrypto.a;libssl.a`, mit dem Include-Pfad auf `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> Wenn Sie einen Build verteilen, der das gebündelte `libcurl` linkt, legen Sie `libcurl-x64.dll` neben Ihre ausführbare Datei.

### Lösung mit MSBuild kompilieren

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Konfiguration & Initialisierung

Definieren Sie Ihre Zugangsdaten aus dem NightAuth-Dashboard:

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

### Kryptografischen Handshake initialisieren (`init`)

Rufen Sie `init()` vor allen anderen Aufrufen auf. Dies baut den Schlüsselaustausch (X25519) und die verschlüsselte Sitzung auf. Vor `init()` muss ein Transport konfiguriert sein (siehe [Benutzerdefinierte Transports](#25-custom-transports--server-key-pinning)); die einfachste Variante nutzt die gebündelten libcurl-Helfer:

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

## 🚀 SDK-Nutzungsleitfaden

### 1. Sitzungsprüfung & Blacklist-Status

```cpp
// Validate active session status with the server
if (NightAuthApp.check()) {
    std::cout << "Session active.\n";
}

// Check if client HWID or IP address is blacklisted
if (NightAuthApp.checkBlacklist()) {
    std::cerr << "Access Denied: Machine or IP is blacklisted.\n";
    return 1;
}
```

### 2. Anmeldung mit Benutzername & Passwort (und 2FA)

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

### 3. Registrierung mit Lizenzschlüssel

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Nur mit Lizenzschlüssel anmelden (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Abonnement erweitern / Schlüssel einlösen (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. Hardware-ID zurücksetzen (HWID Lock)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. Passwort vergessen (Forgot Password & Reset)

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

### 8. Benutzerprofildaten (`result.ToUserData()`)

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

### 9. Cloud-Variablen (Global)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. Benutzervariablen (User Variables)

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

### 11. Anwendungs-Audit-Logs

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Benutzer sperren & Maschine sperren

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Server-Webhooks

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

Um eine Datei anzuhängen, verwenden Sie die Überladung mit `NightAuth::WebhookOptions` (die Datei wird als multipart/form-data hochgeladen):

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Sicheres Herunterladen im Arbeitsspeicher

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
> `SecureZeroMemory()` ist eine Win32-API. Für ein portables Löschen verwenden Sie `std::fill(buffer.begin(), buffer.end(), '\0')` oder `OPENSSL_cleanse(buffer.data(), buffer.size())` (OpenSSL ist bereits verlinkt).

### 15. Echtzeit-Chatkanäle

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Echtzeit-Ereignishandler (SSE)

Abonnieren Sie den Server-Sent-Event-Stream mit einem Callback, der eine `NightAuth::EventMessage` empfängt (Felder: `id`, `type`, `data`). Der Callback muss `bool` zurückgeben: `false` stoppt den Stream. Vorher muss ein Stream-Transport konfiguriert sein (siehe `setStreamTransport` unter [Benutzerdefinierte Transports](#25-custom-transports--server-key-pinning)):

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

Sie können auch auf Lebenszyklus-Benachrichtigungen reagieren, indem Sie die an der `Api`-Instanz verfügbaren Callbacks zuweisen:

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. Zwei-Faktor-Authentifizierung (2FA)

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

### 18. Sitzungslebenszyklus & Abmeldung

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Benutzernamen Ändern

Ändert den Benutzernamen des authentifizierten Kontos (die Sitzung wird aus Sicherheitsgründen geschlossen):

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Anwendungsstatistiken & Online-Benutzer

Ruft aggregierte Anwendungsmetriken und die Liste der Benutzer mit aktiver Sitzung ab:

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

Anwendungsweite Zähler sind direkt an der Instanz als `NightAuthApp.app` verfügbar (ein `NightAuth::AppData` mit `total_users`, `online_users`, `total_keys`, `version`). Die Aliase `user_data` / `app_data` verweisen auf dieselben Member `user` / `app`.

### 21. Zugriffstickets

Fordert ein kurzlebiges Ticket an, das eine bereichsbezogene Operation autorisiert (intern von `downloadFile` und `subscribeEvents` verwendet):

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Datei-Hash Verifizieren

Verifiziert einen SHA-256-Hex-Digest gegen die serverseitig registrierten Werte:

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Hardware-ID-Verwaltung

Das SDK leitet automatisch eine Hardware-ID ab (Hostname + Benutzername, gehasht). Sie können sie pro Instanz überschreiben oder löschen:

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

Statische Hilfsmethoden auf `NightAuth::SecurityProtections`:

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. Signierte Selbst-Updates

Unsigned Auto-Update ist deaktiviert: `applyUpdate()` schlägt immer fehl. Verteilen Sie signierte Release-Manifeste und verifizieren Sie sie mit `SignedUpdate` / `SignedUpdateInstaller`:

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

`NightAuth::VerifiedUpdate` stellt `application`, `version`, `sequence`, `url`, `size`, `expiresAt` und `digest` (SHA-256-Bytes) bereit.

### 25. Benutzerdefinierte Transports & Server-Key-Pinning

Der gesamte HTTP-Verkehr läuft über austauschbare Transports. Die gebündelten libcurl-Helper sind die Standardwahl; Plain-HTTP wird nur für exakte Loopback-Hosts mit explizitem Opt-in akzeptiert:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

Zum Self-Hosting oder Schlüsselwechsel pinnen Sie vor `init()` Ihre eigenen Server-Signier- / Sealed-Box-Schlüssel (beide müssen 64 Hexzeichen haben):

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

Die Standardwerte sind als Konstanten und statische Member verfügbar: `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` und `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Konsolen- und Desktop-Clients ausführen

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Lizenz

Dieses Projekt ist unter der **Elastic License 2.0** lizenziert.

* Die Software darf Dritten nicht als gehosteter Dienst zur Verfügung gestellt werden.
* Das Lizenzierungssystem darf weder verändert noch umgangen werden.
* Copyright- und Lizenzhinweise dürfen nicht entfernt werden.

Den vollständigen Text finden Sie in der [LICENSE](../LICENSE)-Datei.

# NightAuth C++ SDK — Lascia una stella su GitHub ⭐

SDK Header-Only ufficiale di **NightAuth** per C++17 e suite di client di riferimento (GUI desktop moderna nativa **Win32 Pure GDI+** e CLI da terminale interattiva). Proteggi il tuo software nativo in C++ contro reverse engineering e pirateria in pochi minuti.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Video Tutorial & Guide di Configurazione
> I video tutorial ufficiali e le guide dettagliate passo-passo per configurare applicazioni, piani, abbonamenti, variabili cloud, webhook e licenze nel dashboard di NightAuth **saranno pubblicati a breve** sul nostro portale e canale ufficiale.

---

## 📸 Schermate

### Applicazione Desktop (Nativa Win32 GDI+ Modern Dark Theme)

| Schermata di Accesso / Autenticazione | Dashboard & Gestione Abbonamento |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Applicazione Console (CLI Interattiva)

| Accesso da Terminale | Dashboard Console Interattiva |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Pratiche di Sicurezza Consigliate

* **Virtualizzazione Binaria Nativa:** I binari C++ nativi sono soggetti ad analisi statica tramite IDA Pro, Ghidra e Binary Ninja. Per le versioni di produzione, proteggi gli eseguibili compilati utilizzando virtualizzatori e protettori commerciali come **VMProtect**, **Themida** o **CodeVirtualizer**.
* **Crittografia delle Stringhe a Tempo di Compilazione:** Non lasciare stringhe sensibili o token API in testo normale nella sezione `.rdata`. Usa modelli di offuscamento XOR a tempo di compilazione (es. `skCrypter` o codificatori constexpr personalizzati) per decrittografare le stringhe solo quando necessario.
* **Esecuzione Sicura dei File in Memoria:** Durante lo streaming di payload o DLL protetti tramite `NightAuthApp.downloadFile(fileId, buffer)`, **non scrivere mai i byte sul disco fisico**. Carica ed esegui i payload direttamente dalla memoria RAM (es. reflective PE loading o manual map injection) e ripulisci immediatamente il buffer con `SecureZeroMemory()` o `std::fill`.
* **Verifica di Hardware e Integrità:** Mantieni attivi i controlli anti-debugging, di integrità della memoria e di tempistica nelle build di produzione per mitigare hooking a runtime e debugger.

---

## 📦 Requisiti e Compilazione

Richiede **C++17** o successivo e **Visual Studio 2022 / 2026**. I file `.vcxproj` inclusi usano per impostazione predefinita il toolset MSVC `v145`; `v143` è anch'esso supportato.

Dipendenze Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Crittografia e socket nativi di Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Rendering GUI desktop)
* `libcurl` e `OpenSSL` inclusi (o socket HTTPS nativi della piattaforma), importati da `deps/curl_win64` come `libcurl.dll.a;libcrypto.a;libssl.a`, con il percorso di inclusione che punta a `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> Distribuendo una build collegata al `libcurl` incluso, fornisci `libcurl-x64.dll` accanto al tuo eseguibile.

### Compila la soluzione con MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Configurazione & Inizializzazione

Configura le credenziali della tua applicazione dalla dashboard di NightAuth:

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

### Handshake Crittografico (`init`)

Chiama `init()` prima di eseguire qualsiasi altra funzione. Stabilisce lo scambio di chiavi Diffie-Hellman (X25519) e la sessione cifrata. È necessario configurare un transport prima di `init()` (vedere [Transport personalizzati](#25-custom-transports--server-key-pinning)); la soluzione più semplice usa gli helper libcurl inclusi:

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

## 🚀 Guida all'Uso dell'SDK

### 1. Controllo Sessione & Blacklist

```cpp
NightAuthApp.init();

if (!NightAuthApp.response.success) {
    std::cerr << "Initialization failed: " << NightAuthApp.response.message << "\n";
    return 1;
}
```

### 2. Login con Nome Utente & Password (e 2FA)

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

### 3. Registrazione con Chiave di Licenza

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Autenticazione Solo Licenza (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Riscatta Licenza / Aggiorna Abbonamento (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. Reset Hardware ID (HWID Lock)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. Recupero Password (Forgot Password & Reset)

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

### 8. Dati del Profilo Utente (`result.ToUserData()`)

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

### 9. Variabili Cloud (Globali)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. Variabili Utente (User Variables)

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

### 11. Registri di Audit e Telemetria

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Banna Utente e Blocca Macchina

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Webhook Lato Server

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

Per allegare un file, usa l'overload che accetta `NightAuth::WebhookOptions` (il file viene caricato come multipart/form-data):

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Download Sicuro in Memoria

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
> `SecureZeroMemory()` è un'API Win32. Per una cancellazione portabile, usa `std::fill(buffer.begin(), buffer.end(), '\0')` oppure `OPENSSL_cleanse(buffer.data(), buffer.size())` (OpenSSL è già collegato).

### 15. Canali Chat in Tempo Reale

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Gestori di Eventi in Tempo Reale (SSE)

Iscriviti allo stream di eventi del server con un callback che riceve una `NightAuth::EventMessage` (campi: `id`, `type`, `data`). Il callback deve restituire `bool`: restituisci `false` per smettere di ascoltare. Prima va configurato un transport di streaming (vedere `setStreamTransport` in [Transport personalizzati](#25-custom-transports--server-key-pinning)):

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

Puoi anche reagire alle notifiche del ciclo di vita assegnando i callback esposti sull'istanza `Api`:

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. Autenticazione a Due Fattori (2FA)

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

### 18. Ciclo di Vita della Sessione e Logout

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Cambia Nome Utente

Cambia il nome utente dell'account autenticato (la sessione viene chiusa per sicurezza):

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Statistiche dell'Applicazione e Utenti Online

Ottiene le metriche aggregate dell'applicazione e l'elenco degli utenti con sessione attiva:

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

I contatori a livello di applicazione sono disponibili direttamente sull'istanza come `NightAuthApp.app` (un `NightAuth::AppData` con `total_users`, `online_users`, `total_keys`, `version`). Gli alias `user_data` / `app_data` fanno riferimento agli stessi membri `user` / `app`.

### 21. Ticket di Accesso

Richiede un ticket a breve durata che autorizza un'operazione con scope (usato internamente da `downloadFile` e `subscribeEvents`):

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Verifica Hash del File

Verifica un digest SHA-256 esadecimale rispetto ai valori registrati sul server:

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Gestione dell'Hardware ID

Il SDK deriva automaticamente un hardware ID (hostname + username, con hash). Puoi sovrascriverlo o cancellarlo per istanza:

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

Helper statici su `NightAuth::SecurityProtections`:

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. Aggiornamenti Firmati

L'auto-aggiornamento non firmato è disabilitato: `applyUpdate()` fallisce sempre. Distribuisci manifest di release firmati e verificali con `SignedUpdate` / `SignedUpdateInstaller`:

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

`NightAuth::VerifiedUpdate` espone `application`, `version`, `sequence`, `url`, `size`, `expiresAt` e `digest` (byte SHA-256).

### 25. Transport Personalizzati e Fissaggio delle Chiavi del Server

Tutto il traffico HTTP passa attraverso transport intercambiabili. Gli helper libcurl inclusi sono la scelta predefinita; l'HTTP in chiaro è rifiutato tranne per host loopback esatti con opt-in esplicito:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

Per self-hosting o rotazione delle chiavi, fissa le tue chiavi di firma / sealed-box del server prima di `init()` (entrambe devono essere di 64 caratteri esadecimali):

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

I valori predefiniti sono esposti come costanti e membri statici: `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` e `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Esecuzione dei client Console e Desktop

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licenza

Questo progetto è rilasciato sotto licenza **Elastic License 2.0**.

* Non è consentito fornire il software a terzi come servizio gestito o in hosting.
* Non è consentito alterare, disattivare o aggirare le funzioni di licenza.
* Non è consentito rimuovere o modificare gli avvisi di copyright.

Consulta il file [LICENSE](../LICENSE) per il testo completo.

# NightAuth C++ SDK — Star us on GitHub ⭐

Official **NightAuth** C++17 Header-Only SDK & Reference Client Suite (Modern Desktop UI in **Win32 Pure GDI+** and Interactive Terminal CLI). Protect your native C++ software against reverse engineering, piracy, and tampering with cloud licensing, HWID locking, realtime encrypted chat, and secure in-memory streaming in minutes.

Official Portal: [https://nightauth.org](https://nightauth.org)  
Documentation: [https://nightauth.org/docs](https://nightauth.org/docs)  
Technical Architecture Benchmark: [NightAuth vs KeyAuth vs Auth.gg](https://nightauth.org/blog/nightauth-vs-keyauth-authgg-comparison)

---

**Languages / Idiomas / Langues / Sprachen:**  
[English](README.md) • [Español](lang/es.md) • [Français](lang/fr.md) • [Deutsch](lang/de.md) • [Italiano](lang/it.md) • [Português](lang/pt.md) • [Русский](lang/ru.md) • [Türkçe](lang/tr.md) • [Polski](lang/pl.md) • [中文](lang/zh.md)

---

> [!NOTE]
> ### 📺 Video Tutorials & System Setup Guides
> Official video tutorials and detailed step-by-step guides explaining how to configure applications, plans, subscriptions, cloud variables, webhooks, and licenses in the NightAuth Dashboard **will be published soon** on our official portal and YouTube channel.

---

## 📸 Screenshots

### Desktop Application (Native Win32 GDI+ Modern Dark Theme)

| Start / Authentication Screen | Dashboard & Subscription Management |
| :---: | :---: |
| ![NightAuth Desktop Login](image/desktop.png) | ![NightAuth Desktop Dashboard](image/desktop-dashboard.png) |

### Console Application (Interactive Terminal CLI)

| Terminal Login Screen | Interactive Console Dashboard |
| :---: | :---: |
| ![NightAuth Console Login](image/console.png) | ![NightAuth Console Dashboard](image/console-dashboard.png) |

---

## 🛡️ Recommended Security Practices

* **Native Binary Virtualization:** Native C++ binaries are subject to static analysis via IDA Pro, Ghidra, and Binary Ninja. For production releases, protect compiled executables using commercial code virtualizers and protectors such as **VMProtect**, **Themida**, or **CodeVirtualizer**.
* **Compile-Time String Encryption:** Do not leave sensitive strings or API tokens in plain text in the `.rdata` section. Use compile-time XOR obfuscation templates (e.g. `skCrypter` or custom constexpr string encoders) to decrypt strings only at the instant they are required.
* **In-Memory Secure File Execution:** When streaming protected payloads or DLLs via `NightAuthApp.downloadFile(fileId, buffer)`, **never write bytes to physical storage or disk**. Load and execute payloads directly from memory (e.g., reflective PE loading or manual map injection) and immediately wipe the buffer with `SecureZeroMemory()` or `std::fill`.
* **Hardware & Integrity Verification:** Keep anti-debugging, memory integrity, and timing checks active in production releases to mitigate runtime hooking and debugger attachment.

---

## 📦 Requirements & Building

Requires **C++17** or later and **Visual Studio 2022 / 2026**. The bundled `.vcxproj` files default to the MSVC toolset `v145`; `v143` is also supported.

Windows dependencies:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Native Windows Crypto & Sockets)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Desktop GUI Rendering)
* Bundled `libcurl` and `OpenSSL` (or platform native HTTPS sockets), imported from `deps/curl_win64` as `libcurl.dll.a;libcrypto.a;libssl.a`, with the include path pointing to `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> When distributing a build that links the bundled `libcurl`, ship `libcurl-x64.dll` alongside your executable.

### Build Solution with MSBuild

```bash
# Build both Console & Desktop clients in 64-bit Release
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Configuration & Initialization

Include the header in one translation unit with `#define NIGHTAUTH_IMPLEMENTATION`:

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

### Initialize Cryptographic Handshake (`init`)

You must call `init()` prior to executing any other SDK functions. This establishes an ephemeral Diffie-Hellman (X25519) key exchange and an encrypted session. A transport must be configured before `init()` (see [Custom Transports & Server Key Pinning](#25-custom-transports--server-key-pinning)); the simplest setup uses the bundled libcurl helpers:

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

## 🚀 SDK Usage Guide

### 1. Check Session Validation & Blacklist Status

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

### 2. Login with Username & Password (and 2FA)

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

### 3. Register with License Key

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Authenticate with License Key Only

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Upgrade Account / Claim License Key

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

### 7. Forgot Password / Account Recovery

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

### 8. User Profile Data (`NightAuthApp.user`)

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

### 9. Cloud Variables (Global)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. User Variables

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

### 11. Application Audit Logs

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Ban User & Blacklist Machine

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Server-Sided Webhooks

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

To attach a file, use the overload that accepts `NightAuth::WebhookOptions` (the file is uploaded as multipart/form-data):

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Secure In-Memory File Download

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
> `SecureZeroMemory()` is a Win32 API. For a portable wipe, use `std::fill(buffer.begin(), buffer.end(), '\0')` or `OPENSSL_cleanse(buffer.data(), buffer.size())` (OpenSSL is already linked).

### 15. Realtime Chat Channels

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Realtime Event Handlers (Server-Sent Events)

Subscribe to the server-sent event stream with a callback receiving a `NightAuth::EventMessage` (fields: `id`, `type`, `data`). The callback must return `bool`: return `false` to stop listening. A stream transport must be configured first (see `setStreamTransport` in [Custom Transports](#25-custom-transports--server-key-pinning)):

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

You can also react to lifecycle notifications by assigning the event callbacks exposed on the `Api` instance:

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. Two-Factor Authentication (2FA)

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

### 18. Session Lifecycle & Logout

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Change Username

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Application Statistics & Online Users

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

App-level counters are also available directly on the instance as `NightAuthApp.app` (a `NightAuth::AppData` with `total_users`, `online_users`, `total_keys`, `version`). The aliases `user_data` / `app_data` reference the same `user` / `app` members.

### 21. Access Tickets

Request a short-lived ticket that authorizes a scoped operation (used internally by `downloadFile` and `subscribeEvents`):

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Verify File Hash

Verify a SHA-256 hex digest against the values registered server-side:

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Hardware ID Management

The SDK derives a hardware ID automatically (hostname + username, hashed). You can override or clear it per instance:

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

Static helpers on `NightAuth::SecurityProtections`:

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. Signed Self-Updates

Unsigned auto-update is disabled: `applyUpdate()` always fails. Ship signed release manifests and verify them with `SignedUpdate` / `SignedUpdateInstaller` instead:

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

`NightAuth::VerifiedUpdate` exposes `application`, `version`, `sequence`, `url`, `size`, `expiresAt` and `digest` (SHA-256 bytes).

### 25. Custom Transports & Server Key Pinning

All HTTP traffic goes through pluggable transports. The bundled libcurl helpers are the default choice; plain HTTP is refused except for exact loopback hosts with explicit opt-in:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

To self-host or rotate keys, pin your own server signing / sealed-box keys before `init()` (both must be 64 hex characters):

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

The defaults are exposed as constants and static members: `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` and `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Run the Console and Desktop Clients

Launch either reference client from the `source` directory or build with MSBuild:

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

---

## 🆚 Why NightAuth? (KeyAuth & Auth.gg Comparison)

| Feature | NightAuth | KeyAuth | Auth.gg |
| :--- | :---: | :---: | :---: |
| **Cryptography** | **End-to-End Curve25519 (X25519) + ChaCha20-Poly1305** | Static AES-CBC (Vulnerable to MITM & Proxy Interception) | Legacy HTTPS / Plain JSON |
| **Hardware ID (HWID)** | **Multi-Component Cryptographic Hash (Anti-Spoofing)** | Raw WMI Strings (Trivially Spoofed) | Simple Registry / MAC |
| **Edge Network Latency** | **< 35ms Global Anycast Edge Network** | Single-Origin VPS (Frequent Network Outages) | Unmaintained / High Latency |
| **Reseller System & Bots** | **Built-in Multi-Tier Credit Balances + Discord & Telegram Bots** | Manual API / Third-Party Scripts | Incomplete / Deprecated |
| **In-Memory Streaming** | **Zero-Disk Decryption with Direct Memory Execution & Buffer Wiping** | Disk Writes Required | Not Supported |
| **Internationalization** | **10 Languages Supported Out of the Box across Documentation & SDKs** | English Only | English Only |

Read the full architectural benchmark: [NightAuth vs KeyAuth vs Auth.gg](https://nightauth.org/blog/nightauth-vs-keyauth-authgg-comparison)

---

## 📄 License

This project is licensed under the **Elastic License 2.0**.

* You may not provide the software to third parties as a hosted or managed service.
* You may not move, change, disable, or circumvent the license key functionality.
* You may not alter, remove, or obscure any licensing or copyright notices.

See the [LICENSE](LICENSE) file for the full legal text.

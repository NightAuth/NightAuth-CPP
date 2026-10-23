# NightAuth C++ SDK — 在 GitHub 上关注并点星 ⭐

官方 **NightAuth** C++17 Header-Only SDK 与参考客户端套件（现代原生 **Win32 Pure GDI+** 桌面界面与交互式终端 CLI）。在几分钟内保护您的原生 C++ 软件免受逆向工程、盗版和篡改。

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 视频教程与系统部署指南
> 官方视频操作教程以及关于如何在 NightAuth 管理后台配置应用、订阅计划、云端变量、Webhook 与密钥分发的分步指南**即将发布**，敬请关注官方网站与频道。

---

## 📸 界面截图

### 桌面应用程序 (原生 Win32 GDI+ 现代深色主题)

| 登录 / 激活认证界面 | 用户面板与授权监控 |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### 控制台客户端 (交互式 Terminal CLI)

| 终端登录交互 | 控制台交互式仪表盘 |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ 推荐安全实践

* **原生二进制虚拟化保护：** 原生 C++ 二进制文件易受到 IDA Pro、Ghidra 和 Binary Ninja 等工具的静态逆向分析。在生产发布版本中，务必使用商业代码虚拟化保护壳（如 **VMProtect**、**Themida** 或 **CodeVirtualizer**）对编译后的可执行文件进行加壳保护。
* **编译期字符串加密：** 严禁将敏感字符串或 API 密钥以明文形式遗留在 `.rdata` 节区中。使用编译期 XOR 混淆模板（如 `skCrypter` 或自定义 constexpr 字符串编码器），仅在调用瞬间在栈上解密。
* **内存中安全文件执行：** 通过 `NightAuthApp.downloadFile(fileId, buffer)` 串流下载受保护的载荷或 DLL 时，**切勿将字节流写入物理硬盘**。必须直接在内存 RAM 中加载并执行（如反射型 PE 加载或手动映射注入），执行完成后立即调用 `SecureZeroMemory()` 或 `std::fill` 彻底擦除缓冲区。
* **硬件指纹与运行期完整性检测：** 在生产版本中保持反调试、内存完整性校验和时序检测常开，有效阻断运行时 Hook 及调试器挂载。

---

## 📦 系统要求与编译

需要 **C++17** 或更高版本，以及 **Visual Studio 2022 / 2026**。附带的 `.vcxproj` 文件默认使用 MSVC 工具集 `v145`；也支持 `v143`。

Windows 依赖项：
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib`（Windows 原生加密与套接字）
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib`（桌面 GUI 渲染）
* 内置的 `libcurl` 和 `OpenSSL`（或平台原生 HTTPS 套接字），从 `deps/curl_win64` 以 `libcurl.dll.a;libcrypto.a;libssl.a` 形式导入，include 路径指向 `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> 分发链接了内置 `libcurl` 的构建时，请将 `libcurl-x64.dll` 与可执行文件放在同一目录。

### 使用 MSBuild 编译解决方案

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ 应用程序配置与客户端实例化

在 NightAuth 管理后台的 **Manage Apps** 选项卡中获取应用凭证：

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

### 初始化加密握手 (`init`)

在调用其他任何 SDK 方法之前，必须先调用 `init()`。该方法将完成 Diffie-Hellman (X25519) 密钥交换并建立端到端加密会话。在 `init()` 之前必须先配置传输层（参见[自定义传输层](#25-custom-transports--server-key-pinning)）；最简单的方式是使用内置的 libcurl 辅助函数：

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

## 🚀 SDK 核心功能使用指南

### 1. 验证会话与黑名单检查

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

### 2. 账号密码登录 (支持 2FA 双重认证)

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

### 3. 使用授权密钥注册新用户

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. 仅凭卡密激活登录 (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. 升级账号 / 激活充值新卡密 (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. 重置机器码绑定 (HWID Reset)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. 找回密码与重置 (Forgot Password & Reset)

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

### 8. 用户档案与授权信息 (`result.ToUserData()`)

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

### 9. 云端全局变量 (Cloud Variables)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. 用户专属隔离变量 (User Variables)

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

### 11. 远程安全审计日志

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. 封禁用户并拉黑当前设备

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. 服务端安全 Webhook 转发

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

如需附加文件，请使用接受 `NightAuth::WebhookOptions` 的重载（文件将以 multipart/form-data 上传）：

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. 内存中安全下载文件

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
> `SecureZeroMemory()` 是 Win32 API。如需跨平台擦除，请使用 `std::fill(buffer.begin(), buffer.end(), '\0')` 或 `OPENSSL_cleanse(buffer.data(), buffer.size())`（OpenSSL 已链接）。

### 15. 实时聊天频道

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. 实时服务端推送事件处理 (SSE)

使用接收 `NightAuth::EventMessage`（字段：`id`、`type`、`data`）的回调订阅服务器推送事件流。回调必须返回 `bool`：返回 `false` 可停止监听。须先配置流式传输层（参见[自定义传输层](#25-custom-transports--server-key-pinning)中的 `setStreamTransport`）：

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

你还可以通过为 `Api` 实例上的回调赋值来响应生命周期通知：

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. 双重认证管理 (2FA)

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

### 18. 会话保活与安全登出

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. 修改用户名

修改已认证账户的用户名（出于安全考虑，会话将被关闭）：

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. 应用统计与在线用户

获取应用的聚合统计信息和当前在线用户列表：

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

应用级计数器也可直接通过实例上的 `NightAuthApp.app`（一个包含 `total_users`、`online_users`、`total_keys`、`version` 的 `NightAuth::AppData`）获取。别名 `user_data` / `app_data` 引用的是相同的 `user` / `app` 成员。

### 21. 访问票据

请求一个短期票据，用于授权特定范围的操作（`downloadFile` 和 `subscribeEvents` 内部使用）：

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. 校验文件哈希

将十六进制 SHA-256 摘要与服务器端注册的值进行校验：

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. 硬件 ID (HWID) 管理

SDK 会自动派生硬件 ID（hostname + username 的哈希值）。你可以按实例覆盖或清除它：

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

`NightAuth::SecurityProtections` 上的静态辅助方法：

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. 签名自动更新

未签名的自动更新已被禁用：`applyUpdate()` 始终失败。请分发已签名的发布清单，并使用 `SignedUpdate` / `SignedUpdateInstaller` 进行验证：

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

`NightAuth::VerifiedUpdate` 暴露 `application`、`version`、`sequence`、`url`、`size`、`expiresAt` 和 `digest`（SHA-256 字节）。

### 25. 自定义传输层与服务器密钥固定

所有 HTTP 流量都通过可插拔的传输层。内置的 libcurl 辅助函数是默认选择；除显式选择加入的精确 loopback 主机外，纯 HTTP 会被拒绝：

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

如需自托管或轮换密钥，请在 `init()` 之前固定你自己的服务器签名 / sealed-box 密钥（两者均须为 64 个十六进制字符）：

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

默认值以常量和静态成员形式暴露：`NightAuth::DEFAULT_API_URL`、`NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`、`NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`、`NightAuth::Api::ServerSigningPublicKey` 和 `NightAuth::Api::ServerSealedBoxPublicKey`。

---

## 🖥️ 运行控制台与桌面客户端

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 授权协议

本项目基于 **Elastic License 2.0** 协议授权。

* 不得以托管或在线托管服务的形式向第三方提供本软件服务。
* 不得移动、修改、禁用或规避许可密钥验证相关功能。
* 不得修改、移除或遮挡任何许可与版权声明。

详情请参阅 [LICENSE](../LICENSE) 文件。

# NightAuth C++ SDK — Adicione uma estrela no GitHub ⭐

SDK Header-Only oficial do **NightAuth** para C++17 e suíte de clientes de referência (Interface gráfica moderna nativa **Win32 Pure GDI+** e terminal interativo CLI). Proteja seu software nativo em C++ contra engenharia reversa e pirataria em minutos.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Tutoriais em Vídeo & Guias de Configuração
> Tutoriais oficiais em vídeo e guias passo a passo explicando como configurar aplicativos, planos, assinaturas, variáveis em nuvem, webhooks e licenças no painel do NightAuth **serão publicados em breve**.

---

## 📸 Capturas de Tela

### Aplicação Desktop (Nativa Win32 GDI+ Tema Escuro Moderno)

| Tela de Início / Autenticação | Painel de Controle & Assinaturas |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Aplicativo de Console (CLI Interativa)

| Login pelo Terminal | Painel Interativo de Console |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Práticas de Segurança Recomendadas

* **Virtualização de Binários Nativos:** Binários nativos em C++ estão sujeitos à análise estática via IDA Pro, Ghidra e Binary Ninja. Para versões de produção, proteja os executáveis compilados utilizando virtualizadores e protetores comerciais como **VMProtect**, **Themida** ou **CodeVirtualizer**.
* **Criptografia de Strings em Tempo de Compilação:** Não deixe strings sensíveis ou tokens de API em texto puro na seção `.rdata`. Utilize templates de ofuscação XOR em tempo de compilação (como `skCrypter` ou codificadores constexpr personalizados) para descriptografar strings apenas no instante em que forem necessárias.
* **Execução Segura de Arquivos em Memória:** Ao transmitir payloads protegidos ou DLLs via `NightAuthApp.downloadFile(fileId, buffer)`, **nunca grave bytes no armazenamento físico ou disco**. Carregue e execute os executáveis diretamente da memória RAM (por exemplo, carga refletiva de PE ou injeção manual map) e limpe imediatamente o buffer com `SecureZeroMemory()` ou `std::fill`.
* **Verificação de Hardware e Integridade:** Mantenha ativas as verificações anti-depuração, integridade de memória e temporização em versões de produção.

---

## 📦 Requisitos e Compilação

Requer **C++17** ou posterior e **Visual Studio 2022 / 2026**. Os ficheiros `.vcxproj` incluídos utilizam por predefinição o conjunto de ferramentas MSVC `v145`; `v143` também é suportado.

Dependências do Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Criptografia e soquetes nativos do Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Renderização de GUI nativa)
* `libcurl` e `OpenSSL` incluídos (ou sockets HTTPS nativos da plataforma), importados de `deps/curl_win64` como `libcurl.dll.a;libcrypto.a;libssl.a`, com o caminho de inclusão a apontar para `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> Ao distribuir uma compilação ligada ao `libcurl` incluído, coloque `libcurl-x64.dll` junto do seu executável.

### Compilar a Solução com MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Configuração & Inicialização

Configure as credenciais do seu aplicativo obtidas no painel do NightAuth:

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

### Inicializar Handshake Criptográfico (`init`)

Chame `init()` antes de qualquer outra chamada para estabelecer a troca de chaves Diffie-Hellman (X25519) e a sessão criptografada. Um transporte deve ser configurado antes de `init()` (ver [Transportes personalizados](#25-custom-transports--server-key-pinning)); a opção mais simples utiliza os auxiliares libcurl incluídos:

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

## 🚀 Guia de Uso do SDK

### 1. Verificação de Sessão & Blacklist

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

### 2. Login com Usuário & Senha (e 2FA)

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

### 3. Registro com Chave de Licença

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Acesso Apenas com Licença (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Atualizar Assinatura / Resgatar Chave (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. Redefinir Hardware ID (HWID Lock)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. Recuperação de Senha (Forgot Password & Reset)

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

### 8. Dados do Perfil do Usuário (`result.ToUserData()`)

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

### 9. Variáveis na Nuvem (Globais)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. Variáveis do Usuário (User Variables)

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

### 11. Registros de Auditoria

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Banir Usuário & Bloquear Máquina

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Webhooks no Servidor

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

Para anexar um ficheiro, use a sobrecarga que aceita `NightAuth::WebhookOptions` (o ficheiro é enviado como multipart/form-data):

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Download Seguro na Memória

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
> `SecureZeroMemory()` é uma API Win32. Para uma limpeza portátil, use `std::fill(buffer.begin(), buffer.end(), '\0')` ou `OPENSSL_cleanse(buffer.data(), buffer.size())` (OpenSSL já está ligado).

### 15. Canais de Chat em Tempo Real

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Eventos em Tempo Real (SSE)

Subscreva o fluxo de eventos do servidor com um callback que recebe uma `NightAuth::EventMessage` (campos: `id`, `type`, `data`). O callback deve devolver `bool`: devolva `false` para parar de escutar. Primeiro deve configurar-se um transporte de stream (ver `setStreamTransport` em [Transportes personalizados](#25-custom-transports--server-key-pinning)):

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

Também pode reagir às notificações do ciclo de vida atribuindo os callbacks expostos na instância `Api`:

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. Autenticação de Dois Fatores (2FA)

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

### 18. Ciclo de Vida da Sessão e Logout

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Alterar Nome de Utilizador

Altera o nome de utilizador da conta autenticada (a sessão é encerrada por segurança):

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Estatísticas da Aplicação e Utilizadores Online

Obtém métricas agregadas da aplicação e a lista de utilizadores com sessão ativa:

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

Os contadores ao nível da aplicação também estão disponíveis diretamente na instância como `NightAuthApp.app` (um `NightAuth::AppData` com `total_users`, `online_users`, `total_keys`, `version`). Os alias `user_data` / `app_data` referenciam os mesmos membros `user` / `app`.

### 21. Tickets de Acesso

Solicita um ticket de curta duração que autoriza uma operação com âmbito (usado internamente por `downloadFile` e `subscribeEvents`):

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Verificar Hash do Ficheiro

Verifica um digest SHA-256 hexadecimal contra os valores registados no servidor:

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Gestão do Hardware ID

O SDK deriva automaticamente um hardware ID (hostname + username, com hash). Pode sobrescrevê-lo ou limpá-lo por instância:

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

Auxiliares estáticos em `NightAuth::SecurityProtections`:

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. Atualizações Assinadas

A auto-atualização sem assinatura está desativada: `applyUpdate()` falha sempre. Distribua manifestos de release assinados e verifique-os com `SignedUpdate` / `SignedUpdateInstaller`:

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

`NightAuth::VerifiedUpdate` expõe `application`, `version`, `sequence`, `url`, `size`, `expiresAt` e `digest` (bytes SHA-256).

### 25. Transportes Personalizados & Fixação de Chaves do Servidor

Todo o tráfego HTTP passa por transportes intermutáveis. Os auxiliares libcurl incluídos são a opção predefinida; HTTP simples é recusado exceto para hosts loopback exatos com opção explícita:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

Para alojar por conta própria ou rodar chaves, fixe as suas próprias chaves de assinatura / sealed-box do servidor antes de `init()` (ambas devem ter 64 caracteres hexadecimais):

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

Os valores predefinidos são expostos como constantes e membros estáticos: `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` e `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Executar os Clientes de Console e Desktop

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licença

Este projeto é licenciado sob a **Elastic License 2.0**.

* Não é permitido fornecer o software como serviço hospedado ou gerenciado.
* Não é permitido alterar, desativar ou burlar o sistema de licenças.
* Não é permitido remover avisos de direitos autorais.

Consulte o arquivo [LICENSE](../LICENSE) para o texto legal completo.

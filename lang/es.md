# NightAuth C++ SDK — Danos una estrella en GitHub ⭐

SDK Header-Only oficial de **NightAuth** para C++17 y suite de clientes de referencia (interfaz gráfica moderna nativa de **Win32 GDI+** y CLI interactivo para terminal). Protege tu software en C++ contra ingeniería inversa, piratería y manipulaciones en minutos.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Tutoriales en Video y Configuración del Sistema
> Los video-tutoriales oficiales y las guías paso a paso de cómo configurar aplicaciones, planes, suscripciones, variables en la nube, webhooks y licencias en el panel de control de NightAuth **serán publicados próximamente** en nuestro portal y canal oficial.

---

## 📸 Capturas de Pantalla

### Aplicación de Escritorio (Nativa Win32 GDI+ Tema Oscuro Moderno)

| Pantalla de Inicio / Autenticación | Panel de Control & Gestión de Suscripciones |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Aplicación de Consola (CLI Interactiva)

| Inicio de Sesión en Terminal | Panel Interactivo de Consola |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Prácticas de Seguridad Recomendadas

* **Virtualización de Binarios Nativos:** Los binarios nativos en C++ están sujetos a análisis estático mediante IDA Pro, Ghidra y Binary Ninja. Para versiones de producción, protege los ejecutables compilados utilizando virtualizadores y protectores comerciales como **VMProtect**, **Themida** o **CodeVirtualizer**.
* **Cifrado de Cadenas en Tiempo de Compilación:** No dejes cadenas sensibles o tokens de API en texto plano en la sección `.rdata`. Utiliza plantillas de ofuscación XOR en tiempo de compilación (como `skCrypter` o codificadores de cadenas constexpr personalizados) para descifrar cadenas solo en el instante en que se necesitan.
* **Ejecución Segura de Archivos en Memoria:** Al transmitir payloads protegidos o DLLs mediante `NightAuthApp.downloadFile(fileId, buffer)`, **nunca escribas los bytes en el almacenamiento físico o disco**. Carga y ejecuta las cargas útiles directamente desde la memoria (p. ej., carga reflectiva de PE o inyección manual map) y limpia inmediatamente el búfer con `SecureZeroMemory()` o `std::fill`.
* **Verificación de Hardware e Integridad:** Mantén activas las comprobaciones anti-debugging, integridad de memoria y temporización en versiones de producción para mitigar el enganche en tiempo de ejecución y la conexión de depuradores.

---

## 📦 Requisitos y Compilación

Requiere **C++17** o posterior y **Visual Studio 2022 / 2026**. Los `.vcxproj` incluidos usan por defecto el conjunto de herramientas MSVC `v145`; `v143` también es compatible.

Dependencias en Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Criptografía y sockets nativos de Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Renderizado de GUI nativa)
* `libcurl` y `OpenSSL` incluidos (o sockets HTTPS nativos de la plataforma), importados desde `deps/curl_win64` como `libcurl.dll.a;libcrypto.a;libssl.a`, con la ruta de inclusión apuntando a `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> Al distribuir una compilación que enlaza el `libcurl` incluido, incluye `libcurl-x64.dll` junto a tu ejecutable.

### Compilar la Solución con MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Configuración e Inicialización

Configura las credenciales de tu aplicación obtenidas en el panel de NightAuth:

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

Debes invocar `init()` antes de cualquier otra llamada. Establece el intercambio de claves efímeras Diffie-Hellman (X25519) y la sesión cifrada. Debe configurarse un transporte antes de `init()` (véase [Transportes personalizados](#25-custom-transports--server-key-pinning)); la opción más sencilla usa los helpers de libcurl incluidos:

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

## 🚀 Guía de Uso del SDK

### 1. Comprobar Validación de Sesión y Blacklist

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

### 2. Inicio de Sesión (Login con Usuario, Contraseña y 2FA)

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

### 3. Registro con Clave de Licencia

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Acceso con Solo Clave de Licencia (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Actualizar Suscripción / Canjear Clave (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. Restablecer HWID (Hardware ID Lock)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. Recuperación de Contraseña (Forgot Password & Reset)

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

### 8. Datos del Usuario Autenticado (`result.ToUserData()`)

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

### 9. Variables en la Nube (Globales)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. Variables por Usuario (User Variables)

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

### 11. Logs de Aplicación y Auditoría

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Baneo de Usuarios y Lista Negra

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Webhooks Seguros del Servidor

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

Para adjuntar un archivo, usa la sobrecarga que acepta `NightAuth::WebhookOptions` (el archivo se sube como multipart/form-data):

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Descarga Segura de Archivos en Memoria

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
> `SecureZeroMemory()` es una API de Win32. Para un borrado portable, usa `std::fill(buffer.begin(), buffer.end(), '\0')` u `OPENSSL_cleanse(buffer.data(), buffer.size())` (OpenSSL ya está enlazado).

### 15. Canales de Chat en Vivo

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Controladores de Eventos en Tiempo Real (SSE)

Suscríbete al flujo de eventos del servidor con un callback que recibe un `NightAuth::EventMessage` (campos: `id`, `type`, `data`). El callback debe devolver `bool`: devuelve `false` para dejar de escuchar. Primero debe configurarse un transporte de streaming (véase `setStreamTransport` en [Transportes personalizados](#25-custom-transports--server-key-pinning)):

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

También puedes reaccionar a las notificaciones del ciclo de vida asignando los callbacks expuestos en la instancia `Api`:

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. Autenticación de Dos Factores (2FA)

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

### 18. Ciclo de Vida de Sesión y Logout

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Cambiar Nombre de Usuario

Cambia el nombre de usuario de la cuenta autenticada (la sesión se cierra por seguridad):

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Estadísticas de la Aplicación y Usuarios en Línea

Obtiene métricas agregadas de la aplicación y la lista de usuarios con sesión activa:

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

Los contadores a nivel de aplicación también están disponibles directamente en la instancia como `NightAuthApp.app` (un `NightAuth::AppData` con `total_users`, `online_users`, `total_keys`, `version`). Los alias `user_data` / `app_data` referencian los mismos miembros `user` / `app`.

### 21. Tickets de Acceso

Solicita un ticket de corta duración que autoriza una operación con ámbito (usado internamente por `downloadFile` y `subscribeEvents`):

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Verificar Hash de Archivo

Verifica un digest SHA-256 en hexadecimal contra los valores registrados en el servidor:

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Gestión del Hardware ID

El SDK deriva un hardware ID automáticamente (hostname + username, con hash). Puedes sobrescribirlo o borrarlo por instancia:

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

Helpers estáticos en `NightAuth::SecurityProtections`:

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. Actualizaciones Autofirmadas

La autoactualización sin firma está deshabilitada: `applyUpdate()` siempre falla. Distribuye manifiestos de release firmados y verifícalos con `SignedUpdate` / `SignedUpdateInstaller`:

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

`NightAuth::VerifiedUpdate` expone `application`, `version`, `sequence`, `url`, `size`, `expiresAt` y `digest` (bytes SHA-256).

### 25. Transports Personalizados y Fijación de Claves del Servidor

Todo el tráfico HTTP pasa por transports intercambiables. Los helpers de libcurl incluidos son la opción por defecto; HTTP plano se rechaza salvo hosts loopback exactos con opción explícita:

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

Para autoalojarte o rotar claves, fija tus propias claves de firma / sealed-box del servidor antes de `init()` (ambas deben tener 64 caracteres hexadecimales):

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

Los valores predeterminados se exponen como constantes y miembros estáticos: `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` y `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Ejecutar los Clientes de Consola y Escritorio

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licencia

Este proyecto está bajo la licencia **Elastic License 2.0**.

* No está permitido ofrecer el software a terceros como servicio gestionado o alojado.
* No está permitido modificar, deshabilitar o eludir el sistema de licencias.
* No está permitido alterar ni eliminar avisos de copyright o de licencia.

Consulta el archivo [LICENSE](../LICENSE) para el texto legal completo.

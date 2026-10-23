# NightAuth C++ SDK — Mettez-nous une étoile sur GitHub ⭐

SDK Header-Only officiel **NightAuth** C++17 et suite de clients de référence (Interface graphique moderne **Win32 Pure GDI+** et CLI interactif pour terminal). Protégez vos logiciels natifs C++ contre l'ingénierie inverse et le piratage en quelques minutes.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Tutoriels Vidéo & Guides de Déploiement
> Les tutoriels vidéo officiels et les guides pas-à-pas détaillant la configuration des applications, plans, abonnements, variables cloud, webhooks et licences sur le tableau de bord NightAuth **seront publiés prochainement** sur notre portail et chaîne officielle.

---

## 📸 Captures d'écran

### Application de Bureau (Native Win32 GDI+ Thème Sombre Moderne)

| Écran d'Authentification / Connexion | Tableau de Bord & Abonnements |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Application Console (Terminal Interactif)

| Connexion Console | Tableau de Bord Interactif |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Pratiques de Sécurité Recommandées

* **Virtualisation de Binaires Natifs :** Les binaires C++ natifs sont vulnérables à l'analyse statique via IDA Pro, Ghidra et Binary Ninja. Pour les versions de production, protégez les exécutables compilés à l'aide de virtualiseurs et de protecteurs commerciaux tels que **VMProtect**, **Themida** ou **CodeVirtualizer**.
* **Chiffrement des Chaînes à la Compilation :** Ne laissez pas de chaînes sensibles ou de jetons d'API en clair dans la section `.rdata`. Utilisez des modèles d'offuscation XOR au moment de la compilation (par ex. `skCrypter` ou des encodeurs constexpr personnalisés) pour déchiffrer les chaînes uniquement au moment requis.
* **Exécution Sécurisée en Mémoire :** Lors du streaming de payloads ou de DLLs protégés via `NightAuthApp.downloadFile(fileId, buffer)`, **n'écrivez jamais les octets sur le disque physique**. Chargez et exécutez les charges utiles directement depuis la mémoire RAM (par ex. chargement PE réflectif ou injection de mappe manuelle) et effacez immédiatement le tampon avec `SecureZeroMemory()` ou `std::fill`.
* **Vérification du Matériel et de l'Intégrité :** Maintenez actives les vérifications anti-débogage, l'intégrité de la mémoire et les contrôles temporels dans les versions de production pour limiter l'accrochage à l'exécution.

---

## 📦 Prérequis et Compilation

Nécessite **C++17** ou supérieur et **Visual Studio 2022 / 2026**. Les fichiers `.vcxproj` fournis utilisent par défaut le toolset MSVC `v145`; `v143` est également pris en charge.

Dépendances Windows :
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Cryptographie et sockets natifs Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Rendu de l'interface graphique de bureau)
* `libcurl` et `OpenSSL` inclus (ou sockets HTTPS natifs de la plateforme), importés depuis `deps/curl_win64` en tant que `libcurl.dll.a;libcrypto.a;libssl.a`, avec le chemin d'inclusion pointant vers `deps/curl_win64/curl-8.23.0_5-win64-mingw/include`

> [!IMPORTANT]
> Lors de la distribution d'une build liée au `libcurl` inclus, placez `libcurl-x64.dll` à côté de votre exécutable.

### Compiler la solution avec MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

---

## ⚙️ Configuration & Initialisation

Définissez vos identifiants d'application depuis le tableau de bord NightAuth :

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

### Initialiser le Handshake Cryptographique (`init`)

Vous devez appeler `init()` avant toute autre fonction. Il initialise l'échange de clés Diffie-Hellman (X25519) et la session chiffrée. Un transport doit être configuré avant `init()` (voir [Transports personnalisés](#25-custom-transports--server-key-pinning)); le plus simple est d'utiliser les helpers libcurl inclus :

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

## 🚀 Guide d'Utilisation du SDK

### 1. Vérification de Session & Liste Noire

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

### 2. Connexion avec Identifiant & Mot de Passe (et 2FA)

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

### 3. Inscription avec Clé de Licence

```cpp
if (NightAuthApp.registerAccount(username, password, licenseKey, email)) {
    std::cout << "Registered successfully: " << NightAuthApp.user.username << "\n";
} else {
    std::cerr << "Registration failed: " << NightAuthApp.response.message << "\n";
}
```

### 4. Authentification par Clé Seule (Key Only)

```cpp
if (NightAuthApp.license(licenseKey)) {
    std::cout << "License verified successfully. Access granted.\n";
} else {
    std::cerr << "Access denied: " << NightAuthApp.response.message << "\n";
}
```

### 5. Mise à Niveau d'Abonnement (Upgrade)

```cpp
if (NightAuthApp.upgrade(username, newLicenseKey)) {
    std::cout << "Subscription upgraded successfully.\n";
} else {
    std::cerr << "Upgrade failed: " << NightAuthApp.response.message << "\n";
}
```

### 6. Réinitialisation de l'Identifiant Matériel (HWID Lock)

```cpp
if (NightAuthApp.resetHwid(username, password, totpCode)) {
    std::cout << "HWID reset successfully. You can now log in on this machine.\n";
} else {
    std::cerr << "HWID reset failed: " << NightAuthApp.response.message << "\n";
}
```

### 7. Mot de Passe Oublié (Forgot Password & Reset)

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

### 8. Données du Profil Utilisateur (`result.ToUserData()`)

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

### 9. Variables Cloud (Globales)

```cpp
std::string motd = NightAuthApp.getGlobalVar("server_motd");
std::cout << "Cloud Variable: " << motd << "\n";
```

### 10. Variables Utilisateur (User Variables)

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

### 11. Journaux d'Audit & Sécurité

```cpp
NightAuthApp.log("Suspicious memory modification attempt detected.");
```

### 12. Bannissement d'Utilisateur & Machine

```cpp
NightAuthApp.ban("Security tampering detected by Anti-Cheat.");
```

### 13. Webhooks Côté Serveur Sécurisés

```cpp
NightAuth::WebhookResult res;
std::string payload = "{\"content\":\"Client notification\"}";

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res)) {
    std::cout << "Webhook Response: HTTP " << res.status << "\n";
}
```

Pour joindre un fichier, utilisez la surcharge acceptant `NightAuth::WebhookOptions` (le fichier est envoyé en multipart/form-data) :

```cpp
NightAuth::WebhookOptions options;
options.hasFile = true;
options.fileName = "report.bin";
options.fileData = fileBytes; // raw file content

if (NightAuthApp.triggerWebhook("7kR0UedlVI", payload, res, options)) {
    std::cout << "Webhook with attachment delivered: HTTP " << res.status << "\n";
}
```

### 14. Téléchargement Sécurisé en Mémoire

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
> `SecureZeroMemory()` est une API Win32. Pour un effacement portable, utilisez `std::fill(buffer.begin(), buffer.end(), '\0')` ou `OPENSSL_cleanse(buffer.data(), buffer.size())` (OpenSSL est déjà lié).

### 15. Canaux de Chat en Direct

```cpp
// Send a message to a channel
NightAuthApp.sendChatMessage("Hello community!", "general");

// Fetch latest messages
std::vector<NightAuth::ChatMessage> messages = NightAuthApp.getChatMessages("general");
for (const auto& msg : messages) {
    std::cout << "[" << msg.author << "]: " << msg.message << "\n";
}
```

### 16. Gestionnaires d'Événements Temps Réel (SSE)

Abonnez-vous au flux d'événements serveur avec un callback recevant une `NightAuth::EventMessage` (champs : `id`, `type`, `data`). Le callback doit retourner un `bool` : retournez `false` pour arrêter l'écoute. Un transport de flux doit d'abord être configuré (voir `setStreamTransport` dans [Transports personnalisés](#25-custom-transports--server-key-pinning)) :

```cpp
std::atomic<bool> cancelStream{false};

NightAuthApp.subscribeEvents([](const NightAuth::EventMessage& ev) {
    std::cout << "[EVENT] " << ev.id << " " << ev.type << ": " << ev.data << "\n";
    return true; // return false to stop the stream
}, &cancelStream);
```

Vous pouvez aussi réagir aux notifications du cycle de vie en assignant les callbacks exposés sur l'instance `Api` :

```cpp
NightAuthApp.onSessionInvalidated = [](const std::string& reason) { /* session revoked */ };
NightAuthApp.onAppBanned          = [](const std::string& reason) { /* app banned */ };
NightAuthApp.onAppStatusChanged   = [](const std::string& status) { /* "app-paused" / "app-disabled" */ };
NightAuthApp.onVersionUpdated     = [](const std::string& version) { /* new version available */ };
NightAuthApp.onChatMessageReceived = [](const NightAuth::ChatMessage& msg, const std::string& room) { /* chat message */ };
```

### 17. Authentification à Deux Facteurs (2FA)

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

### 18. Cycle de Vie de Session & Déconnexion

```cpp
// Heartbeat ping to keep session alive
NightAuthApp.heartbeat();

// Revoke and terminate session
NightAuthApp.logout();

// Wipe all local session state (keys, user, app data) without a server call.
// Also called automatically by the destructor.
NightAuthApp.close();
```

### 19. Changer de Nom d'Utilisateur

Change le nom d'utilisateur du compte authentifié (la session est fermée pour des raisons de sécurité) :

```cpp
if (NightAuthApp.changeUsername("new_username", password)) {
    std::cout << "Username updated: " << NightAuthApp.user.username << "\n";
    // For security, the session is closed: sign in again with the new username.
}
```

### 20. Statistiques de l'Application & Utilisateurs en Ligne

Récupère les métriques agrégées de l'application et la liste des utilisateurs avec une session active :

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

Les compteurs au niveau de l'application sont aussi disponibles directement sur l'instance via `NightAuthApp.app` (un `NightAuth::AppData` avec `total_users`, `online_users`, `total_keys`, `version`). Les alias `user_data` / `app_data` référencent les mêmes membres `user` / `app`.

### 21. Tickets d'Accès

Demande un ticket à courte durée autorisant une opération à portée délimitée (utilisé en interne par `downloadFile` et `subscribeEvents`) :

```cpp
NightAuth::AccessTicket ticket;
if (NightAuthApp.requestAccessTicket("file", "385624", ticket)) {
    std::cout << "Ticket: " << ticket.ticket << " (expires in " << ticket.expires_in << "s)\n";
}
```

### 22. Vérifier le Hash du Fichier

Vérifie un condensat SHA-256 hexadécimal par rapport aux valeurs enregistrées côté serveur :

```cpp
if (NightAuthApp.verifyFileHash(sha256Hex)) {
    std::cout << "Hash verified.\n";
}
```

### 23. Gestion du Hardware ID

Le SDK dérive automatiquement un hardware ID (hostname + username, hachés). Vous pouvez le remplacer ou l'effacer par instance :

```cpp
NightAuthApp.setHardwareId("my-custom-hwid"); // send this HWID on login/register/license
NightAuthApp.clearHardwareId();               // revert to automatic derivation
```

Helpers statiques sur `NightAuth::SecurityProtections` :

```cpp
std::string hwid = NightAuth::SecurityProtections::getHardwareId();
NightAuth::SecurityProtections::setHardwareIdOverride("shared-hwid");
NightAuth::SecurityProtections::runSecurityChecks();   // reserved integrity hooks
NightAuth::SecurityProtections::checkSystemProxy();    // reserved proxy checks
```

### 24. Mises à Jour Signées

La mise à jour auto non signée est désactivée : `applyUpdate()` échoue toujours. Distribuez des manifestes de release signés et vérifiez-les avec `SignedUpdate` / `SignedUpdateInstaller` :

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

`NightAuth::VerifiedUpdate` expose `application`, `version`, `sequence`, `url`, `size`, `expiresAt` et `digest` (octets SHA-256).

### 25. Transports Personnalisés & Épinglage des Clés Serveur

Tout le trafic HTTP passe par des transports interchangeables. Les helpers libcurl inclus sont le choix par défaut ; le HTTP en clair est refusé sauf pour les hôtes loopback exacts avec opt-in explicite :

```cpp
NightAuthApp.setTransport(NightAuth::Api::makeCurlTransport(
    NightAuthApp.api_url, /*allowInsecureLoopback*/ false,
    /*connectTimeoutMs*/ 10000, /*totalTimeoutMs*/ 30000,
    /*maxResponseBytes*/ 5 * 1024 * 1024));

NightAuthApp.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(NightAuthApp.api_url));
```

Pour auto-héberger ou faire tourner les clés, épinglez vos propres clés de signature / sealed-box serveur avant `init()` (les deux doivent faire 64 caractères hexadécimaux) :

```cpp
if (!NightAuthApp.setServerKeys(signingKeyHex, sealedBoxKeyHex)) { /* invalid key format */ }
```

Les valeurs par défaut sont exposées comme constantes et membres statiques : `NightAuth::DEFAULT_API_URL`, `NightAuth::DEFAULT_SIGNING_PUBLIC_KEY`, `NightAuth::DEFAULT_SEALED_BOX_PUBLIC_KEY`, `NightAuth::Api::ServerSigningPublicKey` et `NightAuth::Api::ServerSealedBoxPublicKey`.

---

## 🖥️ Exécuter les clients Console et Bureau

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licence

Ce projet est sous licence **Elastic License 2.0**.

* Vous ne pouvez pas fournir le logiciel à des tiers en tant que service hébergé ou géré.
* Vous ne pouvez pas altérer, désactiver ou contourner le système de licences.
* Vous ne pouvez pas modifier ou supprimer les mentions de droit d'auteur.

Consultez le fichier [LICENSE](../LICENSE) pour le texte légal complet.

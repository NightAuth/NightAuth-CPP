

#ifndef NIGHTAUTH_CPP_HPP
#define NIGHTAUTH_CPP_HPP

#define _CXX23_SUPPORTED 1

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <atomic>

namespace NightAuth {

inline constexpr const char* DEFAULT_API_URL = "https://nightauth.org/api/v1";
inline constexpr const char* DEFAULT_SIGNING_PUBLIC_KEY = "cdb2285db481666ce40003aadb874c8751dbeb9a0672a37f740dfe881ecdc742";
inline constexpr const char* DEFAULT_SEALED_BOX_PUBLIC_KEY = "b773fc4f6cf1a0938e78e714c3d84cb386584bea24728923008471a6cd203213";
inline constexpr const char* HKDF_INFO = "nightauth-v1-e2e";
inline constexpr const char* SEAL_INFO = "nightauth-v1-init-seal";

class CryptographicException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ResponseTooLargeError : public std::runtime_error {
public:
    ResponseTooLargeError(const std::string& message, int httpStatus) : std::runtime_error(message), status(httpStatus) {}
    int status;
};

struct Response {
    bool success{false};
    std::string message;
    std::string error;

    std::string url;

    int statusCode{0};
};

struct SubscriptionData {
    std::string subscription;
    std::string key;
    std::string expiry;
    int64_t time_left{0};
};

struct UserData {
    std::string username;
    std::string email;
    std::string hwid;
    std::string ip;
    std::string status;
    std::string created_at;
    std::string last_login_at;
    std::string role;
    std::string expiry;
    std::vector<SubscriptionData> subscriptions;

    std::string subscription() const {
        if (!subscriptions.empty()) return subscriptions[0].subscription;
        return "";
    }
};

struct AppData {
    int64_t total_users{0};
    int64_t online_users{0};
    int64_t total_keys{0};
    std::string version;
};

struct AppStats {
    int64_t total_users{0};
    int64_t active_users{0};
    int64_t banned_users{0};
    int64_t total_licenses{0};
    int64_t used_licenses{0};
    int64_t unused_licenses{0};
    std::string version;
};

struct ChatMessage {
    std::string id;
    std::string author;
    std::string message;
    std::string timestamp;
    std::string status;
};

struct TwoFactorSetupData {
    std::string secret;
    std::string qr_data_url;
    std::string uri;
};

struct TwoFactorConfirmData {
    std::vector<std::string> backup_codes;
};

struct AccessTicket {
    std::string ticket;
    int64_t expires_in{0};
};

struct EventMessage {
    std::string id;
    std::string type;
    std::string data;
};

struct WebhookResult {
    int64_t status{0};
    bool truncated{false};
    std::string data;
};

struct WebhookOptions {
    bool hasContent{false};
    std::string content;
    bool hasParams{false};
    std::string params;
    bool hasFile{false};
    std::string fileName{"file.bin"};
    std::string fileData;
};

struct VerifiedUpdate {
    std::string application;
    std::string version;
    uint64_t sequence{0};
    std::string url;
    uint64_t size{0};
    int64_t expiresAt{0};
    std::vector<uint8_t> digest;
};

class SignedUpdate {
public:
    static VerifiedUpdate verify(const std::string& envelopeJson,
                                 const std::string& pinnedReleaseKeyHex,
                                 const std::string& expectedApplication,
                                 uint64_t minimumSequence = 0);

    static void download(const VerifiedUpdate& update,
                         const std::string& destination);
};

class SignedUpdateInstaller {
public:
    static std::string download(const std::string& manifestUrl,
                                const std::string& pinnedReleaseKeyHex,
                                const std::string& application,
                                const std::string& destination);
    static std::string getSequencePath(const std::string& application);
};

class SecurityProtections {
public:
    static void runSecurityChecks();
    static void checkSystemProxy();
    static std::string getHardwareId();

    static void setHardwareIdOverride(const std::string& hwid);
};

#ifdef NIGHTAUTH_TEST_HOOKS
struct TestAccess;
#endif

class Api {
public:

    static const char* ServerSigningPublicKey;
    static const char* ServerSealedBoxPublicKey;

    std::string name;
    std::string api_key;
    std::string version;
    std::string token;
    std::string api_url;
    std::string pendingDownloadUrl;
    std::string requiredVersion;

    Response response;
    UserData user;
    AppData app;
    UserData& user_data{user};
    AppData& app_data{app};

    std::function<void(const std::string&)> onSessionInvalidated;
    std::function<void(const std::string&)> onAppBanned;
    std::function<void(const std::string&)> onAppStatusChanged;
    std::function<void(const std::string&)> onVersionUpdated;
    std::function<void(const ChatMessage&, const std::string&)> onChatMessageReceived;

    Api(const std::string& name, const std::string& apiKey, const std::string& version = "1.0",
        const std::string& token = "", const std::string& apiUrl = DEFAULT_API_URL);
    ~Api();

    Api(const Api&) = delete;
    Api& operator=(const Api&) = delete;
    Api(Api&& other) noexcept;
    Api& operator=(Api&& other) noexcept;

    struct TransportResponse {
        int status{0};
        bool success{false};
        std::string k2, vk, sig;
        int64_t ts{0};
        std::string id, ct, n, rid;

        std::string body;

        std::string location;
    };
    using Transport = std::function<TransportResponse(const std::string& path, const std::string& method, const std::string& body)>;

    using StreamHandler = std::function<bool(const char* data, size_t len)>;
    using StreamTransport = std::function<int(const std::string& pathAndQuery, const std::string& lastEventId,
                                              const StreamHandler& onBytes, const std::function<bool()>& isCancelled)>;

    void setTransport(Transport t);

    bool setServerKeys(const std::string& signingKeyHex, const std::string& sealedBoxKeyHex);

    void setStreamTransport(StreamTransport t);

    static Transport makeCurlTransport(const std::string& baseUrl,
                                        bool allowInsecureLoopback = false,
                                        long connectTimeoutMs = 10000,
                                        long totalTimeoutMs = 30000,
                                        size_t maxResponseBytes = 5 * 1024 * 1024);

    static StreamTransport makeCurlStreamTransport(const std::string& baseUrl,
                                                   bool allowInsecureLoopback = false,
                                                   long connectTimeoutMs = 10000);

    void init();
    bool login(const std::string& username, const std::string& password, const std::string& totpCode = "");
    bool registerAccount(const std::string& username, const std::string& password, const std::string& key, const std::string& email = "");

    bool license(const std::string& key, const std::string& totpCode = "");

    void setHardwareId(const std::string& hwid);
    void clearHardwareId();

    bool upgrade(const std::string& username, const std::string& key);
    bool heartbeat();
    bool check();
    bool logout();

    bool changeUsername(const std::string& newUsername, const std::string& password);
    bool resetHwid(const std::string& username, const std::string& password, const std::string& totpCode = "");
    bool ban(const std::string& reason = "");
    bool checkBlacklist(const std::string& hwid = "");
    bool verifyFileHash(const std::string& hash);
    bool forgotPassword(const std::string& email);
    bool resetPassword(const std::string& token, const std::string& newPassword);

    bool setup2FA(TwoFactorSetupData& outData);
    bool confirm2FA(const std::string& code, TwoFactorConfirmData& outData);
    bool disable2FA(const std::string& password, const std::string& code);

    std::string getGlobalVar(const std::string& key);
    std::string getUserVar(const std::string& key);

    bool getUserVariables(std::vector<std::pair<std::string, std::string>>& out);
    bool setUserVar(const std::string& key, const std::string& value);
    bool log(const std::string& message);

    bool getStats(AppStats& outStats);
    std::vector<std::string> getOnlineUsers();
    std::vector<ChatMessage> getChatMessages(const std::string& channel = "general", int64_t before = 0,
                                             bool markRead = false);
    bool sendChatMessage(const std::string& message, const std::string& channel = "general");

    bool requestAccessTicket(const std::string& scope, const std::string& resourceId, AccessTicket& outTicket);

    bool downloadFile(const std::string& fileId, std::string& outContent);

    using EventCallback = std::function<bool(const EventMessage&)>;
    bool subscribeEvents(const EventCallback& onEvent, const std::atomic<bool>* cancel = nullptr,
                         const std::string& lastEventId = "");

    bool triggerWebhook(const std::string& webId, const std::string& payloadJson, WebhookResult& outResult);

    bool triggerWebhook(const std::string& webId, const std::string& payloadJson, WebhookResult& outResult,
                        const WebhookOptions& options);

    bool applyUpdate(const std::string& downloadUrl = "");
    std::string downloadSignedUpdate(const std::string& manifestUrl,
                                     const std::string& pinnedReleaseKeyHex,
                                     const std::string& destination = "");

    void close();

private:
    struct Impl;

    std::unique_ptr<Impl> m_impl;
    std::string m_sessionKey;
    uint8_t m_sharedKey[32]{};
    bool m_isInitialized{false};

    void checkInit();

    std::string hardwareId() const;
    void setError(const std::string& msg, const std::string& err = "");
    void setSuccess(const std::string& msg);
    void verifyHandshakeResponse(const TransportResponse& raw);

    std::string sendEncrypted(const std::string& path, const std::string& payloadJson, bool allowReinit = true);

    std::string sendEncryptedOnce(const std::string& path, const std::string& payloadJson);

    TransportResponse sendPlain(const std::string& path, const std::string& method, const std::string& body);

    void setFailureFromBody(const std::string& body, const char* fallbackMessage, const char* fallbackError);
    StreamTransport m_streamTransport;
    const WebhookOptions* m_pendingFile = nullptr;

#ifdef NIGHTAUTH_TEST_HOOKS
    friend struct TestAccess;
#endif
};

#ifdef NIGHTAUTH_TEST_HOOKS

namespace TestHooks {
    std::string hkdfSha256Hex(const std::string& ikmHex, const std::string& info, size_t outLen);

    void fillTransportResponse(Api::TransportResponse& out, const std::string& body);

    std::string canonicalJson(const std::string& json);

    std::string jsonField(const std::string& json, const std::string& key);

    void primeSession(Api& api, const std::string& sessionKey, const std::string& sharedKeyHex);

    std::string sessionKey(const Api& api);

    bool gunzipLimited(const std::string& gzip, size_t maxOut, std::string& out);
}
#endif

}

#endif

#ifdef NIGHTAUTH_IMPLEMENTATION
#ifndef NIGHTAUTH_CPP_IMPLEMENTATION_INCLUDED
#define NIGHTAUTH_CPP_IMPLEMENTATION_INCLUDED

#include <cstdlib>

#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <array>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <array>
#include <cctype>
#include <cstdio>
#include <algorithm>

#ifndef NIGHTAUTH_NO_CURL
#include <curl/curl.h>
#endif
#include <openssl/crypto.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace NightAuth {

const char* Api::ServerSigningPublicKey = "cdb2285db481666ce40003aadb874c8751dbeb9a0672a37f740dfe881ecdc742";
const char* Api::ServerSealedBoxPublicKey = "b773fc4f6cf1a0938e78e714c3d84cb386584bea24728923008471a6cd203213";

namespace Crypto {

using Bytes = std::vector<uint8_t>;

static void opensslCheck(int ok, const char* what) {
    if (ok != 1) {
        unsigned long e = ERR_get_error();
        char buf[256];
        ERR_error_string_n(e, buf, sizeof(buf));
        throw std::runtime_error(std::string("OpenSSL error in ") + what + ": " + buf);
    }
}

static std::string toHex(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; i++) oss << std::setw(2) << static_cast<int>(data[i]);
    return oss.str();
}

static Bytes fromHex(const std::string& hex) {
    if (hex.size() % 2 != 0) throw std::runtime_error("invalid hex string");
    Bytes out(hex.size() / 2);
    for (size_t i = 0; i < out.size(); i++) {
        out[i] = static_cast<uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    }
    return out;
}

static std::string base64Encode(const uint8_t* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    size_t i = 0;
    for (; i + 2 < len; i += 3) {
        uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out.push_back(tbl[(n >> 18) & 0x3f]);
        out.push_back(tbl[(n >> 12) & 0x3f]);
        out.push_back(tbl[(n >> 6) & 0x3f]);
        out.push_back(tbl[n & 0x3f]);
    }
    if (len - i == 1) {
        uint32_t n = data[i] << 16;
        out.push_back(tbl[(n >> 18) & 0x3f]);
        out.push_back(tbl[(n >> 12) & 0x3f]);
        out.push_back('=');
        out.push_back('=');
    } else if (len - i == 2) {
        uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
        out.push_back(tbl[(n >> 18) & 0x3f]);
        out.push_back(tbl[(n >> 12) & 0x3f]);
        out.push_back(tbl[(n >> 6) & 0x3f]);
        out.push_back('=');
    }
    return out;
}

static Bytes base64Decode(const std::string& in) {
    auto val = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };
    Bytes out;
    uint32_t buf = 0;
    int bits = 0;
    for (char c : in) {
        if (c == '=' || c == '\n' || c == '\r') continue;
        int v = val(c);
        if (v < 0) continue;
        buf = (buf << 6) | v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<uint8_t>((buf >> bits) & 0xff));
        }
    }
    return out;
}

static Bytes infoBytes(const std::string& literal) {
    return Bytes(literal.begin(), literal.end());
}

static Bytes hkdfSha256(const Bytes& ikm, const Bytes& info, size_t outLen) {
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    if (!pctx) throw std::runtime_error("EVP_PKEY_CTX_new_id(HKDF) failed");
    struct Guard { EVP_PKEY_CTX* p; ~Guard() { EVP_PKEY_CTX_free(p); } } guard{pctx};

    opensslCheck(EVP_PKEY_derive_init(pctx), "EVP_PKEY_derive_init");
    opensslCheck(EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()), "set_hkdf_md");

    opensslCheck(EVP_PKEY_CTX_set1_hkdf_salt(pctx, reinterpret_cast<const unsigned char*>(""), 0), "set1_hkdf_salt");
    opensslCheck(EVP_PKEY_CTX_set1_hkdf_key(pctx, ikm.data(), static_cast<int>(ikm.size())), "set1_hkdf_key");
    opensslCheck(EVP_PKEY_CTX_add1_hkdf_info(pctx, info.data(), static_cast<int>(info.size())), "add1_hkdf_info");

    Bytes out(outLen);
    size_t len = outLen;
    opensslCheck(EVP_PKEY_derive(pctx, out.data(), &len), "EVP_PKEY_derive");
    out.resize(len);
    return out;
}

struct X25519KeyPair {
    EVP_PKEY* pkey{nullptr};

    X25519KeyPair() = default;
    explicit X25519KeyPair(EVP_PKEY* p) : pkey(p) {}
    ~X25519KeyPair() { if (pkey) EVP_PKEY_free(pkey); }

    X25519KeyPair(const X25519KeyPair&) = delete;
    X25519KeyPair& operator=(const X25519KeyPair&) = delete;

    X25519KeyPair(X25519KeyPair&& other) noexcept : pkey(other.pkey) { other.pkey = nullptr; }
    X25519KeyPair& operator=(X25519KeyPair&& other) noexcept {
        if (this != &other) {
            if (pkey) EVP_PKEY_free(pkey);
            pkey = other.pkey;
            other.pkey = nullptr;
        }
        return *this;
    }
};

static X25519KeyPair generateX25519() {
    X25519KeyPair kp;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr);
    if (!pctx) throw std::runtime_error("EVP_PKEY_CTX_new_id(X25519) failed");
    struct Guard { EVP_PKEY_CTX* p; ~Guard() { EVP_PKEY_CTX_free(p); } } guard{pctx};
    opensslCheck(EVP_PKEY_keygen_init(pctx), "EVP_PKEY_keygen_init");
    opensslCheck(EVP_PKEY_keygen(pctx, &kp.pkey), "EVP_PKEY_keygen(X25519)");
    return kp;
}

static Bytes x25519PublicRaw(EVP_PKEY* pkey) {
    Bytes raw(32);
    size_t len = 32;
    opensslCheck(EVP_PKEY_get_raw_public_key(pkey, raw.data(), &len), "get_raw_public_key");
    raw.resize(len);
    return raw;
}

static EVP_PKEY* x25519PublicFromRaw(const Bytes& raw32) {
    if (raw32.size() != 32) throw std::runtime_error("invalid X25519 public key length");
    EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, nullptr, raw32.data(), raw32.size());
    if (!pkey) throw std::runtime_error("EVP_PKEY_new_raw_public_key(X25519) failed");
    return pkey;
}

static Bytes x25519Dh(EVP_PKEY* priv, const Bytes& peerPubRaw) {
    EVP_PKEY* peer = x25519PublicFromRaw(peerPubRaw);
    struct Guard { EVP_PKEY* p; ~Guard() { EVP_PKEY_free(p); } } guard{peer};

    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new(priv, nullptr);
    if (!pctx) throw std::runtime_error("EVP_PKEY_CTX_new(priv) failed");
    struct Guard2 { EVP_PKEY_CTX* p; ~Guard2() { EVP_PKEY_CTX_free(p); } } guard2{pctx};

    opensslCheck(EVP_PKEY_derive_init(pctx), "EVP_PKEY_derive_init(dh)");
    opensslCheck(EVP_PKEY_derive_set_peer(pctx, peer), "EVP_PKEY_derive_set_peer");
    size_t len = 0;
    opensslCheck(EVP_PKEY_derive(pctx, nullptr, &len), "EVP_PKEY_derive(len)");
    Bytes secret(len);
    opensslCheck(EVP_PKEY_derive(pctx, secret.data(), &len), "EVP_PKEY_derive(secret)");
    secret.resize(len);
    return secret;
}

static bool ed25519Verify(const Bytes& pubRaw, const Bytes& data, const Bytes& sig) {
    if (pubRaw.size() != 32) return false;
    EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, pubRaw.data(), pubRaw.size());
    if (!pkey) return false;
    struct Guard { EVP_PKEY* p; ~Guard() { EVP_PKEY_free(p); } } guard{pkey};

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) return false;
    struct Guard2 { EVP_MD_CTX* p; ~Guard2() { EVP_MD_CTX_free(p); } } guard2{mdctx};

    if (EVP_DigestVerifyInit(mdctx, nullptr, nullptr, nullptr, pkey) != 1) return false;

    int rc = EVP_DigestVerify(mdctx, sig.data(), sig.size(), data.data(), data.size());
    return rc == 1;
}

struct AeadResult { std::string ctB64; std::string nonceB64; };

static AeadResult aesGcmEncrypt(const Bytes& key, const Bytes& plaintext, const Bytes& aad) {
    if (key.size() != 32) throw std::runtime_error("AES-256-GCM key must be 32 bytes");
    Bytes nonce(12);
    opensslCheck(RAND_bytes(nonce.data(), static_cast<int>(nonce.size())), "RAND_bytes(nonce)");

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    struct Guard { EVP_CIPHER_CTX* p; ~Guard() { EVP_CIPHER_CTX_free(p); } } guard{ctx};

    opensslCheck(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr), "EncryptInit(alg)");
    opensslCheck(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce.size()), nullptr), "set ivlen");
    opensslCheck(EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data()), "EncryptInit(key/iv)");

    int outLen = 0;
    if (!aad.empty()) opensslCheck(EVP_EncryptUpdate(ctx, nullptr, &outLen, aad.data(), static_cast<int>(aad.size())), "EncryptUpdate(aad)");

    Bytes ct(plaintext.size());
    int len1 = 0;
    opensslCheck(EVP_EncryptUpdate(ctx, ct.data(), &len1, plaintext.data(), static_cast<int>(plaintext.size())), "EncryptUpdate(pt)");
    int len2 = 0;
    opensslCheck(EVP_EncryptFinal_ex(ctx, ct.data() + len1, &len2), "EncryptFinal");
    ct.resize(len1 + len2);

    Bytes tag(16);
    opensslCheck(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()), "get tag");

    Bytes ctWithTag;
    ctWithTag.reserve(ct.size() + tag.size());
    ctWithTag.insert(ctWithTag.end(), ct.begin(), ct.end());
    ctWithTag.insert(ctWithTag.end(), tag.begin(), tag.end());

    AeadResult r;
    r.ctB64 = base64Encode(ctWithTag.data(), ctWithTag.size());
    r.nonceB64 = base64Encode(nonce.data(), nonce.size());
    return r;
}

static Bytes aesGcmDecrypt(const Bytes& key, const std::string& ctB64, const std::string& nonceB64, const Bytes& aad) {
    if (key.size() != 32) throw std::runtime_error("AES-256-GCM key must be 32 bytes");
    Bytes nonce = base64Decode(nonceB64);
    Bytes combined = base64Decode(ctB64);
    if (nonce.size() != 12) throw std::runtime_error("invalid nonce size");
    if (combined.size() < 16) throw std::runtime_error("invalid encrypted response size");
    Bytes tag(combined.end() - 16, combined.end());
    Bytes ct(combined.begin(), combined.end() - 16);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    struct Guard { EVP_CIPHER_CTX* p; ~Guard() { EVP_CIPHER_CTX_free(p); } } guard{ctx};

    opensslCheck(EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr), "DecryptInit(alg)");
    opensslCheck(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce.size()), nullptr), "set ivlen");
    opensslCheck(EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data()), "DecryptInit(key/iv)");

    int outLen = 0;
    if (!aad.empty()) opensslCheck(EVP_DecryptUpdate(ctx, nullptr, &outLen, aad.data(), static_cast<int>(aad.size())), "DecryptUpdate(aad)");

    Bytes pt(ct.size());
    int len1 = 0;
    opensslCheck(EVP_DecryptUpdate(ctx, pt.data(), &len1, ct.data(), static_cast<int>(ct.size())), "DecryptUpdate(ct)");

    opensslCheck(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, const_cast<uint8_t*>(tag.data())), "set tag");
    int len2 = 0;

    if (EVP_DecryptFinal_ex(ctx, pt.data() + len1, &len2) != 1) {
        throw std::runtime_error("AEAD authentication failed (tampered ciphertext or wrong key)");
    }
    pt.resize(len1 + len2);
    return pt;
}

static std::string sealInit(const std::string& plaintextJson, const std::string& recipientPubHex) {
    Bytes recipientRaw = fromHex(recipientPubHex);
    X25519KeyPair ephemeral = generateX25519();
    Bytes ephemeralPubRaw = x25519PublicRaw(ephemeral.pkey);

    Bytes secret = x25519Dh(ephemeral.pkey, recipientRaw);
    Bytes key = hkdfSha256(secret, infoBytes("nightauth-v1-init-seal"), 32);

    Bytes nonce(12);
    opensslCheck(RAND_bytes(nonce.data(), 12), "RAND_bytes(seal nonce)");

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    struct Guard { EVP_CIPHER_CTX* p; ~Guard() { EVP_CIPHER_CTX_free(p); } } guard{ctx};

    opensslCheck(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr), "seal EncryptInit(alg)");
    opensslCheck(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr), "seal set ivlen");
    opensslCheck(EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data()), "seal EncryptInit(key/iv)");

    int outLen = 0;
    opensslCheck(EVP_EncryptUpdate(ctx, nullptr, &outLen, ephemeralPubRaw.data(), static_cast<int>(ephemeralPubRaw.size())), "seal aad");

    Bytes plaintext(plaintextJson.begin(), plaintextJson.end());
    Bytes ct(plaintext.size());
    int len1 = 0;
    opensslCheck(EVP_EncryptUpdate(ctx, ct.data(), &len1, plaintext.data(), static_cast<int>(plaintext.size())), "seal EncryptUpdate");
    int len2 = 0;
    opensslCheck(EVP_EncryptFinal_ex(ctx, ct.data() + len1, &len2), "seal EncryptFinal");
    ct.resize(len1 + len2);

    Bytes tag(16);
    opensslCheck(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()), "seal get tag");

    Bytes out;
    out.reserve(32 + 12 + 16 + ct.size());
    out.insert(out.end(), ephemeralPubRaw.begin(), ephemeralPubRaw.end());
    out.insert(out.end(), nonce.begin(), nonce.end());
    out.insert(out.end(), tag.begin(), tag.end());
    out.insert(out.end(), ct.begin(), ct.end());
    return base64Encode(out.data(), out.size());
}

static bool verifyInitSignature(const std::string& k2Hex, const std::string& vkHex, int64_t ts, const std::string& sigB64) {
    try {
        Bytes vkRaw = fromHex(vkHex);
        Bytes sig = base64Decode(sigB64);
        std::string dataStr = k2Hex + "." + vkHex + "." + std::to_string(ts);
        Bytes data(dataStr.begin(), dataStr.end());
        return ed25519Verify(vkRaw, data, sig);
    } catch (...) {
        return false;
    }
}

}

namespace Json {

enum class Type { Null, Bool, Number, String, Array, Object };

struct Value {
    Type type{Type::Null};
    bool boolVal{false};
    double numVal{0};
    std::string strVal;
    std::vector<Value> arrVal;
    std::vector<std::pair<std::string, Value>> objVal;

    bool isObject() const { return type == Type::Object; }
    bool isBool() const { return type == Type::Bool; }
    bool asBool() const { return boolVal; }

    const Value* find(const std::string& key) const {
        if (type != Type::Object) return nullptr;
        for (const auto& kv : objVal) {
            if (kv.first == key) return &kv.second;
        }
        return nullptr;
    }
};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

class Parser {
public:
    explicit Parser(const std::string& s) : m_s(s), m_i(0) {}

    Value parse() {
        skipWs();
        Value v = parseValue();
        skipWs();
        if (m_i != m_s.size()) throw ParseError("trailing data after JSON value");
        return v;
    }

private:
    const std::string& m_s;
    size_t m_i;

    char peek() { if (m_i >= m_s.size()) throw ParseError("unexpected end of JSON input"); return m_s[m_i]; }
    char next() { char c = peek(); m_i++; return c; }
    void skipWs() { while (m_i < m_s.size() && (m_s[m_i] == ' ' || m_s[m_i] == '\t' || m_s[m_i] == '\n' || m_s[m_i] == '\r')) m_i++; }
    void expect(char c) { if (next() != c) throw ParseError(std::string("expected '") + c + "'"); }

    Value parseValue() {
        skipWs();
        char c = peek();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
        throw ParseError("unexpected character in JSON input");
    }

    Value parseObject() {
        Value v; v.type = Type::Object;
        expect('{');
        skipWs();
        if (peek() == '}') { m_i++; return v; }
        while (true) {
            skipWs();
            Value keyVal = parseString();
            skipWs();
            expect(':');
            Value val = parseValue();
            v.objVal.emplace_back(keyVal.strVal, std::move(val));
            skipWs();
            char c = next();
            if (c == ',') continue;
            if (c == '}') break;
            throw ParseError("expected ',' or '}' in object");
        }
        return v;
    }

    Value parseArray() {
        Value v; v.type = Type::Array;
        expect('[');
        skipWs();
        if (peek() == ']') { m_i++; return v; }
        while (true) {
            Value val = parseValue();
            v.arrVal.push_back(std::move(val));
            skipWs();
            char c = next();
            if (c == ',') continue;
            if (c == ']') break;
            throw ParseError("expected ',' or ']' in array");
        }
        return v;
    }

    Value parseString() {
        Value v; v.type = Type::String;
        expect('"');
        std::string out;
        while (true) {
            char c = next();
            if (c == '"') break;
            if (c == '\\') {
                char e = next();
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'u': {
                        if (m_i + 4 > m_s.size()) throw ParseError("truncated \\u escape");
                        unsigned code = 0;
                        for (int k = 0; k < 4; k++) {
                            char h = next();
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= (h - '0');
                            else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                            else throw ParseError("invalid \\u escape");
                        }

                        if (code < 0x80) out += static_cast<char>(code);
                        else if (code < 0x800) {
                            out += static_cast<char>(0xC0 | (code >> 6));
                            out += static_cast<char>(0x80 | (code & 0x3F));
                        } else {
                            out += static_cast<char>(0xE0 | (code >> 12));
                            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                            out += static_cast<char>(0x80 | (code & 0x3F));
                        }
                        break;
                    }
                    default: throw ParseError("invalid escape sequence");
                }
            } else {
                out += c;
            }
        }
        v.strVal = std::move(out);
        return v;
    }

    Value parseBool() {
        Value v; v.type = Type::Bool;
        if (m_s.compare(m_i, 4, "true") == 0) { v.boolVal = true; m_i += 4; }
        else if (m_s.compare(m_i, 5, "false") == 0) { v.boolVal = false; m_i += 5; }
        else throw ParseError("invalid literal (expected true/false)");
        return v;
    }

    Value parseNull() {
        if (m_s.compare(m_i, 4, "null") != 0) throw ParseError("invalid literal (expected null)");
        m_i += 4;
        Value v; v.type = Type::Null;
        return v;
    }

    Value parseNumber() {
        size_t start = m_i;
        if (peek() == '-') m_i++;
        while (m_i < m_s.size() && isdigit(static_cast<unsigned char>(m_s[m_i]))) m_i++;
        if (m_i < m_s.size() && m_s[m_i] == '.') {
            m_i++;
            while (m_i < m_s.size() && isdigit(static_cast<unsigned char>(m_s[m_i]))) m_i++;
        }
        if (m_i < m_s.size() && (m_s[m_i] == 'e' || m_s[m_i] == 'E')) {
            m_i++;
            if (m_i < m_s.size() && (m_s[m_i] == '+' || m_s[m_i] == '-')) m_i++;
            while (m_i < m_s.size() && isdigit(static_cast<unsigned char>(m_s[m_i]))) m_i++;
        }
        Value v; v.type = Type::Number;
        v.numVal = std::stod(m_s.substr(start, m_i - start));
        return v;
    }
};

static Value parse(const std::string& s) { return Parser(s).parse(); }

static bool hasRootSuccessTrue(const std::string& s) {
    try {
        Value v = parse(s);
        const Value* f = v.find("success");
        return f != nullptr && f->isBool() && f->asBool();
    } catch (...) {
        return false;
    }
}

static std::string quoteString(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\n"; break;
            case '\r': out += "\r"; break;
            case '\t': out += "\t"; break;
            case '\b': out += "\b"; break;
            case '\f': out += "\f"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out + "\"";
}

static std::string serialize(const Value& v) {
    switch (v.type) {
        case Type::Null: return "null";
        case Type::Bool: return v.boolVal ? "true" : "false";
        case Type::Number: {
            char buf[64];
            if (v.numVal == static_cast<double>(static_cast<long long>(v.numVal)) && v.numVal > -1e15 && v.numVal < 1e15) {
                std::snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(v.numVal));
            } else {
                std::snprintf(buf, sizeof(buf), "%.17g", v.numVal);
            }
            return buf;
        }
        case Type::String: return quoteString(v.strVal);
        case Type::Array: {
            std::string out = "[";
            for (size_t i = 0; i < v.arrVal.size(); ++i) {
                if (i) out += ",";
                out += serialize(v.arrVal[i]);
            }
            return out + "]";
        }
        case Type::Object: {
            std::vector<const std::pair<std::string, Value>*> items;
            for (const auto& kv : v.objVal) items.push_back(&kv);
            std::stable_sort(items.begin(), items.end(),
                             [](const std::pair<std::string, Value>* a, const std::pair<std::string, Value>* b) {
                                 return a->first < b->first;
                             });
            std::string out = "{";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i) out += ",";
                out += quoteString(items[i]->first) + ":" + serialize(items[i]->second);
            }
            return out + "}";
        }
    }
    return "null";
}

}

namespace Gzip {
namespace {

const uint16_t kLenBase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                               35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
const uint8_t kLenExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                               3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
const uint16_t kDistBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129,
                                193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
const uint8_t kDistExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                                7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
const uint8_t kCodeOrder[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

class BitReader {
public:
    BitReader(const uint8_t* data, size_t size, size_t pos) : m_data(data), m_size(size), m_pos(pos) {}

    int bit() {
        if (m_count == 0) {
            if (m_pos >= m_size) {
                m_error = true;
                return 0;
            }
            m_buf = m_data[m_pos++];
            m_count = 8;
        }
        const int b = m_buf & 1;
        m_buf >>= 1;
        --m_count;
        return b;
    }
    uint32_t bits(int n) {
        uint32_t v = 0;
        for (int i = 0; i < n; ++i) v |= static_cast<uint32_t>(bit()) << i;
        return v;
    }
    void alignToByte() {
        m_buf = 0;
        m_count = 0;
    }
    void skipTo(size_t pos) { m_pos = pos; }
    bool error() const { return m_error; }
    size_t pos() const { return m_pos; }
    size_t size() const { return m_size; }
    const uint8_t* data() const { return m_data; }

private:
    const uint8_t* m_data;
    size_t m_size;
    size_t m_pos;
    uint32_t m_buf = 0;
    int m_count = 0;
    bool m_error = false;
};

struct Huffman {
    uint16_t count[16];
    std::vector<uint16_t> symbol;
};

bool buildHuffman(Huffman& h, const uint16_t* length, int n) {
    std::memset(h.count, 0, sizeof(h.count));
    h.symbol.assign(static_cast<size_t>(n), 0);
    for (int s = 0; s < n; ++s) h.count[length[s]]++;
    if (h.count[0] == n) return true;
    int left = 1;
    for (int len = 1; len < 16; ++len) {
        left <<= 1;
        left -= h.count[len];
        if (left < 0) return false;
    }
    uint16_t offs[16];
    offs[1] = 0;
    for (int len = 1; len < 15; ++len) offs[len + 1] = static_cast<uint16_t>(offs[len] + h.count[len]);
    for (int s = 0; s < n; ++s) {
        if (length[s] != 0) h.symbol[offs[length[s]]++] = static_cast<uint16_t>(s);
    }
    return true;
}

int decodeSymbol(BitReader& br, const Huffman& h) {
    int code = 0;
    int first = 0;
    int index = 0;
    for (int len = 1; len < 16; ++len) {
        code |= br.bit();
        const int count = h.count[len];
        if (code - count < first) return h.symbol[static_cast<size_t>(index + (code - first))];
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -1;
}

bool inflateCodes(BitReader& br, std::string& out, size_t maxOut, const Huffman& lit, const Huffman& dist) {
    for (;;) {
        const int sym = decodeSymbol(br, lit);
        if (br.error() || sym < 0) return false;
        if (sym < 256) {
            if (out.size() >= maxOut) return false;
            out.push_back(static_cast<char>(sym));
        } else if (sym == 256) {
            return true;
        } else {
            const int li = sym - 257;
            if (li >= 29) return false;
            const size_t len = kLenBase[li] + br.bits(kLenExtra[li]);
            const int di = decodeSymbol(br, dist);
            if (br.error() || di < 0 || di >= 30) return false;
            const size_t d = kDistBase[di] + br.bits(kDistExtra[di]);
            if (br.error() || d > out.size() || len > maxOut - out.size()) return false;
            const size_t start = out.size() - d;
            for (size_t i = 0; i < len; ++i) {
                const char c = out[start + i];
                out.push_back(c);
            }
        }
    }
}

bool inflateStored(BitReader& br, std::string& out, size_t maxOut) {
    br.alignToByte();
    const size_t p = br.pos();
    if (p + 4 > br.size()) return false;
    const uint8_t* d = br.data();
    const uint32_t len = static_cast<uint32_t>(d[p]) | (static_cast<uint32_t>(d[p + 1]) << 8);
    const uint32_t nlen = static_cast<uint32_t>(d[p + 2]) | (static_cast<uint32_t>(d[p + 3]) << 8);
    if (len != (~nlen & 0xffffu)) return false;
    if (p + 4 + len > br.size() || len > maxOut - out.size()) return false;
    out.append(reinterpret_cast<const char*>(d + p + 4), len);
    br.skipTo(p + 4 + len);
    return true;
}

bool inflateFixed(BitReader& br, std::string& out, size_t maxOut) {
    uint16_t lengths[288];
    for (int i = 0; i < 144; ++i) lengths[i] = 8;
    for (int i = 144; i < 256; ++i) lengths[i] = 9;
    for (int i = 256; i < 280; ++i) lengths[i] = 7;
    for (int i = 280; i < 288; ++i) lengths[i] = 8;
    uint16_t distLengths[30];
    for (auto& v : distLengths) v = 5;
    Huffman lit;
    Huffman dist;
    if (!buildHuffman(lit, lengths, 288) || !buildHuffman(dist, distLengths, 30)) return false;
    return inflateCodes(br, out, maxOut, lit, dist);
}

bool inflateDynamic(BitReader& br, std::string& out, size_t maxOut) {
    const int nlen = static_cast<int>(br.bits(5)) + 257;
    const int ndist = static_cast<int>(br.bits(5)) + 1;
    const int ncode = static_cast<int>(br.bits(4)) + 4;
    if (br.error() || nlen > 286 || ndist > 30) return false;

    uint16_t codeLengths[19] = {0};
    for (int i = 0; i < ncode; ++i) codeLengths[kCodeOrder[i]] = static_cast<uint16_t>(br.bits(3));
    Huffman codeCode;
    if (!buildHuffman(codeCode, codeLengths, 19)) return false;

    const int total = nlen + ndist;
    uint16_t all[286 + 30] = {0};
    int index = 0;
    while (index < total) {
        const int sym = decodeSymbol(br, codeCode);
        if (br.error() || sym < 0) return false;
        if (sym < 16) {
            all[index++] = static_cast<uint16_t>(sym);
            continue;
        }
        uint16_t value = 0;
        int rep = 0;
        if (sym == 16) {
            if (index == 0) return false;
            value = all[index - 1];
            rep = 3 + static_cast<int>(br.bits(2));
        } else if (sym == 17) {
            rep = 3 + static_cast<int>(br.bits(3));
        } else {
            rep = 11 + static_cast<int>(br.bits(7));
        }
        if (br.error() || index + rep > total) return false;
        while (rep-- > 0) all[index++] = value;
    }
    if (all[256] == 0) return false;

    Huffman lit;
    Huffman dist;
    if (!buildHuffman(lit, all, nlen)) return false;
    if (!buildHuffman(dist, all + nlen, ndist)) return false;
    return inflateCodes(br, out, maxOut, lit, dist);
}

bool inflateBlocks(BitReader& br, std::string& out, size_t maxOut) {
    int last = 0;
    do {
        last = br.bit();
        const int type = static_cast<int>(br.bits(2));
        if (br.error()) return false;
        bool ok = false;
        if (type == 0) {
            ok = inflateStored(br, out, maxOut);
        } else if (type == 1) {
            ok = inflateFixed(br, out, maxOut);
        } else if (type == 2) {
            ok = inflateDynamic(br, out, maxOut);
        } else {
            return false;
        }
        if (!ok || br.error()) return false;
    } while (last == 0);
    return true;
}

uint32_t crc32Of(const char* data, size_t n) {
    static const std::array<uint32_t, 256> table = [] {
        std::array<uint32_t, 256> t{};
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            t[i] = c;
        }
        return t;
    }();
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; ++i) c = table[(c ^ static_cast<uint8_t>(data[i])) & 0xFFu] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

uint32_t le32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

}

bool gunzip(const std::string& gz, size_t maxOut, std::string& out) {
    out.clear();
    const uint8_t* d = reinterpret_cast<const uint8_t*>(gz.data());
    const size_t n = gz.size();
    if (n == 0) return false;
    size_t p = 0;
    while (p < n) {
        if (n - p < 18 || d[p] != 0x1f || d[p + 1] != 0x8b || d[p + 2] != 8) return false;
        const uint8_t flags = d[p + 3];
        if ((flags & 0xe0) != 0) return false;
        size_t q = p + 10;
        if (flags & 0x04) {
            if (q + 2 > n) return false;
            q += 2 + (static_cast<size_t>(d[q]) | (static_cast<size_t>(d[q + 1]) << 8));
        }
        if (flags & 0x08) {
            while (q < n && d[q] != 0) ++q;
            if (q >= n) return false;
            ++q;
        }
        if (flags & 0x10) {
            while (q < n && d[q] != 0) ++q;
            if (q >= n) return false;
            ++q;
        }
        if (flags & 0x02) q += 2;
        if (q > n) return false;

        const size_t before = out.size();
        BitReader br(d, n, q);
        if (!inflateBlocks(br, out, maxOut)) return false;
        br.alignToByte();
        const size_t t = br.pos();
        if (t + 8 > n) return false;
        const size_t produced = out.size() - before;
        if (le32(d + t) != crc32Of(out.data() + before, produced)) return false;
        if (le32(d + t + 4) != static_cast<uint32_t>(produced)) return false;
        p = t + 8;
    }
    return true;
}

}

struct Api::Impl {

    int lastStatus{0};

    bool hasHwid{false};
    std::string hwid;
    EVP_PKEY* clientEphemeral{nullptr};
    std::vector<uint8_t> sharedKey;
    Api::Transport transport;

    std::string signingPin{Api::ServerSigningPublicKey};
    std::string sealPin{Api::ServerSealedBoxPublicKey};

    ~Impl() {
        zero();
    }
    void zero() {
        if (clientEphemeral) {
            EVP_PKEY_free(clientEphemeral);
            clientEphemeral = nullptr;
        }
        if (!sharedKey.empty()) {
            OPENSSL_cleanse(sharedKey.data(), sharedKey.size());
            sharedKey.clear();
        }
    }
};

Api::Api(const std::string& name_, const std::string& apiKey_, const std::string& version_,
         const std::string& token_, const std::string& apiUrl_)
    : name(name_), api_key(apiKey_), version(version_.empty() ? "1.0" : version_),
      token(token_), api_url(apiUrl_.empty() ? DEFAULT_API_URL : apiUrl_),
      user_data(user), app_data(app),
      m_impl(std::make_unique<Impl>()) {
}

Api::~Api() = default;

Api::Api(Api&& other) noexcept
    : name(std::move(other.name)), api_key(std::move(other.api_key)), version(std::move(other.version)),
      token(std::move(other.token)), api_url(std::move(other.api_url)),
      response(std::move(other.response)), user(std::move(other.user)), app(std::move(other.app)),
      onSessionInvalidated(std::move(other.onSessionInvalidated)), onAppBanned(std::move(other.onAppBanned)),
      onAppStatusChanged(std::move(other.onAppStatusChanged)), onVersionUpdated(std::move(other.onVersionUpdated)),
      onChatMessageReceived(std::move(other.onChatMessageReceived)),
      pendingDownloadUrl(std::move(other.pendingDownloadUrl)), requiredVersion(std::move(other.requiredVersion)),
      m_impl(std::move(other.m_impl)), m_sessionKey(std::move(other.m_sessionKey)),
      m_isInitialized(other.m_isInitialized), m_streamTransport(std::move(other.m_streamTransport)) {
    std::memcpy(m_sharedKey, other.m_sharedKey, sizeof(m_sharedKey));

    OPENSSL_cleanse(other.m_sharedKey, sizeof(other.m_sharedKey));
    other.m_isInitialized = false;
}

Api& Api::operator=(Api&& other) noexcept {
    if (this != &other) {
        name = std::move(other.name);
        api_key = std::move(other.api_key);
        version = std::move(other.version);
        token = std::move(other.token);
        api_url = std::move(other.api_url);
        response = std::move(other.response);
        user = std::move(other.user);
        app = std::move(other.app);
        onSessionInvalidated = std::move(other.onSessionInvalidated);
        onAppBanned = std::move(other.onAppBanned);
        onAppStatusChanged = std::move(other.onAppStatusChanged);
        onVersionUpdated = std::move(other.onVersionUpdated);
        onChatMessageReceived = std::move(other.onChatMessageReceived);
        pendingDownloadUrl = std::move(other.pendingDownloadUrl);
        requiredVersion = std::move(other.requiredVersion);
        m_impl = std::move(other.m_impl);
        m_streamTransport = std::move(other.m_streamTransport);
        m_sessionKey = std::move(other.m_sessionKey);
        m_isInitialized = other.m_isInitialized;
        std::memcpy(m_sharedKey, other.m_sharedKey, sizeof(m_sharedKey));
        OPENSSL_cleanse(other.m_sharedKey, sizeof(other.m_sharedKey));
        other.m_isInitialized = false;
    }
    return *this;
}

void Api::setTransport(Transport t) { m_impl->transport = std::move(t); }

static bool isKeyHex(const std::string& s) {
    if (s.size() != 64) return false;
    for (char c : s)
        if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

bool Api::setServerKeys(const std::string& signingKeyHex, const std::string& sealedBoxKeyHex) {
    if (!isKeyHex(signingKeyHex) || !isKeyHex(sealedBoxKeyHex)) return false;

    auto lower = [](std::string v) {
        for (char& c : v) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return v;
    };
    m_impl->signingPin = lower(signingKeyHex);
    m_impl->sealPin = lower(sealedBoxKeyHex);
    return true;
}

static std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c;
        }
    }
    return out;
}

static std::string transportFailureCode(const std::exception& e) {
    const std::string m = e.what();
    if (m.find("maximum allowed size") != std::string::npos) return "response_too_large";
    if (m.find("Timeout") != std::string::npos || m.find("timed out") != std::string::npos) return "timeout";
    return "network_error";
}

static int failureStatus(const std::exception& e) {
    const auto* tooLarge = dynamic_cast<const ResponseTooLargeError*>(&e);
    return tooLarge ? tooLarge->status : 0;
}

static std::string failureJson(const std::string& code, const std::string& message) {
    return "{\"success\":false,\"error\":\"" + jsonEscape(code) + "\",\"message\":\"" + jsonEscape(message) + "\"}";
}

static bool isJsonObject(const std::string& text) {
    try {
        return Json::parse(text).isObject();
    } catch (const Json::ParseError&) {
        return false;
    }
}

static void fillFromJson(Api::TransportResponse& r, const std::string& body) {
    r.body = body;
    try {
        Json::Value v = Json::parse(body);
        if (!v.isObject()) return;
        if (const Json::Value* f = v.find("success")) if (f->isBool()) r.success = f->asBool();
        auto str = [&](const char* key, std::string& out) {
            if (const Json::Value* f = v.find(key)) if (f->type == Json::Type::String) out = f->strVal;
        };
        str("k2", r.k2); str("vk", r.vk); str("sig", r.sig);
        str("id", r.id); str("ct", r.ct); str("n", r.n); str("rid", r.rid);
        if (const Json::Value* f = v.find("ts")) if (f->type == Json::Type::Number) r.ts = static_cast<int64_t>(f->numVal);
    } catch (const Json::ParseError&) {

    }
}

void Api::setHardwareId(const std::string& hwid) {
    m_impl->hasHwid = true;
    m_impl->hwid = hwid;
}

void Api::clearHardwareId() {
    m_impl->hasHwid = false;
    m_impl->hwid.clear();
}

std::string Api::hardwareId() const {
    return m_impl->hasHwid ? m_impl->hwid : SecurityProtections::getHardwareId();
}

void Api::checkInit() {
    if (!m_isInitialized) {
        throw std::runtime_error("NightAuth must be initialized before calling API methods. Call init() first.");
    }
}

void Api::setError(const std::string& msg, const std::string& err) {
    response = Response{};
    response.statusCode = m_impl->lastStatus;
    response.success = false;
    response.message = msg.empty() ? "Operation failed" : msg;
    response.error = err.empty() ? "error" : err;
}

void Api::setSuccess(const std::string& msg) {
    response = Response{};
    response.statusCode = m_impl->lastStatus;
    response.success = true;
    response.message = msg;
}

void Api::init() {
    if (!m_impl->transport) throw std::runtime_error("no transport configured — call setTransport() with a real HTTPS client before init()");

    if (name.empty() || api_key.empty()) {
        setError("Application credentials are not configured. Please specify your app credentials from the NightAuth Dashboard.", "unconfigured");
        return;
    }

    close();
    if (m_impl->clientEphemeral) { EVP_PKEY_free(m_impl->clientEphemeral); m_impl->clientEphemeral = nullptr; }
    Crypto::X25519KeyPair kp = Crypto::generateX25519();

    m_impl->clientEphemeral = kp.pkey;
    kp.pkey = nullptr;

    Crypto::Bytes clientPubRaw = Crypto::x25519PublicRaw(m_impl->clientEphemeral);
    std::string k1Hex = Crypto::toHex(clientPubRaw.data(), clientPubRaw.size());

    Crypto::Bytes idBytes(16);
    if (RAND_bytes(idBytes.data(), static_cast<int>(idBytes.size())) != 1) {
        OPENSSL_cleanse(idBytes.data(), idBytes.size());
        throw std::runtime_error("cryptographic random generator failed");
    }

    const std::string newSessionKey = Crypto::toHex(idBytes.data(), idBytes.size());

    std::ostringstream payload;
    payload << "{\"apikey\":\"" << jsonEscape(api_key) << "\",\"name\":\"" << jsonEscape(name)
            << "\",\"version\":\"" << jsonEscape(version) << "\",\"id\":\"" << jsonEscape(newSessionKey)
            << "\",\"k1\":\"" << k1Hex << "\"}";

    std::string sealedB64 = Crypto::sealInit(payload.str(), m_impl->sealPin);
    std::string body = "{\"p\":\"" + sealedB64 + "\"}";

    TransportResponse raw;
    m_impl->lastStatus = 0;
    try {
        raw = m_impl->transport("/init", "POST", body);
        m_impl->lastStatus = raw.status;
    } catch (const CryptographicException&) {
        throw;
    } catch (const std::exception& e) {
        m_impl->lastStatus = failureStatus(e);
        setError("Initialization failed: network request failed.", transportFailureCode(e));
        return;
    }

    verifyHandshakeResponse(raw);

    if (m_isInitialized) m_sessionKey = newSessionKey;
}

void Api::verifyHandshakeResponse(const TransportResponse& raw) {
    if (raw.status != 200 || !raw.success) {

        if (!isJsonObject(raw.body)) {
            setError("Initialization failed: the answer is not a JSON object (HTTP " + std::to_string(raw.status) + ").", "invalid_response");
            return;
        }
        setFailureFromBody(raw.body, ("Initialization failed: HTTP " + std::to_string(raw.status)).c_str(), "init_failed");
        return;
    }
    if (raw.k2.empty() || raw.vk.empty() || raw.sig.empty()) {
        throw CryptographicException("init response missing signature binding fields (k2/vk/ts/sig)");
    }

    if (raw.vk != m_impl->signingPin) {
        throw CryptographicException("init response signing key does not match pinned key — refusing handshake");
    }
    if (!Crypto::verifyInitSignature(raw.k2, m_impl->signingPin, raw.ts, raw.sig)) {
        throw CryptographicException("init response signature verification failed — refusing to trust this handshake");
    }
    int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
    if (std::llabs(nowMs - raw.ts) > 90 * 1000) {
        throw CryptographicException("init response timestamp is stale/future — possible replay");
    }

    Crypto::Bytes serverPubRaw = Crypto::fromHex(raw.k2);
    Crypto::Bytes secret = Crypto::x25519Dh(m_impl->clientEphemeral, serverPubRaw);
    Crypto::Bytes derivedKey = Crypto::hkdfSha256(secret, Crypto::infoBytes("nightauth-v1-e2e"), 32);
    OPENSSL_cleanse(secret.data(), secret.size());

    m_impl->zero();
    m_impl->sharedKey = derivedKey;
    std::memcpy(m_sharedKey, derivedKey.data(), 32);
    OPENSSL_cleanse(derivedKey.data(), derivedKey.size());
    m_isInitialized = true;
    setSuccess("Initialized successfully.");
}

static bool isHandshakeExpiredBody(const std::string& text) {
    try {
        Json::Value v = Json::parse(text);
        if (!v.isObject()) return false;
        if (const Json::Value* ok = v.find("success")) if (ok->isBool() && ok->asBool()) return false;
        std::string error, message;
        if (const Json::Value* f = v.find("error")) if (f->type == Json::Type::String) error = f->strVal;
        if (const Json::Value* f = v.find("message")) if (f->type == Json::Type::String) message = f->strVal;
        if (error == "handshake_required" || error == "session_invalid" || error == "unauthenticated") return true;
        for (char& c : message) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return message.find("handshake") != std::string::npos || message.find("initialize app") != std::string::npos;
    } catch (const Json::ParseError&) {
        return false;
    }
}

namespace {
inline std::string resolveTargetPath(const std::string& apiUrl, const std::string& path) {
    std::string basePath;
    const size_t schemePos = apiUrl.find("://");
    if (schemePos != std::string::npos) {
        const size_t slashPos = apiUrl.find('/', schemePos + 3);
        if (slashPos != std::string::npos) {
            basePath = apiUrl.substr(slashPos);
            while (!basePath.empty() && basePath.back() == '/') {
                basePath.pop_back();
            }
        }
    }
    if (!basePath.empty()) {
        if (path.rfind(basePath, 0) == 0 && (path.size() == basePath.size() || path[basePath.size()] == '/')) {
            return path;
        }
        return basePath + (path.empty() || path[0] == '/' ? path : "/" + path);
    }
    return path;
}
}

std::string Api::sendEncrypted(const std::string& path, const std::string& payloadJson, bool allowReinit) {

    if (allowReinit && !m_isInitialized) {
        init();
        if (!m_isInitialized) {
            return "{\"success\":false,\"error\":\"" + jsonEscape(response.error.empty() ? "init_failed" : response.error) +
                   "\",\"message\":\"" + jsonEscape(response.message) + "\"}";
        }
    }
    std::string text = sendEncryptedOnce(path, payloadJson);
    if (!allowReinit || !isHandshakeExpiredBody(text)) return text;

    try {
        init();
    } catch (const std::exception&) {
        return text;
    }
    if (!m_isInitialized) return text;
    return sendEncryptedOnce(path, payloadJson);
}

std::string Api::sendEncryptedOnce(const std::string& path, const std::string& payloadJson) {
    checkInit();
    if (!m_impl->transport) throw std::runtime_error("no transport configured");

    int64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch()).count();
    Crypto::Bytes ridBytes(8);
    if (RAND_bytes(ridBytes.data(), static_cast<int>(ridBytes.size())) != 1) {
        OPENSSL_cleanse(ridBytes.data(), ridBytes.size());
        throw std::runtime_error("cryptographic random generator failed");
    }
    std::string rid = "rid-" + std::to_string(ts) + "-" + Crypto::toHex(ridBytes.data(), ridBytes.size());

    std::string targetPath = resolveTargetPath(api_url, path);
    std::string aad = "POST:" + targetPath + ":" + m_sessionKey + ":" + rid + ":" + std::to_string(ts);
    Crypto::Bytes aadBytes(aad.begin(), aad.end());
    Crypto::Bytes plaintext(payloadJson.begin(), payloadJson.end());
    Crypto::AeadResult enc = Crypto::aesGcmEncrypt(m_impl->sharedKey, plaintext, aadBytes);

    std::ostringstream envelope;
    envelope << "{\"id\":\"" << jsonEscape(m_sessionKey) << "\",\"ct\":\"" << enc.ctB64
              << "\",\"n\":\"" << enc.nonceB64 << "\",\"rid\":\"" << rid << "\",\"ts\":" << ts << "}";

    std::string wire = envelope.str();
    if (m_pendingFile != nullptr) {

        std::string safeName;
        for (char ch : m_pendingFile->fileName) {
            if (ch == '"') safeName += "%22";
            else if (ch == '\r') safeName += "%0D";
            else if (ch == '\n') safeName += "%0A";
            else safeName += ch;
        }
        const std::string boundary = "NightAuthBoundary" + Crypto::toHex(reinterpret_cast<const uint8_t*>(rid.data()), rid.size() < 8 ? rid.size() : 8) + std::to_string(ts);
        std::string mp = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"envelope\"\r\n\r\n" + wire + "\r\n";
        mp += "--" + boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + safeName +
              "\"\r\nContent-Type: application/octet-stream\r\n\r\n";
        mp += m_pendingFile->fileData;
        mp += "\r\n--" + boundary + "--\r\n";
        wire = std::move(mp);
    }
    TransportResponse raw;
    m_impl->lastStatus = 0;
    try {
        raw = m_impl->transport(path, "POST", wire);
        m_impl->lastStatus = raw.status;
    } catch (const CryptographicException&) {
        throw;
    } catch (const std::exception& e) {
        m_impl->lastStatus = failureStatus(e);
        return failureJson(transportFailureCode(e), e.what());
    }
    if ((raw.ct.empty() || raw.n.empty()) && !isJsonObject(raw.body)) {

        return failureJson("invalid_response", "Response body is not a JSON object (HTTP " + std::to_string(raw.status) + ").");
    }
    if ((raw.ct.empty() || raw.n.empty()) && (raw.status < 200 || raw.status >= 300)) {

        std::string error, message;
        try {
            Json::Value v = Json::parse(raw.body);
            if (v.isObject()) {
                if (const Json::Value* f = v.find("error")) if (f->type == Json::Type::String) error = f->strVal;
                if (const Json::Value* f = v.find("message")) if (f->type == Json::Type::String) message = f->strVal;
            }
        } catch (const Json::ParseError&) {
        }
        if (error.empty()) error = raw.status == 428 ? "handshake_required" : "http_" + std::to_string(raw.status);
        if (raw.status == 428 || error == "handshake_required") close();
        return "{\"success\":false,\"error\":\"" + jsonEscape(error) + "\",\"message\":\"" + jsonEscape(message) + "\"}";
    }
    if (raw.ct.empty() || raw.n.empty()) {

        throw CryptographicException("response was not encrypted (plaintext success is never valid)");
    }
    if (raw.id != m_sessionKey || raw.rid != rid || raw.ts != ts) {
        throw CryptographicException("response id/rid/ts does not match the request - binding violation, discarding");
    }
    std::string respAad = "v1:res:" + raw.id + ":" + raw.rid + ":" + std::to_string(raw.ts);
    Crypto::Bytes respAadBytes(respAad.begin(), respAad.end());
    Crypto::Bytes decrypted;
    try {
        decrypted = Crypto::aesGcmDecrypt(m_impl->sharedKey, raw.ct, raw.n, respAadBytes);
    } catch (const std::exception& e) {
        throw CryptographicException(std::string("encrypted response does not decrypt: ") + e.what());
    }
    return std::string(decrypted.begin(), decrypted.end());
}

bool Api::logout() {
    bool confirmed = false;
    std::string code, message;
    try {
        std::string body = "{\"apikey\":\"" + jsonEscape(api_key) + "\",\"name\":\"" + jsonEscape(name) + "\"}";

        std::string respJson = sendEncrypted("/logout", body, true);
        try {
            Json::Value v = Json::parse(respJson);
            if (v.isObject()) {
                if (const Json::Value* f = v.find("error")) if (f->type == Json::Type::String) code = f->strVal;
                if (const Json::Value* f = v.find("message")) if (f->type == Json::Type::String) message = f->strVal;
            }
        } catch (const Json::ParseError&) {
        }

        confirmed = Json::hasRootSuccessTrue(respJson);
    } catch (const CryptographicException&) {
        throw;
    } catch (...) {
        confirmed = false;
    }
    user = UserData{};
    m_impl->zero();
    m_sessionKey.clear();
    std::memset(m_sharedKey, 0, sizeof(m_sharedKey));
    m_isInitialized = false;
    if (confirmed) {
        setSuccess("Logged out successfully.");
    } else if (code == "network_error" || code == "timeout" || code == "invalid_response" ||
               code == "response_too_large" || code == "handshake_required") {

        setError(message.empty() ? "Logout failed" : message, code);
    } else {
        setError("Local session cleared; server revocation unconfirmed.", "logout_unconfirmed");
    }
    return confirmed;
}

void Api::close() {
    m_impl->zero();
    std::memset(m_sharedKey, 0, sizeof(m_sharedKey));
    m_sessionKey.clear();
    user = UserData{};
    app = AppData{};
    response = Response{};
    m_isInitialized = false;
}

namespace {

const size_t kMaxEventBytes = 1024 * 1024;
const size_t kMaxFileBytes = 64 * 1024 * 1024;
const size_t kMaxRefusalBytes = 64 * 1024;

class RequestBody {
public:
    RequestBody& add(const char* key, const std::string& value) {
        append(key, Json::quoteString(value));
        return *this;
    }
    RequestBody& addRaw(const char* key, const std::string& json) {
        append(key, json);
        return *this;
    }
    std::string str() const { return "{" + m_body + "}"; }

private:
    void append(const char* key, const std::string& valueJson) {
        if (!m_body.empty()) m_body += ",";
        m_body += Json::quoteString(key) + ":" + valueJson;
    }
    std::string m_body;
};

RequestBody baseBody(const Api& api) {
    RequestBody b;
    b.add("apikey", api.api_key).add("name", api.name);
    if (!api.token.empty()) b.add("token", api.token);
    return b;
}

Json::Value parseObject(const std::string& text) {
    Json::Value v = Json::parse(text);
    if (!v.isObject()) throw std::runtime_error("protocol violation: decrypted response is not a JSON object");
    return v;
}

bool okFlag(const Json::Value& v) {
    const Json::Value* f = v.find("success");
    return f != nullptr && f->isBool() && f->asBool();
}

std::string strOf(const Json::Value& v, const char* key) {
    const Json::Value* f = v.find(key);
    return (f != nullptr && f->type == Json::Type::String) ? f->strVal : std::string();
}

bool boolOf(const Json::Value& v, const char* key) {
    const Json::Value* f = v.find(key);
    return f != nullptr && f->isBool() && f->asBool();
}

int64_t intOf(const Json::Value& v, const char* key) {
    const Json::Value* f = v.find(key);
    return (f != nullptr && f->type == Json::Type::Number) ? static_cast<int64_t>(f->numVal) : 0;
}

std::string textOf(const Json::Value* f) {
    if (f == nullptr) return "";
    if (f->type == Json::Type::String) return f->strVal;
    if (f->type == Json::Type::Number) return Json::serialize(*f);
    return "";
}

std::string formatTimestampOrText(const Json::Value& obj, const char* k1, const char* k2 = nullptr) {
    const Json::Value* f = obj.find(k1);
    if ((f == nullptr || f->type == Json::Type::Null) && k2 != nullptr) {
        f = obj.find(k2);
    }
    if (f == nullptr) return "";
    if (f->type == Json::Type::Number && f->numVal > 0) {
        std::time_t ts = static_cast<std::time_t>(static_cast<int64_t>(f->numVal));
        std::tm* tmInfo = std::gmtime(&ts);
        if (tmInfo != nullptr) {
            char buf[64] = {0};
            if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M UTC", tmInfo) > 0) {
                return std::string(buf);
            }
        }
        return Json::serialize(*f);
    }
    if (f->type == Json::Type::String && !f->strVal.empty()) {
        bool allDigits = f->strVal.size() >= 9;
        for (char c : f->strVal) {
            if (!std::isdigit(static_cast<unsigned char>(c))) { allDigits = false; break; }
        }
        if (allDigits) {
            std::time_t ts = static_cast<std::time_t>(std::atoll(f->strVal.c_str()));
            std::tm* tmInfo = std::gmtime(&ts);
            if (tmInfo != nullptr) {
                char buf[64] = {0};
                if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M UTC", tmInfo) > 0) {
                    return std::string(buf);
                }
            }
        }
        return f->strVal;
    }
    return "";
}

void populateSubscriptions(std::vector<SubscriptionData>& out, const Json::Value* arr) {
    out.clear();
    if (arr == nullptr || arr->type != Json::Type::Array) return;
    for (const auto& item : arr->arrVal) {
        if (!item.isObject()) continue;
        SubscriptionData s;
        s.subscription = strOf(item, "subscription");
        s.key = strOf(item, "key");
        s.expiry = formatTimestampOrText(item, "expiry", "expiresAt");
        s.time_left = intOf(item, "timeleft");
        out.push_back(s);
    }
}

void populateUser(UserData& u, const Json::Value* info) {
    if (info == nullptr || !info->isObject()) return;
    u.username = strOf(*info, "username");
    u.email = strOf(*info, "email");
    u.hwid = strOf(*info, "hwid");
    u.ip = strOf(*info, "ip");
    u.status = strOf(*info, "status");
    u.created_at = formatTimestampOrText(*info, "createdate", "createdAt");
    u.last_login_at = formatTimestampOrText(*info, "lastlogin", "lastLoginAt");
    populateSubscriptions(u.subscriptions, info->find("subscriptions"));
}

bool isHeartbeatSessionError(const std::string& e) {
    return e == "session_invalid" || e == "session_not_validated" || e == "unauthenticated";
}

bool isCheckSessionError(const std::string& e) {
    return e == "session_invalid" || e == "no_active_subscription" || e == "subscription_paused" || e == "unauthenticated";
}

}

Api::TransportResponse Api::sendPlain(const std::string& path, const std::string& method, const std::string& body) {
    if (!m_impl->transport) throw std::runtime_error("no transport configured — call setTransport() with a real HTTPS client first");
    m_impl->lastStatus = 0;
    try {
        TransportResponse r = m_impl->transport(path, method, body);
        m_impl->lastStatus = r.status;
        return r;
    } catch (const std::exception& e) {
        m_impl->lastStatus = failureStatus(e);
        throw;
    }
}

void Api::setFailureFromBody(const std::string& body, const char* fallbackMessage, const char* fallbackError) {
    std::string message = fallbackMessage;
    std::string error = fallbackError;
    try {
        Json::Value v = Json::parse(body);
        if (v.isObject()) {
            const std::string m = strOf(v, "message");
            const std::string e = strOf(v, "error");
            if (!m.empty()) message = m;
            if (!e.empty()) error = e;
            const std::string reqVer = strOf(v, "requiredVersion");
            const std::string dlUrl = strOf(v, "downloadUrl");
            if (!reqVer.empty()) requiredVersion = reqVer;
            if (!dlUrl.empty()) pendingDownloadUrl = dlUrl;
        }
    } catch (const Json::ParseError&) {

    }
    setError(message, error);
}

bool Api::login(const std::string& username, const std::string& password, const std::string& totpCode) {
    RequestBody b = baseBody(*this);
    b.add("version", version).add("username", username).add("password", password)
        .add("hwid", hardwareId());
    if (!totpCode.empty()) b.add("totpCode", totpCode);
    const std::string text = sendEncrypted("/login", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Login failed", "login_failed");
        return false;
    }
    populateUser(user, root.find("info"));
    setSuccess("Logged in successfully.");
    return true;
}

bool Api::registerAccount(const std::string& username, const std::string& password, const std::string& key, const std::string& email) {
    RequestBody b = baseBody(*this);
    b.add("version", version).add("username", username).add("password", password).add("key", key);
    if (!email.empty()) b.add("email", email);
    b.add("hwid", hardwareId());
    const std::string text = sendEncrypted("/register", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Registration failed", "registration_failed");
        return false;
    }
    populateUser(user, root.find("info"));
    setSuccess("Registered successfully.");
    return true;
}

bool Api::license(const std::string& key, const std::string& totpCode) {
    RequestBody b = baseBody(*this);
    b.add("version", version).add("key", key).add("hwid", hardwareId());
    if (!totpCode.empty()) b.add("totpCode", totpCode);
    const std::string text = sendEncrypted("/license", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "License activation failed", "license_failed");
        return false;
    }
    populateUser(user, root.find("info"));
    setSuccess("License verified successfully.");
    return true;
}

bool Api::upgrade(const std::string& username, const std::string& key) {
    RequestBody b = baseBody(*this);
    if (!username.empty()) b.add("username", username);
    b.add("key", key);
    const std::string text = sendEncrypted("/upgrade", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Upgrade failed", "upgrade_failed");
        return false;
    }
    setSuccess("Subscription upgraded successfully.");
    return true;
}

bool Api::heartbeat() {
    RequestBody b = baseBody(*this);
    b.add("version", version);
    const std::string text = sendEncrypted("/heartbeat", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Heartbeat rejected", "heartbeat_failed");
        if (isHeartbeatSessionError(response.error) && onSessionInvalidated) {
            onSessionInvalidated("Session terminated or invalid (Heartbeat failure)");
        }
        return false;
    }
    setSuccess("Session alive.");
    return true;
}

bool Api::check() {
    RequestBody b = baseBody(*this);
    b.add("version", version);
    const std::string text = sendEncrypted("/check", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Session invalid", "check_failed");
        if (isCheckSessionError(response.error) && onSessionInvalidated) {
            onSessionInvalidated("Session check rejected: " + response.error);
        }
        return false;
    }
    populateSubscriptions(user.subscriptions, root.find("subscriptions"));
    setSuccess("Session valid.");
    return true;
}

bool Api::changeUsername(const std::string& newUsername, const std::string& password) {
    RequestBody b = baseBody(*this);
    b.add("newUsername", newUsername).add("password", password);
    const std::string text = sendEncrypted("/change-username", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to change username", "change_username_failed");
        return false;
    }
    user.username = newUsername;
    setSuccess("Username changed successfully.");
    return true;
}

bool Api::resetHwid(const std::string& username, const std::string& password, const std::string& totpCode) {
    RequestBody b = baseBody(*this);
    b.add("username", username).add("password", password);
    if (!totpCode.empty()) b.add("totpCode", totpCode);
    const std::string text = sendEncrypted("/hwid-reset", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "HWID reset failed", "hwid_reset_failed");
        return false;
    }
    const std::string message = strOf(root, "message");
    setSuccess(message.empty() ? "HWID reset successfully." : message);
    return true;
}

bool Api::ban(const std::string& reason) {
    RequestBody b = baseBody(*this);
    if (!reason.empty()) b.add("reason", reason);
    const std::string text = sendEncrypted("/ban", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to self-report ban", "ban_failed");
        return false;
    }
    setSuccess("Account banned.");
    return true;
}

bool Api::checkBlacklist(const std::string& hwid) {
    RequestBody b = baseBody(*this);
    b.add("hwid", hwid.empty() ? hardwareId() : hwid);
    const std::string text = sendEncrypted("/check-blacklist", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Blacklist check failed", "check_blacklist_failed");
        return false;
    }
    setSuccess("Blacklist check complete.");
    return boolOf(root, "blacklisted");
}

bool Api::verifyFileHash(const std::string& hash) {
    RequestBody b = baseBody(*this);
    b.add("hash", hash);
    const std::string text = sendEncrypted("/verify-hash", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Hash check failed", "verify_hash_failed");
        return false;
    }
    setSuccess("Hash check complete.");
    return boolOf(root, "verified");
}

bool Api::forgotPassword(const std::string& email) {
    RequestBody b = baseBody(*this);
    b.add("email", email);
    const std::string text = sendEncrypted("/forgot-password", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to request a password reset", "forgot_password_failed");
        return false;
    }
    setSuccess("Password reset requested.");
    return true;
}

bool Api::resetPassword(const std::string& token, const std::string& newPassword) {

    RequestBody b;
    b.add("token", token).add("newPassword", newPassword);

    TransportResponse r;
    try {
        r = sendPlain("/reset-password", "POST", b.str());
    } catch (const CryptographicException&) {
        throw;
    } catch (const std::exception& e) {
        setError("Failed to reset password: network request failed.", transportFailureCode(e));
        return false;
    }
    Json::Value root;
    try {
        root = parseObject(r.body);
    } catch (const std::runtime_error&) {
        setError("Response body is not a JSON object (HTTP " + std::to_string(r.status) + ").", "invalid_response");
        return false;
    }
    if (r.status != 200 || !okFlag(root)) {
        setFailureFromBody(r.body, "Failed to reset password", "reset_password_failed");
        return false;
    }
    setSuccess("Password has been reset.");
    return true;
}

bool Api::setup2FA(TwoFactorSetupData& outData) {
    RequestBody b = baseBody(*this);
    const std::string text = sendEncrypted("/2fa/setup", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "2FA setup failed", "2fa_setup_failed");
        return false;
    }
    outData.secret = strOf(root, "secret");
    outData.uri = strOf(root, "uri");
    outData.qr_data_url = strOf(root, "qr");
    setSuccess("2FA setup started.");
    return true;
}

bool Api::confirm2FA(const std::string& code, TwoFactorConfirmData& outData) {
    RequestBody b = baseBody(*this);
    b.add("code", code);
    const std::string text = sendEncrypted("/2fa/confirm", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "2FA confirmation failed", "2fa_confirm_failed");
        return false;
    }
    outData.backup_codes.clear();
    const Json::Value* codes = root.find("backupCodes");
    if (codes != nullptr && codes->type == Json::Type::Array) {
        for (const auto& c : codes->arrVal) {
            if (c.type == Json::Type::String) outData.backup_codes.push_back(c.strVal);
        }
    }
    setSuccess("2FA enabled.");
    return true;
}

bool Api::disable2FA(const std::string& password, const std::string& code) {
    RequestBody b = baseBody(*this);
    b.add("password", password).add("code", code);
    const std::string text = sendEncrypted("/2fa/disable", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to disable 2FA", "2fa_disable_failed");
        return false;
    }
    setSuccess("2FA disabled.");
    return true;
}

std::string Api::getGlobalVar(const std::string& key) {
    RequestBody b = baseBody(*this);
    b.add("varName", key);
    const std::string text = sendEncrypted("/global-var", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Variable not found", "var_not_found");
        return "";
    }
    setSuccess("Variable loaded.");
    return strOf(root, "value");
}

std::string Api::getUserVar(const std::string& key) {
    RequestBody b = baseBody(*this);
    b.add("key", key);
    const std::string text = sendEncrypted("/user-vars/get", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "User variable not found", "user_var_not_found");
        return "";
    }
    setSuccess("User variable loaded.");
    const Json::Value* vars = root.find("variables");
    if (vars != nullptr && vars->type == Json::Type::Array) {
        for (const auto& item : vars->arrVal) {
            if (item.isObject() && strOf(item, "key") == key) return strOf(item, "value");
        }
    }
    return "";
}

bool Api::getUserVariables(std::vector<std::pair<std::string, std::string>>& out) {
    out.clear();
    RequestBody b = baseBody(*this);
    const std::string text = sendEncrypted("/user-vars/get", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "User variables not found", "user_vars_get_failed");
        return false;
    }
    const Json::Value* vars = root.find("variables");
    if (vars != nullptr && vars->type == Json::Type::Array) {
        for (const auto& item : vars->arrVal) {
            if (item.isObject()) out.emplace_back(strOf(item, "key"), strOf(item, "value"));
        }
    }
    setSuccess("User variables retrieved.");
    return true;
}

bool Api::setUserVar(const std::string& key, const std::string& value) {
    RequestBody b = baseBody(*this);
    b.add("key", key).add("value", value);
    const std::string text = sendEncrypted("/user-vars/set", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to set variable", "set_user_var_failed");
        return false;
    }
    setSuccess("User variable saved.");
    return true;
}

bool Api::log(const std::string& message) {
    RequestBody b = baseBody(*this);
    b.add("message", message);
    const std::string text = sendEncrypted("/log", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Log event rejected", "log_failed");
        return false;
    }
    setSuccess("Log event recorded.");
    return true;
}

bool Api::getStats(AppStats& outStats) {
    RequestBody b = baseBody(*this);
    const std::string text = sendEncrypted("/stats", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to retrieve stats", "stats_failed");
        return false;
    }
    outStats = AppStats{};
    outStats.version = strOf(root, "version");
    if (const Json::Value* users = root.find("users"); users != nullptr && users->isObject()) {
        outStats.total_users = intOf(*users, "total");
        outStats.active_users = intOf(*users, "active");
        outStats.banned_users = intOf(*users, "banned");
    }
    if (const Json::Value* licenses = root.find("licenses"); licenses != nullptr && licenses->isObject()) {
        outStats.total_licenses = intOf(*licenses, "total");
        outStats.used_licenses = intOf(*licenses, "used");
        outStats.unused_licenses = intOf(*licenses, "notUsed");
    }
    setSuccess("Stats loaded.");
    return true;
}

std::vector<std::string> Api::getOnlineUsers() {
    RequestBody b = baseBody(*this);
    const std::string text = sendEncrypted("/online-users", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to fetch online users", "online_users_failed");
        return {};
    }
    std::vector<std::string> names;
    const Json::Value* users = root.find("users");
    if (users != nullptr && users->type == Json::Type::Array) {
        for (const auto& u : users->arrVal) {
            if (u.type == Json::Type::String) names.push_back(u.strVal);
        }
    }
    setSuccess("Online users loaded.");
    return names;
}

std::vector<ChatMessage> Api::getChatMessages(const std::string& channel, int64_t before, bool markRead) {
    RequestBody b = baseBody(*this);
    b.add("room", channel);
    if (before > 0) b.addRaw("before", std::to_string(before));
    if (markRead) b.addRaw("markRead", "true");
    const std::string text = sendEncrypted("/chat/get", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to fetch messages", "chat_fetch_failed");
        return {};
    }
    std::vector<ChatMessage> list;
    const Json::Value* messages = root.find("messages");
    if (messages != nullptr && messages->type == Json::Type::Array) {
        for (const auto& item : messages->arrVal) {
            if (!item.isObject()) continue;
            ChatMessage m;
            m.id = textOf(item.find("id"));
            m.author = strOf(item, "author");
            m.message = strOf(item, "content");
            m.timestamp = strOf(item, "createdAt");
            m.status = strOf(item, "status");
            list.push_back(m);
        }
    }
    setSuccess("Messages loaded.");
    return list;
}

bool Api::sendChatMessage(const std::string& message, const std::string& channel) {
    RequestBody b = baseBody(*this);
    b.add("room", channel).add("content", message);
    const std::string text = sendEncrypted("/chat/send", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Failed to send chat message", "chat_send_failed");
        return false;
    }
    setSuccess("Message sent.");
    return true;
}

bool Api::requestAccessTicket(const std::string& scope, const std::string& resourceId, AccessTicket& outTicket) {
    RequestBody b = baseBody(*this);
    b.add("scope", scope);
    if (!resourceId.empty()) b.add("resourceId", resourceId);
    const std::string text = sendEncrypted("/access-ticket", b.str());
    Json::Value root = parseObject(text);
    if (!okFlag(root)) {
        setFailureFromBody(text, "Access ticket denied", "ticket_denied");
        return false;
    }
    const std::string ticket = strOf(root, "ticket");
    if (ticket.empty()) {
        setError("Access ticket missing from response", "ticket_missing");
        return false;
    }
    outTicket.ticket = ticket;
    outTicket.expires_in = intOf(root, "expiresIn");
    setSuccess("Access ticket issued.");
    return true;
}

bool Api::downloadFile(const std::string& fileId, std::string& outContent) {
    AccessTicket ticket;
    if (!requestAccessTicket("file", fileId, ticket)) return false;

    TransportResponse r = sendPlain("/files/download?ticket=" + ticket.ticket, "GET", "");
    if (r.status >= 300 && r.status < 400) {

        setError("File download redirected to an external location; it was not followed.", "external_download");
        response.url = r.location;
        return false;
    }
    Json::Value root;
    try {
        root = parseObject(r.body);
    } catch (const std::runtime_error&) {
        setError("File download failed (HTTP " + std::to_string(r.status) + ")", "download_http_error");
        return false;
    }
    if (r.status != 200 || !okFlag(root)) {
        setFailureFromBody(r.body, ("File download failed (HTTP " + std::to_string(r.status) + ")").c_str(), "download_http_error");
        return false;
    }
    const std::string data = strOf(root, "data");
    if (data.empty()) {
        setError("File response is missing content", "download_missing_field");
        return false;
    }
    const Crypto::Bytes raw = Crypto::base64Decode(data);
    std::string content;
    if (!Gzip::gunzip(std::string(raw.begin(), raw.end()), kMaxFileBytes, content)) {
        setError("File content could not be decompressed", "download_decode_failed");
        return false;
    }
    outContent = std::move(content);
    setSuccess("File downloaded.");
    return true;
}

bool Api::triggerWebhook(const std::string& webId, const std::string& payloadJson, WebhookResult& outResult) {

    Json::Value payload;
    try {
        payload = Json::parse(payloadJson);
    } catch (const Json::ParseError& e) {
        throw std::invalid_argument(std::string("payloadJson is not valid JSON: ") + e.what());
    }
    if (!payload.isObject()) throw std::invalid_argument("payloadJson must be a JSON object");
    return triggerWebhook(webId, payloadJson, outResult, WebhookOptions{});
}

bool Api::triggerWebhook(const std::string& webId, const std::string& payloadJson, WebhookResult& outResult,
                         const WebhookOptions& options) {
    RequestBody b = baseBody(*this);
    b.add("webId", webId);
    if (options.hasContent) b.add("content", options.content);
    if (options.hasParams) b.add("params", options.params);
    if (!payloadJson.empty()) {

        Json::Value payload;
        try {
            payload = Json::parse(payloadJson);
        } catch (const Json::ParseError& e) {
            throw std::invalid_argument(std::string("payloadJson is not valid JSON: ") + e.what());
        }
        b.addRaw("payload", Json::serialize(payload));
    }
    struct PendingFile {
        const WebhookOptions*& slot;
        PendingFile(const WebhookOptions*& s, const WebhookOptions* v) : slot(s) { slot = v; }
        ~PendingFile() { slot = nullptr; }
    } pending(m_pendingFile, options.hasFile ? &options : nullptr);
    const std::string text = sendEncrypted("/webhook", b.str());
    Json::Value root = parseObject(text);

    outResult = WebhookResult{};
    outResult.status = intOf(root, "status");
    outResult.truncated = boolOf(root, "truncated");
    if (const Json::Value* d = root.find("data"); d != nullptr) {
        outResult.data = d->type == Json::Type::String ? d->strVal : Json::serialize(*d);
    }
    if (!okFlag(root)) {
        setFailureFromBody(text, "Webhook returned a non-success status", "webhook_failed");
        return false;
    }
    setSuccess("Webhook delivered.");
    return true;
}

bool Api::subscribeEvents(const EventCallback& onEvent, const std::atomic<bool>* cancel, const std::string& lastEventId) {

    if (!m_streamTransport) {
        throw std::runtime_error("no stream transport configured — call setStreamTransport() with a real HTTPS client before subscribeEvents()");
    }
    if (!lastEventId.empty() && (lastEventId.size() > 20 || lastEventId.find_first_not_of("0123456789") != std::string::npos)) {
        throw std::invalid_argument("lastEventId must be a decimal event id");
    }
    if (cancel != nullptr && cancel->load()) return true;

    AccessTicket ticket;
    if (!requestAccessTicket("events", "", ticket)) return false;

    struct State {
        std::string buffer;
        std::vector<std::string> dataLines;
        size_t dataBytes = 0;
        std::string lastId;
        bool stopped = false;
        bool failed = false;
        std::string failMessage;
        std::string failError;
        std::string raw;
    } st;
    st.lastId = lastEventId;
    const std::string sessionKey = m_sessionKey;

    auto fail = [&st](const char* message, const char* error) {
        st.failed = true;
        st.stopped = true;
        st.failMessage = message;
        st.failError = error;
    };
    auto cancelled = [&st, cancel]() { return st.stopped || (cancel != nullptr && cancel->load()); };

    auto dispatch = [&](const std::string& raw) {
        if (cancel != nullptr && cancel->load()) {
            st.stopped = true;
            return;
        }
        Json::Value v;
        try {
            v = Json::parse(raw);
        } catch (const Json::ParseError&) {
            fail("Event frame is not valid JSON", "event_malformed");
            return;
        }
        if (!v.isObject()) {
            fail("Event frame is not a JSON object", "event_malformed");
            return;
        }
        const std::string type = strOf(v, "type");
        EventMessage message;
        message.id = st.lastId;
        message.type = type;
        message.data = raw;

        if (onEvent && !onEvent(message)) {
            st.stopped = true;
            return;
        }

        if (type == "session-invalid" && strOf(v, "sessionKey") != sessionKey) return;
        if (type == "chat-message" && onChatMessageReceived) {
            ChatMessage chat;
            chat.id = textOf(v.find("id"));
            chat.author = strOf(v, "author");
            chat.message = strOf(v, "content");
            chat.timestamp = strOf(v, "createdAt");
            onChatMessageReceived(chat, strOf(v, "room"));
        }
        if (type == "session-invalid") {
            if (onSessionInvalidated) onSessionInvalidated("Session invalidated by server.");
            fail("Session invalidated by server.", "session_invalid");
        } else if (type == "app-banned") {
            if (onAppBanned) onAppBanned(strOf(v, "reason"));
            fail("Application was banned.", "app_banned");
        } else if (type == "app-paused" || type == "app-disabled") {
            if (onAppStatusChanged) onAppStatusChanged(type);
            fail("Application status changed.", type == "app-paused" ? "app_paused" : "app_disabled");
        } else if (type == "version") {
            const std::string ver = strOf(v, "version");
            const std::string dl = strOf(v, "downloadUrl");
            if (!ver.empty()) requiredVersion = ver;
            if (!dl.empty()) pendingDownloadUrl = dl;
            if (onVersionUpdated) onVersionUpdated(ver);
        } else if (type == "server-shutdown") {
            st.stopped = true;
        }
    };

    auto processLine = [&](std::string line) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) {
            if (st.dataLines.empty()) return;
            std::string raw;
            for (size_t i = 0; i < st.dataLines.size(); ++i) {
                if (i != 0) raw += '\n';
                raw += st.dataLines[i];
            }
            st.dataLines.clear();
            st.dataBytes = 0;
            dispatch(raw);
            return;
        }
        if (line[0] == ':') return;
        const size_t colon = line.find(':');
        const std::string field = colon == std::string::npos ? line : line.substr(0, colon);
        std::string value = colon == std::string::npos ? std::string() : line.substr(colon + 1);
        if (!value.empty() && value[0] == ' ') value.erase(0, 1);
        if (field == "data") {
            st.dataBytes += value.size() + 1;
            if (st.dataBytes > kMaxEventBytes) {
                fail("Event frame exceeds the size limit", "event_stream_too_large");
                return;
            }
            st.dataLines.push_back(value);
        } else if (field == "id") {
            if (value.find('\0') == std::string::npos) st.lastId = value;
        }
    };

    StreamHandler onBytes = [&](const char* data, size_t len) -> bool {
        if (st.stopped) return false;
        if (st.raw.size() < kMaxRefusalBytes) st.raw.append(data, std::min(len, kMaxRefusalBytes - st.raw.size()));
        st.buffer.append(data, len);
        size_t pos = 0;
        size_t nl = 0;
        while (!st.stopped && (nl = st.buffer.find('\n', pos)) != std::string::npos) {
            processLine(st.buffer.substr(pos, nl - pos));
            pos = nl + 1;
        }
        st.buffer.erase(0, pos);
        if (!st.stopped && st.buffer.size() > kMaxEventBytes) {
            fail("Event stream line exceeds the size limit", "event_stream_too_large");
        }
        return !st.stopped;
    };

    m_impl->lastStatus = 0;
    const int status = m_streamTransport("/events?ticket=" + ticket.ticket, lastEventId, onBytes, cancelled);
    m_impl->lastStatus = status;
    if (status >= 300 && status < 400) {
        setError("Event stream redirected; redirects are refused.", "events_redirect_refused");
        return false;
    }
    if (status != 200) {

        setFailureFromBody(st.raw, ("Event stream rejected: HTTP " + std::to_string(status)).c_str(), "events_rejected");
        return false;
    }
    if (st.failed) {
        setError(st.failMessage, st.failError);
        return false;
    }
    setSuccess("Event stream ended.");
    return true;
}

void Api::setStreamTransport(StreamTransport t) { m_streamTransport = std::move(t); }

static std::string& hardwareIdOverride() {
    static std::string value;
    return value;
}

void SecurityProtections::setHardwareIdOverride(const std::string& hwid) { hardwareIdOverride() = hwid; }

std::string SecurityProtections::getHardwareId() {
    if (!hardwareIdOverride().empty()) return hardwareIdOverride();

    auto env = [](const char* a, const char* b, const char* fallback) {
        const char* v = std::getenv(a);
        if ((!v || !*v) && b) v = std::getenv(b);
        return std::string((v && *v) ? v : fallback);
    };
    std::string hostname = env("COMPUTERNAME", "HOSTNAME", "unknown_host");
    for (char& c : hostname) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    }
    std::string username = env("USERNAME", "USER", "unknown_user");
    for (char& c : username) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    const std::string raw = hostname + "-" + username + "-nightauth-hwid";
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    if (EVP_Digest(raw.data(), raw.size(), hash, &len, EVP_sha256(), nullptr) != 1) return "default_hwid_hash";
    return Crypto::toHex(hash, len);
}

void SecurityProtections::runSecurityChecks() {}
void SecurityProtections::checkSystemProxy() {}

#ifdef NIGHTAUTH_TEST_HOOKS
namespace TestHooks {
std::string hkdfSha256Hex(const std::string& ikmHex, const std::string& info, size_t outLen) {
    Crypto::Bytes ikm = Crypto::fromHex(ikmHex);
    Crypto::Bytes infoBytes(info.begin(), info.end());
    Crypto::Bytes out = Crypto::hkdfSha256(ikm, infoBytes, outLen);
    return Crypto::toHex(out.data(), out.size());
}
}
#endif

#ifndef NIGHTAUTH_NO_CURL
namespace {

struct CurlBuffer {
    std::string data;
    size_t cap;
};

static size_t curlWriteCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    CurlBuffer* buf = static_cast<CurlBuffer*>(userdata);
    size_t n = size * nmemb;
    if (buf->data.size() + n > buf->cap) {

        size_t room = buf->cap > buf->data.size() ? (buf->cap - buf->data.size()) : 0;
        buf->data.append(ptr, room);
        return 0;
    }
    buf->data.append(ptr, n);
    return n;
}

static bool equalsAsciiCaseInsensitive(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (std::tolower(static_cast<unsigned char>(*a)) !=
            std::tolower(static_cast<unsigned char>(*b))) return false;
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static bool parseBaseUrl(const std::string& baseUrl, std::string& schemeOut,
                         bool& loopbackOut) {
    CURLU* parsed = curl_url();
    if (!parsed) return false;
    bool valid = curl_url_set(parsed, CURLUPART_URL, baseUrl.c_str(), 0) == CURLUE_OK;
    char *scheme = nullptr, *host = nullptr, *user = nullptr, *password = nullptr;
    char *path = nullptr, *query = nullptr, *fragment = nullptr;
    if (valid) valid = curl_url_get(parsed, CURLUPART_SCHEME, &scheme, 0) == CURLUE_OK;
    if (valid) valid = curl_url_get(parsed, CURLUPART_HOST, &host, 0) == CURLUE_OK;
    if (valid && curl_url_get(parsed, CURLUPART_USER, &user, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_PASSWORD, &password, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_QUERY, &query, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_FRAGMENT, &fragment, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_PATH, &path, 0) == CURLUE_OK &&
        path && path[0] && std::string(path) != "/" && std::string(path) != "/api/v1") valid = false;

    if (valid) {
        if (equalsAsciiCaseInsensitive(scheme, "https")) schemeOut = "https";
        else if (equalsAsciiCaseInsensitive(scheme, "http")) schemeOut = "http";
        else valid = false;

        loopbackOut = equalsAsciiCaseInsensitive(host, "localhost") ||
                      equalsAsciiCaseInsensitive(host, "127.0.0.1") ||
                      equalsAsciiCaseInsensitive(host, "::1") ||
                      equalsAsciiCaseInsensitive(host, "[::1]");
    }

    curl_free(scheme);
    curl_free(host);
    curl_free(user);
    curl_free(password);
    curl_free(path);
    curl_free(query);
    curl_free(fragment);
    curl_url_cleanup(parsed);
    return valid;
}

}

namespace {

void requireRequestPath(const std::string& path, const std::string& method) {
    if (path.empty() || path[0] != '/') {
        throw std::invalid_argument("API request path must be an absolute path");
    }
    for (char c : path) {
        if (static_cast<unsigned char>(c) <= 0x20 || c == 0x7f || c == '#') {
            throw std::invalid_argument("API request path contains a fragment, space or control character");
        }
    }
    const size_t q = path.find('?');
    if (q == std::string::npos) return;
    if (method != "GET") throw std::invalid_argument("only GET requests may carry a query string");
    for (size_t i = q + 1; i < path.size(); ++i) {
        const char c = path[i];
        const bool safe = std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' ||
                          c == '~' || c == '=' || c == '&' || c == '%';
        if (!safe) throw std::invalid_argument("query string contains a character that must be escaped");
    }
}

}

Api::Transport Api::makeCurlTransport(const std::string& baseUrl, bool allowInsecureLoopback,
                                       long connectTimeoutMs, long totalTimeoutMs, size_t maxResponseBytes) {
    std::string scheme;
    bool loopback = false;
    if (!parseBaseUrl(baseUrl, scheme, loopback))
        throw std::invalid_argument("invalid API base URL (expected an origin without credentials, query, fragment, or path)");
    if (scheme == "http" && !(allowInsecureLoopback && loopback))
        throw std::invalid_argument("plain HTTP is allowed only for exact loopback hosts with explicit opt-in");
    if (connectTimeoutMs <= 0 || totalTimeoutMs <= 0 || maxResponseBytes == 0)
        throw std::invalid_argument("transport timeouts and response limit must be positive");

    std::string base = baseUrl;
    if (!base.empty() && base.back() == '/') base.pop_back();

    return [base, scheme, connectTimeoutMs, totalTimeoutMs, maxResponseBytes]
           (const std::string& path, const std::string& method, const std::string& body) -> Api::TransportResponse {
        requireRequestPath(path, method);
        TransportResponse out;

        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        struct Guard { CURL* c; ~Guard() { curl_easy_cleanup(c); } } guard{curl};

        std::string url = base + path;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, scheme.c_str());

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connectTimeoutMs);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, totalTimeoutMs);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

        struct curl_slist* headers = nullptr;

        std::string contentTypeHeader = "Content-Type: application/json";
        if (body.compare(0, 19, "--NightAuthBoundary") == 0) {
            const size_t eol = body.find("\r\n");
            if (eol != std::string::npos) contentTypeHeader = "Content-Type: multipart/form-data; boundary=" + body.substr(2, eol - 2);
        }
        headers = curl_slist_append(headers, contentTypeHeader.c_str());
        struct HeaderGuard { curl_slist* h; ~HeaderGuard() { curl_slist_free_all(h); } } hguard{headers};
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
        } else {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        }

        CurlBuffer respBuf{"", maxResponseBytes};
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respBuf);

        CURLcode rc = curl_easy_perform(curl);
        if (rc != CURLE_OK) {
            if (rc == CURLE_WRITE_ERROR && respBuf.data.size() >= maxResponseBytes) {
                long tooLargeStatus = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &tooLargeStatus);
                throw ResponseTooLargeError("response exceeded maximum allowed size (" + std::to_string(maxResponseBytes) + " bytes) — aborted",
                                            static_cast<int>(tooLargeStatus));
            }
            throw std::runtime_error(std::string("HTTP transport error: ") + curl_easy_strerror(rc));
        }

        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        out.status = static_cast<int>(status);
        if (status >= 300 && status < 400) {
            char* loc = nullptr;
            if (curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &loc) == CURLE_OK && loc) out.location = loc;
        }

        fillFromJson(out, respBuf.data);
        return out;
    };
}
namespace {

struct CurlStreamCtx {
    CURL* curl;
    const Api::StreamHandler* onBytes;
    const std::function<bool()>* isCancelled;
    bool aborted;
    size_t refusalBytes;
};

size_t curlStreamWriteCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    CurlStreamCtx* ctx = static_cast<CurlStreamCtx*>(userdata);
    const size_t n = size * nmemb;
    long code = 0;
    curl_easy_getinfo(ctx->curl, CURLINFO_RESPONSE_CODE, &code);
    if (code != 200) {

        ctx->refusalBytes += n;
        if (ctx->refusalBytes > kMaxRefusalBytes) {
            ctx->aborted = true;
            return 0;
        }
        (*ctx->onBytes)(ptr, n);
        return n;
    }
    if ((*ctx->isCancelled)() || !(*ctx->onBytes)(ptr, n)) {
        ctx->aborted = true;
        return 0;
    }
    return n;
}

int curlStreamProgressCb(void* clientp, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    CurlStreamCtx* ctx = static_cast<CurlStreamCtx*>(clientp);
    return (*ctx->isCancelled)() ? 1 : 0;
}

}

Api::StreamTransport Api::makeCurlStreamTransport(const std::string& baseUrl, bool allowInsecureLoopback, long connectTimeoutMs) {
    std::string scheme;
    bool loopback = false;
    if (!parseBaseUrl(baseUrl, scheme, loopback)) {
        throw std::invalid_argument("invalid API base URL (expected an origin without credentials, query, fragment, or path)");
    }
    if (scheme == "http" && !(allowInsecureLoopback && loopback)) {
        throw std::invalid_argument("plain HTTP is allowed only for exact loopback hosts with explicit opt-in");
    }
    if (connectTimeoutMs <= 0) throw std::invalid_argument("connect timeout must be positive");

    std::string base = baseUrl;
    if (!base.empty() && base.back() == '/') base.pop_back();

    return [base, scheme, connectTimeoutMs](const std::string& pathAndQuery, const std::string& lastEventId,
                                            const StreamHandler& onBytes, const std::function<bool()>& isCancelled) -> int {
        requireRequestPath(pathAndQuery, "GET");
        if (!lastEventId.empty() && (lastEventId.size() > 20 || lastEventId.find_first_not_of("0123456789") != std::string::npos)) {
            throw std::invalid_argument("lastEventId must be a decimal event id");
        }

        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        struct Guard { CURL* c; ~Guard() { curl_easy_cleanup(c); } } guard{curl};

        CurlStreamCtx ctx{curl, &onBytes, &isCancelled, false, 0};
        const std::string url = base + pathAndQuery;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, scheme.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connectTimeoutMs);

        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 120L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Accept: text/event-stream");
        headers = curl_slist_append(headers, "Cache-Control: no-cache");
        if (!lastEventId.empty()) headers = curl_slist_append(headers, ("Last-Event-ID: " + lastEventId).c_str());
        struct HeaderGuard { curl_slist* h; ~HeaderGuard() { curl_slist_free_all(h); } } hguard{headers};
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlStreamWriteCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curlStreamProgressCb);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);

        const CURLcode rc = curl_easy_perform(curl);
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (rc != CURLE_OK && !(ctx.aborted && (rc == CURLE_WRITE_ERROR || rc == CURLE_ABORTED_BY_CALLBACK))) {
            throw std::runtime_error(std::string("HTTP stream error: ") + curl_easy_strerror(rc));
        }
        return static_cast<int>(status);
    };
}

bool Api::applyUpdate(const std::string& /*downloadUrl*/) {
    setError("Unsigned auto-update is disabled. Verify a signed release manifest with SignedUpdateInstaller.", "unsigned_update_disabled");
    return false;
}

std::string Api::downloadSignedUpdate(const std::string& manifestUrl,
                                      const std::string& pinnedReleaseKeyHex,
                                      const std::string& destination) {
    return SignedUpdateInstaller::download(manifestUrl, pinnedReleaseKeyHex, name, destination);
}

VerifiedUpdate SignedUpdate::verify(const std::string& envelopeJson,
                                    const std::string& pinnedReleaseKeyHex,
                                    const std::string& expectedApplication,
                                    uint64_t minimumSequence) {
    if (envelopeJson.size() > 65536) {
        throw std::invalid_argument("Release envelope exceeds size limit.");
    }
    Json::Value env = Json::parse(envelopeJson);
    if (!env.isObject()) {
        throw std::invalid_argument("Release envelope is not a JSON object.");
    }
    std::string payloadB64 = strOf(env, "payload");
    std::string sigB64 = strOf(env, "signature");
    if (payloadB64.empty() || sigB64.empty()) {
        throw std::invalid_argument("Release envelope missing payload or signature.");
    }
    if (pinnedReleaseKeyHex.size() != 64) {
        throw std::invalid_argument("Pinned release key must be 64 hex characters (32 bytes).");
    }
    Crypto::Bytes keyBytes;
    try {
        keyBytes = Crypto::fromHex(pinnedReleaseKeyHex);
    } catch (...) {
        throw std::invalid_argument("Invalid pinned release key hex.");
    }
    Crypto::Bytes payloadBytes = Crypto::base64Decode(payloadB64);
    Crypto::Bytes sigBytes = Crypto::base64Decode(sigB64);
    if (payloadBytes.empty() || payloadBytes.size() > 32768 || sigBytes.size() != 64) {
        throw std::invalid_argument("Invalid manifest payload size or signature length.");
    }
    if (!Crypto::ed25519Verify(keyBytes, payloadBytes, sigBytes)) {
        throw CryptographicException("Release signature rejected.");
    }

    std::string payloadStr(payloadBytes.begin(), payloadBytes.end());
    Json::Value data = Json::parse(payloadStr);
    if (!data.isObject()) {
        throw std::invalid_argument("Manifest payload is not a JSON object.");
    }

    int64_t schema = intOf(data, "schema");
    std::string app = strOf(data, "application");
    std::string ver = strOf(data, "version");
    int64_t seq = intOf(data, "sequence");
    int64_t size = intOf(data, "size");
    int64_t expiresAt = intOf(data, "expiresAt");
    std::string urlStr = strOf(data, "url");
    std::string sha256Hex = strOf(data, "sha256");

    if (schema != 1 || app != expectedApplication || ver.empty() || ver.size() > 100) {
        throw std::invalid_argument("Release identity or schema rejected.");
    }
    if (seq <= static_cast<int64_t>(minimumSequence) || seq > 9007199254740991LL) {
        throw std::invalid_argument("Release sequence rejected.");
    }
    if (size < 1 || size > 256 * 1024 * 1024) {
        throw std::invalid_argument("Release artifact size rejected.");
    }
    int64_t nowSec = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (expiresAt <= nowSec) {
        throw std::invalid_argument("Release expired.");
    }
    if (urlStr.rfind("https://", 0) != 0 || urlStr.find('#') != std::string::npos || urlStr.find('@') != std::string::npos) {
        throw std::invalid_argument("Release URL must be HTTPS without user info or fragment.");
    }
    if (sha256Hex.size() != 64) {
        throw std::invalid_argument("Invalid sha256 hex length.");
    }
    Crypto::Bytes digest;
    try {
        digest = Crypto::fromHex(sha256Hex);
    } catch (...) {
        throw std::invalid_argument("Invalid sha256 hex characters.");
    }

    VerifiedUpdate vu;
    vu.application = app;
    vu.version = ver;
    vu.sequence = static_cast<uint64_t>(seq);
    vu.url = urlStr;
    vu.size = static_cast<uint64_t>(size);
    vu.expiresAt = expiresAt;
    vu.digest = digest;
    return vu;
}

namespace {
struct UpdateDownloadContext {
    FILE* fp{nullptr};
    EVP_MD_CTX* mdctx{nullptr};
    uint64_t totalBytes{0};
    uint64_t maxSize{0};
    bool error{false};
};

static size_t updateDownloadWriteCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<UpdateDownloadContext*>(userdata);
    size_t bytes = size * nmemb;
    if (ctx->totalBytes + bytes > ctx->maxSize) {
        ctx->error = true;
        return 0;
    }
    ctx->totalBytes += bytes;
    if (EVP_DigestUpdate(ctx->mdctx, ptr, bytes) != 1) {
        ctx->error = true;
        return 0;
    }
    size_t written = std::fwrite(ptr, 1, bytes, ctx->fp);
    if (written != bytes) {
        ctx->error = true;
        return 0;
    }
    return bytes;
}

static void ensureDirectoriesForFile(const std::string& filePath) {
    size_t lastSep = filePath.find_last_of("/\\");
    if (lastSep != std::string::npos) {
        std::string dir = filePath.substr(0, lastSep);
#ifdef _WIN32
        for (size_t i = 0; i < dir.size(); ++i) {
            if (dir[i] == '/' || dir[i] == '\\') {
                std::string sub = dir.substr(0, i);
                if (!sub.empty() && sub.back() != ':') {
                    CreateDirectoryA(sub.c_str(), NULL);
                }
            }
        }
        CreateDirectoryA(dir.c_str(), NULL);
#else
        for (size_t i = 1; i < dir.size(); ++i) {
            if (dir[i] == '/') {
                std::string sub = dir.substr(0, i);
                mkdir(sub.c_str(), 0700);
            }
        }
        mkdir(dir.c_str(), 0700);
#endif
    }
}

struct ManifestBuffer {
    std::string data;
    size_t maxSize{65536};
    bool overflow{false};
};

static size_t manifestWriteCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* mb = static_cast<ManifestBuffer*>(userdata);
    size_t bytes = size * nmemb;
    if (mb->data.size() + bytes > mb->maxSize) {
        mb->overflow = true;
        return 0;
    }
    mb->data.append(ptr, bytes);
    return bytes;
}
}

void SignedUpdate::download(const VerifiedUpdate& update, const std::string& destination) {
    if (destination.empty()) throw std::invalid_argument("Destination path cannot be empty.");
    int64_t nowSec = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (update.expiresAt <= nowSec) throw std::invalid_argument("Release expired.");

    FILE* testFp = std::fopen(destination.c_str(), "rb");
    if (testFp) {
        std::fclose(testFp);
        throw std::invalid_argument("Destination file already exists.");
    }

    std::string tempPath = destination + "." + std::to_string(nowSec) + ".partial";
    FILE* fp = std::fopen(tempPath.c_str(), "wb");
    if (!fp) {
        throw std::runtime_error("Failed to create temporary download file: " + tempPath);
    }

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx || EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
        std::fclose(fp);
        std::remove(tempPath.c_str());
        if (mdctx) EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize SHA-256 context.");
    }

    UpdateDownloadContext dc;
    dc.fp = fp;
    dc.mdctx = mdctx;
    dc.totalBytes = 0;
    dc.maxSize = update.size;
    dc.error = false;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::fclose(fp);
        std::remove(tempPath.c_str());
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize libcurl.");
    }

    curl_easy_setopt(curl, CURLOPT_URL, update.url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, updateDownloadWriteCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dc);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");

    struct curl_slist* headers = curl_slist_append(nullptr, "User-Agent: NightAuth-Updater/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    std::fclose(fp);

    if (res != CURLE_OK || httpCode != 200 || dc.error) {
        std::remove(tempPath.c_str());
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Artifact download failed: HTTP " + std::to_string(httpCode) + " (" + curl_easy_strerror(res) + ")");
    }

    if (dc.totalBytes != update.size) {
        std::remove(tempPath.c_str());
        EVP_MD_CTX_free(mdctx);
        throw std::invalid_argument("Artifact size differs from signed manifest.");
    }

    uint8_t computed[32];
    unsigned int compLen = 0;
    EVP_DigestFinal_ex(mdctx, computed, &compLen);
    EVP_MD_CTX_free(mdctx);

    if (compLen != 32 || CRYPTO_memcmp(computed, update.digest.data(), 32) != 0) {
        std::remove(tempPath.c_str());
        throw CryptographicException("Artifact hash rejected.");
    }

    nowSec = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (update.expiresAt <= nowSec) {
        std::remove(tempPath.c_str());
        throw std::invalid_argument("Release expired during download.");
    }

    if (std::rename(tempPath.c_str(), destination.c_str()) != 0) {
        std::remove(tempPath.c_str());
        throw std::runtime_error("Failed to rename temporary download file to destination.");
    }
}

std::string SignedUpdateInstaller::getSequencePath(const std::string& application) {
    if (application.empty() || application.size() > 100) {
        throw std::invalid_argument("Invalid application identifier.");
    }
    std::string root;
#ifdef _WIN32
    const char* envVal = std::getenv("LOCALAPPDATA");
    root = (envVal && envVal[0]) ? envVal : "C:\\NightAuth";
    std::string dir = root + "\\NightAuth\\release-sequences";
#else
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if (xdg && xdg[0]) {
        root = xdg;
    } else {
        const char* home = std::getenv("HOME");
        root = (home && home[0]) ? (std::string(home) + "/.local/share") : "/tmp";
    }
    std::string dir = root + "/NightAuth/release-sequences";
#endif

    unsigned char appHash[32];
    unsigned int appHashLen = 0;
    EVP_MD_CTX* md = EVP_MD_CTX_new();
    if (md) {
        EVP_DigestInit_ex(md, EVP_sha256(), nullptr);
        EVP_DigestUpdate(md, application.data(), application.size());
        EVP_DigestFinal_ex(md, appHash, &appHashLen);
        EVP_MD_CTX_free(md);
    }
    std::string appHashHex = Crypto::toHex(appHash, 32);

#ifdef _WIN32
    return dir + "\\" + appHashHex + ".seq";
#else
    return dir + "/" + appHashHex + ".seq";
#endif
}

std::string SignedUpdateInstaller::download(const std::string& manifestUrl,
                                            const std::string& pinnedReleaseKeyHex,
                                            const std::string& application,
                                            const std::string& destination) {
    if (manifestUrl.rfind("https://", 0) != 0 || manifestUrl.find('#') != std::string::npos || manifestUrl.find('@') != std::string::npos) {
        throw std::invalid_argument("Release manifest must use an HTTPS URL without user info or fragment.");
    }
    if (pinnedReleaseKeyHex.size() != 64) {
        throw std::invalid_argument("Pinned release key must be 64 hex characters.");
    }
    if (application.empty()) {
        throw std::invalid_argument("Application name cannot be empty.");
    }
    if (destination.empty()) {
        throw std::invalid_argument("Destination path cannot be empty.");
    }

    std::string seqPath = getSequencePath(application);
    ensureDirectoriesForFile(seqPath);

    uint64_t currentSeq = 0;
    FILE* sf = std::fopen(seqPath.c_str(), "r");
    if (sf) {
        char buf[64];
        if (std::fgets(buf, sizeof(buf), sf)) {
            char* end = nullptr;
            long long val = std::strtoll(buf, &end, 10);
            if (val < 0) {
                std::fclose(sf);
                throw std::runtime_error("Stored release sequence is corrupt; refusing update.");
            }
            currentSeq = static_cast<uint64_t>(val);
        }
        std::fclose(sf);
    }

    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize libcurl.");

    ManifestBuffer mb;
    curl_easy_setopt(curl, CURLOPT_URL, manifestUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, manifestWriteCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");

    struct curl_slist* headers = curl_slist_append(nullptr, "User-Agent: NightAuth-Updater/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || httpCode != 200 || mb.overflow || mb.data.empty()) {
        throw std::runtime_error("Failed to fetch release manifest: HTTP " + std::to_string(httpCode) + " (" + curl_easy_strerror(res) + ")");
    }

    VerifiedUpdate verified = SignedUpdate::verify(mb.data, pinnedReleaseKeyHex, application, currentSeq);
    SignedUpdate::download(verified, destination);

    std::string tmpSeq = seqPath + "." + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count()) + ".tmp";
    FILE* outSf = std::fopen(tmpSeq.c_str(), "w");
    if (outSf) {
        std::fprintf(outSf, "%llu\n", static_cast<unsigned long long>(verified.sequence));
        std::fclose(outSf);
        std::remove(seqPath.c_str());
        std::rename(tmpSeq.c_str(), seqPath.c_str());
    }

    return destination;
}
#endif

#ifdef NIGHTAUTH_TEST_HOOKS
struct TestAccess {
    static std::string sessionKey(const Api& api) { return api.m_sessionKey; }

    static std::string& sessionKey(Api& api) { return api.m_sessionKey; }

    static std::string sharedKeyHex(const Api& api) {
        const auto& k = api.m_impl->sharedKey;
        return k.empty() ? std::string() : Crypto::toHex(k.data(), k.size());
    }
    static void primeSession(Api& api, const std::string& sessionKey, const std::string& sharedKeyHex) {
        Crypto::Bytes key = Crypto::fromHex(sharedKeyHex);
        if (key.size() != 32) throw std::invalid_argument("primeSession: shared key must be 32 bytes");
        api.m_sessionKey = sessionKey;
        std::memcpy(api.m_sharedKey, key.data(), 32);
        api.m_impl->sharedKey = key;
        api.m_isInitialized = true;
    }
};

namespace TestHooks {
void fillTransportResponse(Api::TransportResponse& out, const std::string& body) { fillFromJson(out, body); }
std::string canonicalJson(const std::string& json) { return Json::serialize(Json::parse(json)); }
std::string jsonField(const std::string& json, const std::string& key) {
    Json::Value v = Json::parse(json);
    const Json::Value* f = v.find(key);
    if (!f) return "";
    if (f->type == Json::Type::String) return f->strVal;
    if (f->type == Json::Type::Number) return Json::serialize(*f);
    return "";
}
void primeSession(Api& api, const std::string& sessionKey, const std::string& sharedKeyHex) {
    TestAccess::primeSession(api, sessionKey, sharedKeyHex);
}
std::string sessionKey(const Api& api) { return TestAccess::sessionKey(api); }
bool gunzipLimited(const std::string& gzip, size_t maxOut, std::string& out) { return Gzip::gunzip(gzip, maxOut, out); }
}
#endif

}

#endif
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define NIGHTAUTH_IMPLEMENTATION
#include "nightauth.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <thread>

static NightAuth::Api NightAuthApp(
    "",
    "",
    "1.0",
    ""
);

/* ANSI TrueColor & 256-Color Design Tokens */
#define C_RESET      "\x1b[0m"
#define C_BOLD       "\x1b[1m"
#define C_DIM        "\x1b[38;2;100;116;139m"
#define C_MUTED      "\x1b[38;2;148;163;184m"
#define C_TEXT       "\x1b[38;2;226;232;240m"
#define C_WHITE      "\x1b[38;2;248;250;252m"
#define C_BLUE       "\x1b[38;2;59;130;246m"
#define C_GLOW       "\x1b[38;2;96;165;250m"
#define C_CYAN       "\x1b[38;2;56;189;248m"
#define C_GREEN      "\x1b[38;2;52;211;153m"
#define C_AMBER      "\x1b[38;2;251;191;36m"
#define C_RED        "\x1b[38;2;248;113;113m"
#define C_BORDER     "\x1b[38;2;42;50;70m"
#define C_BADGE_BG   "\x1b[48;2;23;37;68m"
#define C_GREEN_BG   "\x1b[48;2;14;46;38m"
#define C_RED_BG     "\x1b[48;2;56;20;26m"

static void clear_screen() {
    std::printf("\x1b[2J\x1b[H");
    std::fflush(stdout);
}

static void print_hr() {
    std::printf("  " C_BORDER "────────────────────────────────────────────────────────────────────────────" C_RESET "\n");
}

static void print_section_header(const char* title, const char* subtitle) {
    std::printf("\n  " C_BLUE "▌" C_RESET " " C_WHITE C_BOLD "%s" C_RESET, title);
    if (subtitle && subtitle[0]) {
        std::printf("  " C_DIM "·  %s" C_RESET, subtitle);
    }
    std::printf("\n");
    print_hr();
}

static void print_ok(const char* title, const std::string& detail = "") {
    std::printf("\n  " C_GREEN_BG C_GREEN C_BOLD " ✓ SUCCESS " C_RESET " " C_WHITE C_BOLD "%s" C_RESET, title);
    if (!detail.empty()) {
        std::printf(" " C_MUTED "— %s" C_RESET, detail.c_str());
    }
    std::printf("\n\n");
}

static void print_err(const char* title, const std::string& detail = "") {
    std::printf("\n  " C_RED_BG C_RED C_BOLD " ✕ ERROR " C_RESET " " C_WHITE C_BOLD "%s" C_RESET, title);
    if (!detail.empty()) {
        std::printf(" " C_RED "— %s" C_RESET, detail.c_str());
    }
    std::printf("\n\n");
}

static void print_info(const char* label, const std::string& value) {
    std::printf("  " C_BLUE "•" C_RESET " " C_MUTED "%-18s" C_RESET " " C_CYAN "%s" C_RESET "\n", label, value.c_str());
}

static std::string ask(const std::string& prompt) {
    std::printf("  " C_GLOW "❯" C_RESET " " C_TEXT "%s" C_RESET, prompt.c_str());
    std::fflush(stdout);
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::exit(0);
    }
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
        line.pop_back();
    }
    return line;
}

static void render_banner(const std::string& app_name, const std::string& hwid, bool connected) {
    std::printf("\n");
    std::printf("  " C_BLUE C_BOLD "  _   _ _       _     _      _         _   _     \n" C_RESET);
    std::printf("  " C_BLUE C_BOLD " | \\ | (_) __ _| |__ | |_   / \\  _   _| |_| |__  \n" C_RESET);
    std::printf("  " C_GLOW C_BOLD " |  \\| | |/ _` | '_ \\| __| / _ \\| | | | __| '_ \\ \n" C_RESET);
    std::printf("  " C_GLOW C_BOLD " | |\\  | | (_| | | | | |_ / ___ \\ |_| | |_| | | |\n" C_RESET);
    std::printf("  " C_CYAN C_BOLD " |_| \\_|_|\\__, |_| |_|\\__/_/   \\_\\__,_|\\__|_| |_|\n" C_RESET);
    std::printf("  " C_CYAN C_BOLD "          |___/                                  " C_RESET "  " C_BADGE_BG C_GLOW C_BOLD " NATIVE C++ CLI " C_RESET "\n\n");

    std::printf("  " C_BORDER "╭──────────────────────────────────────────────────────────────────────────╮" C_RESET "\n");
    std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "APPLICATION" C_RESET "  " C_WHITE C_BOLD "%-26.26s" C_RESET "  " C_DIM "STATUS" C_RESET "  %s%-16s" C_RESET " " C_BORDER "│" C_RESET "\n",
                !app_name.empty() ? app_name.c_str() : "Unconfigured",
                connected ? C_GREEN "● " : C_AMBER "◌ ",
                connected ? "Connected" : "Initializing");
    if (!hwid.empty()) {
        std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "DEVICE HWID" C_RESET "  " C_CYAN "%-58.58s" C_RESET " " C_BORDER "│" C_RESET "\n", hwid.c_str());
    }
    std::printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n");
}

static void perform_handshake(NightAuth::Api& api, const std::string& app_name) {
    std::printf("\n  " C_MUTED "◌ Establishing encrypted session with NightAuth Cloud..." C_RESET "\r");
    std::fflush(stdout);
    try {
        api.init();
    } catch (const std::exception& ex) {
        std::printf("                                                                      \r");
        render_banner(app_name, "", false);
        print_err("Handshake Exception", ex.what());
        std::exit(1);
    }
    std::printf("                                                                      \r");
    std::fflush(stdout);

    if (!api.response.success) {
        render_banner(app_name, "", false);
        print_err("Handshake Failed", api.response.message);
        std::printf("  " C_DIM "Please verify your App Name, API Key, and Server URL." C_RESET "\n\n");
        ask("Press Enter to exit...");
        std::exit(1);
    }

    std::string hwid = NightAuth::SecurityProtections::getHardwareId();
    clear_screen();
    render_banner(app_name, hwid, true);
}

static bool handle_login(NightAuth::Api& api) {
    print_section_header("ACCOUNT SIGN IN", "Authenticate with your username and password");
    std::string user = ask("Username: ");
    std::string pass = ask("Password: ");
    std::string totp = ask("2FA Code " C_DIM "(Optional, Enter to skip)" C_TEXT ": ");

    if (api.login(user, pass, totp)) {
        print_ok("Session Authenticated", "Welcome back to NightAuth.");
        return true;
    }

    print_err("Login Failed", api.response.message);
    return false;
}

static bool handle_register(NightAuth::Api& api) {
    print_section_header("CREATE NEW ACCOUNT", "Activate a license key and bind your credentials");
    std::string user  = ask("Username: ");
    std::string pass  = ask("Password: ");
    std::string key   = ask("License Key: ");
    std::string email = ask("Email Address " C_DIM "(Optional, Enter to skip)" C_TEXT ": ");

    if (api.registerAccount(user, pass, key, email)) {
        print_ok("Account Registered & Activated", "Session started automatically.");
        return true;
    }

    std::string detail = api.response.message;
    if (!api.response.error.empty()) {
        detail += " [" + api.response.error + "]";
    }
    print_err("Registration Failed", detail);
    return false;
}

static bool handle_license_only(NightAuth::Api& api) {
    print_section_header("LICENSE KEY AUTHENTICATION", "Instant passwordless login bound to your HWID");
    std::string key = ask("Enter License Key: ");

    if (api.license(key)) {
        print_ok("License Validated", "Direct hardware-bound session active.");
        return true;
    }

    print_err("License Validation Failed", api.response.message);
    return false;
}

static bool handle_upgrade(NightAuth::Api& api, const std::string& known_user = "") {
    print_section_header("UPGRADE ACCOUNT SUBSCRIPTION", "Attach a new license key tier to an account");
    std::string user = known_user;
    if (!user.empty()) {
        print_info("Target Account:", user);
    } else {
        user = ask("Target Username: ");
    }
    std::string key = ask("Upgrade License Key: ");

    if (api.upgrade(user, key)) {
        print_ok("Upgrade Complete", api.response.message);
        return true;
    } else {
        print_err("Upgrade Failed", api.response.message);
        return false;
    }
}

static void handle_forgot_password(NightAuth::Api& api) {
    print_section_header("PASSWORD RECOVERY", "Request a reset link to your registered email");
    std::string email = ask("Registered Email: ");

    if (api.forgotPassword(email)) {
        print_ok("Recovery Dispatched", "If an account matches, a password reset link was sent to your email.");
    } else {
        print_err("Request Failed", api.response.message);
    }
}

static void handle_reset_password(NightAuth::Api& api) {
    print_section_header("COMPLETE PASSWORD RESET", "Confirm recovery token and set a new password");
    std::string token    = ask("Reset Token: ");
    std::string new_pass = ask("New Password: ");

    if (api.resetPassword(token, new_pass)) {
        print_ok("Password Updated", "Your password has been reset successfully.");
    } else {
        print_err("Reset Failed", api.response.message);
    }
}

static void handle_reset_hwid(NightAuth::Api& api) {
    print_section_header("RESET HARDWARE FINGERPRINT (HWID)", "Unbind previous device to authorize a new machine");
    std::string user = ask("Username: ");
    std::string pass = ask("Password: ");
    std::string totp = ask("2FA Code " C_DIM "(Optional, Enter to skip)" C_TEXT ": ");

    if (api.resetHwid(user, pass, totp)) {
        print_ok("HWID Reset", api.response.message);
    } else {
        print_err("HWID Reset Failed", api.response.message);
    }
}

static void handle_check_blacklist(NightAuth::Api& api) {
    print_section_header("HARDWARE BLACKLIST CHECK", "Verify if a machine fingerprint is restricted");
    std::string target_hwid = ask("Hardware ID " C_DIM "(Press Enter for local machine HWID)" C_TEXT ": ");
    if (target_hwid.empty()) {
        target_hwid = NightAuth::SecurityProtections::getHardwareId();
    }

    bool is_blacklisted = api.checkBlacklist(target_hwid);
    if (api.response.success) {
        if (is_blacklisted) {
            print_err("Blacklisted", "This Hardware ID is currently restricted.");
        } else {
            print_ok("Clean Fingerprint", "Hardware ID is not on the blacklist.");
        }
    } else {
        print_err("Blacklist Query Failed", api.response.message);
    }
}

static void handle_verify_hash(NightAuth::Api& api) {
    print_section_header("BINARY INTEGRITY CHECK", "Validate SHA-256 checksum against server registry");
    std::string hash = ask("SHA-256 File Hash: ");
    bool verified = api.verifyFileHash(hash);
    if (api.response.success) {
        if (verified) {
            print_ok("Integrity Verified", "Checksum matches authorized server release.");
        } else {
            print_err("Verification Failed", "Checksum does not match any approved file release.");
        }
    } else {
        print_err("Verification Failed", api.response.message);
    }
}

static bool handle_authentication_flow(NightAuth::Api& api) {
    while (true) {
        print_section_header("AUTHENTICATION & PRE-SESSION GATEWAY", "Select an operation to continue");

        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 01 " C_RESET " " C_WHITE "Sign In (Username & Password)" C_RESET
                    "          " C_BADGE_BG C_GLOW C_BOLD " 06 " C_RESET " " C_WHITE "Complete Password Reset (Token)" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 02 " C_RESET " " C_WHITE "Register New Account + License" C_RESET
                    "         " C_BADGE_BG C_GLOW C_BOLD " 07 " C_RESET " " C_WHITE "Reset Hardware Binding (HWID)" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 03 " C_RESET " " C_WHITE "Instant License Key Login" C_RESET
                    "              " C_BADGE_BG C_GLOW C_BOLD " 08 " C_RESET " " C_WHITE "Check HWID Blacklist Status" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 04 " C_RESET " " C_WHITE "Upgrade Existing Account Plan" C_RESET
                    "          " C_BADGE_BG C_GLOW C_BOLD " 09 " C_RESET " " C_WHITE "Verify Binary SHA-256 Hash" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 05 " C_RESET " " C_WHITE "Forgot Password (Email Link)" C_RESET
                    "           " C_RED_BG C_RED C_BOLD " 10 " C_RESET " " C_MUTED "Exit Application" C_RESET "\n\n");

        std::string sel = ask("Select option [1-10]: ");

        if (sel == "1" || sel == "01") {
            if (handle_login(api)) return true;
        } else if (sel == "2" || sel == "02") {
            if (handle_register(api)) return true;
        } else if (sel == "3" || sel == "03") {
            if (handle_license_only(api)) return true;
        } else if (sel == "4" || sel == "04") {
            handle_upgrade(api);
        } else if (sel == "5" || sel == "05") {
            handle_forgot_password(api);
        } else if (sel == "6" || sel == "06") {
            handle_reset_password(api);
        } else if (sel == "7" || sel == "07") {
            handle_reset_hwid(api);
        } else if (sel == "8" || sel == "08") {
            handle_check_blacklist(api);
        } else if (sel == "9" || sel == "09") {
            handle_verify_hash(api);
        } else if (sel == "10" || sel == "0") {
            return false;
        }
    }
}

static void render_user_profile(const NightAuth::Api& api) {
    const auto& u = api.user;
    std::printf("\n");
    std::printf("  " C_BORDER "╭─ " C_GLOW C_BOLD "AUTHENTICATED IDENTITY & ENTITLEMENTS" C_BORDER " ──────────────────────────────────╮" C_RESET "\n");
    std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "Username :" C_RESET " " C_WHITE C_BOLD "%-24.24s" C_RESET "  " C_DIM "Status     :" C_RESET " " C_GREEN C_BOLD "%-20.20s" C_RESET " " C_BORDER "│" C_RESET "\n",
                !u.username.empty() ? u.username.c_str() : "Authenticated",
                !u.status.empty() ? u.status.c_str() : "ACTIVE");
    std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "Email    :" C_RESET " " C_TEXT "%-24.24s" C_RESET "  " C_DIM "Client IP  :" C_RESET " " C_CYAN "%-20.20s" C_RESET " " C_BORDER "│" C_RESET "\n",
                !u.email.empty() ? u.email.c_str() : "Not linked",
                !u.ip.empty() ? u.ip.c_str() : "Protected / TLS");
    std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "Created  :" C_RESET " " C_MUTED "%-24.24s" C_RESET "  " C_DIM "Last Login :" C_RESET " " C_MUTED "%-20.20s" C_RESET " " C_BORDER "│" C_RESET "\n",
                !u.created_at.empty() ? u.created_at.c_str() : "-",
                !u.last_login_at.empty() ? u.last_login_at.c_str() : "Just now");
    std::printf("  " C_BORDER "├──────────────────────────────────────────────────────────────────────────┤" C_RESET "\n");

    if (!u.subscriptions.empty()) {
        for (const auto& sub : u.subscriptions) {
            std::printf("  " C_BORDER "│" C_RESET "  " C_GREEN "●" C_RESET " " C_WHITE C_BOLD "%-16.16s" C_RESET " " C_DIM "Key:" C_RESET " " C_CYAN "%-22.22s" C_RESET " " C_DIM "Exp:" C_RESET " " C_AMBER "%-16.16s" C_RESET " " C_BORDER "│" C_RESET "\n",
                        sub.subscription.c_str(),
                        !sub.key.empty() ? sub.key.c_str() : "Direct Entitlement",
                        !sub.expiry.empty() ? sub.expiry.c_str() : "Lifetime");
        }
    } else {
        std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "No active subscription tiers attached to this account." C_RESET "                   " C_BORDER "│" C_RESET "\n");
    }
    std::printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n");
}

static void handle_chat_room(NightAuth::Api& api) {
    print_section_header("LIVE CHANNEL CHAT", "Type /exit at any time to leave the channel");
    std::string room = ask("Channel name (e.g. general): ");
    if (room.empty()) room = "general";

    auto msgs = api.getChatMessages(room, 0, true);
    std::printf("\n  " C_BORDER "╭─ " C_GLOW C_BOLD "#%-20s" C_BORDER " ──────────────────────────────────────────────────╮" C_RESET "\n", room.c_str());
    if (msgs.empty()) {
        std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "No messages in this channel yet. Start the conversation below." C_RESET "          " C_BORDER "│" C_RESET "\n");
    } else {
        for (const auto& m : msgs) {
            std::printf("  " C_BORDER "│" C_RESET " " C_DIM "[%s]" C_RESET " " C_CYAN C_BOLD "%s" C_RESET ": " C_TEXT "%s" C_RESET "\n",
                        m.timestamp.c_str(), m.author.c_str(), m.message.c_str());
        }
    }
    std::printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n\n");

    while (true) {
        std::string input = ask(std::string(C_CYAN "#") + room + C_RESET " message: ");
        if (input == "/exit" || input == "/EXIT") {
            std::printf("  " C_DIM "Left channel #%s." C_RESET "\n\n", room.c_str());
            return;
        }
        if (input.empty()) continue;
        if (!api.sendChatMessage(input, room)) {
            print_err("Send Failed", api.response.message);
        }
    }
}

static void render_stats_table(NightAuth::Api& api) {
    NightAuth::AppStats stats;
    if (api.getStats(stats)) {
        std::printf("\n");
        std::printf("  " C_BORDER "╭─ " C_GLOW C_BOLD "APPLICATION LIVE TELEMETRY" C_BORDER " ─────────────────────────────────────────────╮" C_RESET "\n");
        std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "Total Users     :" C_RESET " " C_WHITE C_BOLD "%-14lld" C_RESET "  " C_DIM "Total Licenses  :" C_RESET " " C_WHITE C_BOLD "%-18lld" C_RESET " " C_BORDER "│" C_RESET "\n",
                    static_cast<long long>(stats.total_users), static_cast<long long>(stats.total_licenses));
        std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "Active Users    :" C_RESET " " C_GREEN C_BOLD "%-14lld" C_RESET "  " C_DIM "Used Licenses   :" C_RESET " " C_CYAN C_BOLD "%-18lld" C_RESET " " C_BORDER "│" C_RESET "\n",
                    static_cast<long long>(stats.active_users), static_cast<long long>(stats.used_licenses));
        std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "Banned Users    :" C_RESET " " C_RED C_BOLD "%-14lld" C_RESET "  " C_DIM "Unused Licenses :" C_RESET " " C_AMBER C_BOLD "%-18lld" C_RESET " " C_BORDER "│" C_RESET "\n",
                    static_cast<long long>(stats.banned_users), static_cast<long long>(stats.unused_licenses));
        std::printf("  " C_BORDER "│" C_RESET "  " C_DIM "Target Version  :" C_RESET " " C_GLOW "%-52s" C_RESET " " C_BORDER "│" C_RESET "\n",
                    !stats.version.empty() ? stats.version.c_str() : api.version.c_str());
        std::printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n\n");
    } else {
        print_err("Telemetry Error", api.response.message);
    }
}

static void handle_setup_2fa(NightAuth::Api& api) {
    print_section_header("TWO-FACTOR AUTHENTICATION (TOTP)", "Enroll an authenticator app (RFC 6238)");
    NightAuth::TwoFactorSetupData setup;
    if (api.setup2FA(setup)) {
        print_info("TOTP Secret Key:", setup.secret);
        print_info("Provisioning URI:", setup.uri);
        std::string code = ask("Enter 6-digit Authenticator code: ");
        NightAuth::TwoFactorConfirmData conf;
        if (api.confirm2FA(code, conf)) {
            print_ok("2FA Activated", "Store these recovery backup codes safely:");
            for (const auto& bc : conf.backup_codes) {
                std::printf("    " C_AMBER "• %s" C_RESET "\n", bc.c_str());
            }
            std::printf("\n");
        } else {
            print_err("Verification Failed", api.response.message);
        }
    } else {
        print_err("2FA Setup Failed", api.response.message);
    }
}

static void handle_download_file(NightAuth::Api& api) {
    print_section_header("PROTECTED ASSET DOWNLOADER", "Download encrypted files via scoped access ticket");
    std::string fileId = ask("Enter File ID or Asset Name: ");
    std::string content;
    if (api.downloadFile(fileId, content)) {
        print_ok("Download Complete", "Retrieved " + std::to_string(content.size()) + " bytes securely.");
    } else {
        print_err("Download Failed", api.response.message);
    }
}

static void handle_list_all_user_vars(NightAuth::Api& api) {
    std::vector<std::pair<std::string, std::string>> vars;
    if (!api.getUserVariables(vars)) {
        print_err("Fetch Failed", api.response.message);
        return;
    }
    print_section_header("USER CLOUD VARIABLES", "Per-user encrypted key-value pairs");
    if (vars.empty()) {
        std::printf("  " C_DIM "No user variables stored for this account yet." C_RESET "\n\n");
        return;
    }
    for (const auto& kv : vars) {
        std::printf("  " C_BLUE "•" C_RESET " " C_CYAN C_BOLD "%-22s" C_RESET " = " C_WHITE "%s" C_RESET "\n", kv.first.c_str(), kv.second.c_str());
    }
    std::printf("\n");
}

static void handle_trigger_webhook(NightAuth::Api& api) {
    print_section_header("SERVER-SIDE WEBHOOK TRIGGER", "Execute an authenticated webhook endpoint");
    std::string webId   = ask("Webhook ID: ");
    std::string payload = ask("JSON Payload " C_DIM "(Enter for {})" C_TEXT ": ");
    if (payload.empty()) payload = "{}";

    NightAuth::WebhookResult result;
    try {
        if (api.triggerWebhook(webId, payload, result)) {
            print_ok("Webhook Dispatched", "HTTP " + std::to_string(result.status) + " OK");
        } else {
            print_err("Webhook Failed", api.response.message);
        }
    } catch (const std::exception& ex) {
        print_err("Invalid Webhook Payload", ex.what());
    }
}

static void handle_events_feed(NightAuth::Api& api) {
    print_section_header("REAL-TIME APPLICATION EVENTS (SSE)", "Press Enter at any time to stop listening");
    std::atomic<bool> cancel{false};

    std::thread stopper([&]() {
        std::string line;
        std::getline(std::cin, line);
        cancel.store(true);
    });

    bool ok = api.subscribeEvents([](const NightAuth::EventMessage& ev) {
        std::printf("  " C_BADGE_BG C_GLOW " EVENT " C_RESET " " C_DIM "[%s]" C_RESET " " C_CYAN C_BOLD "%s" C_RESET ": %s\n",
                    ev.id.c_str(), ev.type.c_str(), ev.data.c_str());
        return true;
    }, &cancel);

    cancel.store(true);
    if (stopper.joinable()) stopper.join();

    if (ok) {
        print_ok("Stream Closed", "Disconnected from real-time event feed.");
    } else {
        print_err("Stream Interrupted", api.response.message);
    }
}

static void handle_self_ban(NightAuth::Api& api) {
    print_section_header("EMERGENCY KILLSWITCH / BAN", "Report security violation and lock this session");
    std::string reason = ask("Ban Reason " C_DIM "(Optional)" C_TEXT ": ");

    if (api.ban(reason)) {
        print_ok("Killswitch Executed", api.response.message);
    } else {
        print_err("Ban Report Failed", api.response.message);
    }
}

static void handle_heartbeat(NightAuth::Api& api) {
    if (api.heartbeat()) {
        print_ok("Heartbeat Acknowledged", api.response.message);
    } else {
        print_err("Heartbeat Failed", api.response.message);
    }
}

static void handle_kill_session_demo(NightAuth::Api& api) {
    print_section_header("SESSION REVOCATION & INVALIDATION VERIFICATION", "3-Step Live Security Audit");
    std::printf("  " C_MUTED "1. Verifying active session before revocation..." C_RESET "\n");
    if (api.check()) {
        print_ok("Step 1 Passed", "Session is active and authenticated on server.");
    } else {
        print_err("Step 1 Failed", api.response.message);
        return;
    }

    std::printf("  " C_MUTED "2. Revoking and terminating session on server (/logout)..." C_RESET "\n");
    if (api.logout()) {
        print_ok("Step 2 Passed", "Server confirmed session revocation.");
    } else {
        print_err("Step 2 Failed", api.response.message);
    }

    std::printf("  " C_MUTED "3. Verifying that subsequent session check is rejected..." C_RESET "\n");
    if (api.check()) {
        print_err("Step 3 Unexpected", "Session was still accepted after revocation.");
    } else {
        print_ok("Step 3 Passed", "Server rejected revoked session as expected.");
    }
}

static void run_application_features_menu(NightAuth::Api& api) {
    while (true) {
        print_section_header("WORKSPACE MODULES & ACTIONS", "Select a command [1-21]");

        std::printf("  " C_DIM "CLOUD VARIABLES & CHAT" C_RESET "                    " C_DIM "SECURITY & TELEMETRY" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 01 " C_RESET " " C_WHITE "Read Global Variable" C_RESET
                    "                 " C_BADGE_BG C_GLOW C_BOLD " 10 " C_RESET " " C_WHITE "Setup Two-Factor Auth (2FA)" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 02 " C_RESET " " C_WHITE "Get User Cloud Variable" C_RESET
                    "              " C_BADGE_BG C_GLOW C_BOLD " 11 " C_RESET " " C_WHITE "Disable Two-Factor Auth (2FA)" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 03 " C_RESET " " C_WHITE "Set User Cloud Variable" C_RESET
                    "              " C_BADGE_BG C_GLOW C_BOLD " 12 " C_RESET " " C_WHITE "Download Protected Asset" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 04 " C_RESET " " C_WHITE "List All User Variables" C_RESET
                    "              " C_BADGE_BG C_GLOW C_BOLD " 13 " C_RESET " " C_WHITE "Trigger Server Webhook" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 05 " C_RESET " " C_WHITE "Enter Live Chat Channel" C_RESET
                    "              " C_BADGE_BG C_GLOW C_BOLD " 14 " C_RESET " " C_WHITE "Send Audit Log Event" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 19 " C_RESET " " C_WHITE "Fetch Paginated Chat History" C_RESET
                    "         " C_BADGE_BG C_GLOW C_BOLD " 15 " C_RESET " " C_WHITE "Verify Session Entitlement" C_RESET "\n\n");

        std::printf("  " C_DIM "ACCOUNT & INSIGHTS" C_RESET "                        " C_DIM "SESSION LIFECYCLE" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 06 " C_RESET " " C_WHITE "Application Live Statistics" C_RESET
                    "          " C_BADGE_BG C_GLOW C_BOLD " 16 " C_RESET " " C_WHITE "Send Heartbeat Ping" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 07 " C_RESET " " C_WHITE "View Online Active Users" C_RESET
                    "             " C_BADGE_BG C_GLOW C_BOLD " 17 " C_RESET " " C_WHITE "Emergency Killswitch / Ban" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 08 " C_RESET " " C_WHITE "Change Account Username" C_RESET
                    "              " C_BADGE_BG C_GLOW C_BOLD " 18 " C_RESET " " C_WHITE "Stream Live Events (SSE)" C_RESET "\n");
        std::printf("  " C_BADGE_BG C_GLOW C_BOLD " 09 " C_RESET " " C_WHITE "Upgrade Plan with License Key" C_RESET
                    "        " C_BADGE_BG C_GLOW C_BOLD " 20 " C_RESET " " C_WHITE "Session Revocation Audit Demo" C_RESET "\n");
        std::printf("                                            " C_RED_BG C_RED C_BOLD " 21 " C_RESET " " C_MUTED "Sign Out & Terminate Session" C_RESET "\n\n");

        std::string choice = ask("Action [1-21]: ");
        int num = std::atoi(choice.c_str());

        if (num == 1) {
            std::string gKey = ask("Global Variable Key: ");
            std::string gVal = api.getGlobalVar(gKey);
            if (!gVal.empty()) {
                print_ok(gKey.c_str(), gVal);
            } else {
                print_err("Fetch Failed", api.response.message);
            }
        } else if (num == 2) {
            std::string uKey = ask("User Variable Key: ");
            std::string uVal = api.getUserVar(uKey);
            if (!uVal.empty()) {
                print_ok(uKey.c_str(), uVal);
            } else {
                print_err("Fetch Failed", api.response.message);
            }
        } else if (num == 3) {
            std::string sKey = ask("User Variable Key: ");
            std::string sVal = ask("Value to Store: ");
            if (api.setUserVar(sKey, sVal)) {
                print_ok("Variable Saved", sKey);
            } else {
                print_err("Save Failed", api.response.message);
            }
        } else if (num == 4) {
            handle_list_all_user_vars(api);
        } else if (num == 5) {
            handle_chat_room(api);
        } else if (num == 6) {
            render_stats_table(api);
        } else if (num == 7) {
            auto users = api.getOnlineUsers();
            std::string hdr = std::to_string(users.size()) + " active session(s)";
            print_section_header("ONLINE ACTIVE SESSIONS", hdr.c_str());
            for (const auto& u : users) {
                std::printf("  " C_GREEN "●" C_RESET " " C_WHITE "%s" C_RESET "\n", u.c_str());
            }
            std::printf("\n");
        } else if (num == 8) {
            std::string newU = ask("New Username: ");
            std::string p1   = ask("Confirm Password: ");
            if (api.changeUsername(newU, p1)) {
                print_ok("Username Updated", api.user.username);
                std::printf("  " C_AMBER "For security, your session has been closed. Please sign in with your new username." C_RESET "\n\n");
                return;
            } else {
                print_err("Update Failed", api.response.message);
            }
        } else if (num == 9) {
            if (handle_upgrade(api, api.user.username)) {
                api.check();
                render_user_profile(api);
            }
        } else if (num == 10) {
            handle_setup_2fa(api);
        } else if (num == 11) {
            std::string p3    = ask("Account Password: ");
            std::string code3 = ask("6-digit Authenticator Code: ");
            if (api.disable2FA(p3, code3)) {
                print_ok("2FA Disabled", "Two-factor protection has been removed from this account.");
            } else {
                print_err("Disable Failed", api.response.message);
            }
        } else if (num == 12) {
            handle_download_file(api);
        } else if (num == 13) {
            handle_trigger_webhook(api);
        } else if (num == 14) {
            std::string logMsg = ask("Audit Log Message: ");
            if (api.log(logMsg)) {
                print_ok("Audit Log Recorded", "Delivered to application telemetry feed.");
            } else {
                print_err("Log Failed", api.response.message);
            }
        } else if (num == 15) {
            if (api.check()) {
                print_ok("Session Verified", "Active and cryptographically valid.");
                render_user_profile(api);
            } else {
                print_err("Session Invalid", api.response.message);
            }
        } else if (num == 16) {
            handle_heartbeat(api);
        } else if (num == 17) {
            handle_self_ban(api);
        } else if (num == 18) {
            handle_events_feed(api);
        } else if (num == 19) {
            std::string hist_room   = ask("Channel Name: ");
            std::string hist_before = ask("Before Message ID " C_DIM "(Optional, Enter for latest)" C_TEXT ": ");
            std::string hist_mr     = ask("Mark messages as read? (y/n): ");
            int64_t before_id = !hist_before.empty() ? std::atoll(hist_before.c_str()) : 0;
            bool mark_read = (!hist_mr.empty() && (hist_mr[0] == 'y' || hist_mr[0] == 'Y'));

            auto hist = api.getChatMessages(hist_room, before_id, mark_read);
            if (!api.response.success) {
                print_err("History Query Failed", api.response.message);
            } else {
                std::string sub = std::to_string(hist.size()) + " message(s) retrieved";
                print_section_header(hist_room.c_str(), sub.c_str());
                for (const auto& m : hist) {
                    std::printf("  " C_DIM "[%s]" C_RESET " " C_CYAN C_BOLD "%s" C_RESET ": " C_TEXT "%s" C_RESET " " C_DIM "(#%s · %s)" C_RESET "\n",
                                m.timestamp.c_str(), m.author.c_str(), m.message.c_str(),
                                m.id.c_str(), !m.status.empty() ? m.status.c_str() : "-");
                }
                std::printf("\n");
            }
        } else if (num == 20) {
            handle_kill_session_demo(api);
            return;
        } else if (num == 21) {
            api.logout();
            print_ok("Signed Out", "Session revoked cleanly. Goodbye!");
            return;
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitleA("NightAuth — Native C++ Security Console");
#endif

    const char* env_name  = std::getenv("NIGHTAUTH_APP_NAME");
    const char* env_key   = std::getenv("NIGHTAUTH_API_KEY");
    const char* env_ver   = std::getenv("NIGHTAUTH_APP_VERSION");
    const char* env_token = std::getenv("NIGHTAUTH_APP_TOKEN");
    const char* env_url   = std::getenv("NIGHTAUTH_API_URL");

    std::string appName  = (env_name && env_name[0]) ? env_name : NightAuthApp.name;
    std::string apiKey   = (env_key && env_key[0]) ? env_key : NightAuthApp.api_key;
    std::string appVer   = (env_ver && env_ver[0]) ? env_ver : NightAuthApp.version;
    std::string appToken = (env_token && env_token[0]) ? env_token : NightAuthApp.token;
    std::string apiUrl   = (env_url && env_url[0]) ? env_url : NightAuthApp.api_url;

    NightAuthApp.name = appName;
    NightAuthApp.api_key = apiKey;
    NightAuthApp.version = appVer;
    NightAuthApp.token = appToken;
    NightAuthApp.api_url = apiUrl;

    clear_screen();
    render_banner(appName, "", false);

    NightAuth::Api& api = NightAuthApp;
    api.setTransport(NightAuth::Api::makeCurlTransport(apiUrl));
    api.setStreamTransport(NightAuth::Api::makeCurlStreamTransport(apiUrl));

    perform_handshake(api, appName);

    if (!handle_authentication_flow(api)) {
        api.close();
        return 0;
    }

    std::string hwid = NightAuth::SecurityProtections::getHardwareId();
    clear_screen();
    render_banner(appName, hwid, true);
    render_user_profile(api);

    run_application_features_menu(api);
    api.close();
    return 0;
}

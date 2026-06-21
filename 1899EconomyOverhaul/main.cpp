#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <fstream>
#include <string>
#include <chrono>
#include "script.h"

// Replaces GetTickCount() to avoid heuristic AV flags
unsigned long long GetTimeMS() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()
	).count();
}

struct RightToastData {
	alignas(8) int duration;
	alignas(8) const char* SoundDict;
	alignas(8) const char* SoundName;
	alignas(8) int f_3;
};

struct RightToastInfo {
	alignas(8) Any f_0;
	alignas(8) const char* text;
	alignas(8) const char* IconDict;
	alignas(8) int Icon;
	alignas(8) int f_4;
	alignas(8) int ColorHash;
	alignas(8) int Stars;
};

int DisplayItemFeedTicker(std::string text, std::string IconDict, std::string Icon, int duration, std::string soundDict, std::string soundName, std::string color, int stars) {
	RightToastData args1{};
	args1.duration = duration;
	args1.SoundDict = soundDict.c_str();
	args1.SoundName = soundName.c_str();
	args1.f_3 = 0;

	RightToastInfo args2{};
	args2.text = MISC::VAR_STRING(10, "LITERAL_STRING", text.c_str());
	args2.IconDict = IconDict.c_str();
	args2.Icon = MISC::GET_HASH_KEY(Icon.c_str());
	args2.f_4 = 0;
	args2.ColorHash = MISC::GET_HASH_KEY(color.c_str());
	args2.Stars = stars;

	return UIFEED::_UI_FEED_POST_SAMPLE_TOAST_RIGHT((Any*)&args1, (Any*)&args2, true);
}

bool g_logInitialized = false;
bool g_loggingEnabled = false;

void Log(const char* fmt, ...) {
	if (!g_loggingEnabled) return;

	FILE* f;
	const char* mode = g_logInitialized ? "a" : "w";
	if (fopen_s(&f, "1899_Economy_Log.txt", mode) == 0) {
		g_logInitialized = true;
		va_list args;
		va_start(args, fmt);
		std::vfprintf(f, fmt, args);
		va_end(args);
		std::fprintf(f, "\n");
		std::fclose(f);
	}
}

std::string FormatMoney(int cents)
{
	char buffer[64];
	int abs_cents = std::abs(cents);

	if (abs_cents < 100)
	{
		if (cents < 0)
			sprintf_s(buffer, "-%d\xC2\xA2", abs_cents);
		else
			sprintf_s(buffer, "%d\xC2\xA2", abs_cents);
	}
	else
	{
		if (cents < 0)
			sprintf_s(buffer, "-$%.2f", (float)abs_cents / 100.0f);
		else
			sprintf_s(buffer, "$%.2f", (float)abs_cents / 100.0f);
	}

	return std::string(buffer);
}

void InitSettings() {
	g_loggingEnabled = GetPrivateProfileIntA("Settings", "EnableLogging", 0, "./1899EconomyOverhaul.ini") != 0;

	if (g_loggingEnabled) {
		Log("=================================================");
		Log("      1899 Economy Overhaul - Initializing       ");
		Log("=================================================");
		Log("SYSTEM: Logging for savefile initialized successfully.");
	}
}

int ReadIniInt(const char* key, int defaultValue) {
	return GetPrivateProfileIntA("Payouts", key, defaultValue, "./1899EconomyOverhaul.ini");
}

float ReadIniRate(const char* key, float defaultValue) {
	int val = GetPrivateProfileIntA("Payouts", key, (int)(defaultValue * 100.0f), "./1899EconomyOverhaul.ini");
	return (float)val / 100.0f;
}

float get_tax_rate(float dollars) {
	if (dollars < (float)ReadIniInt("Tier1_Limit_Dollars", 15))   return ReadIniRate("Tier1_Tax_Percentage", 0.50f);
	if (dollars < (float)ReadIniInt("Tier2_Limit_Dollars", 100))  return ReadIniRate("Tier2_Tax_Percentage", 0.60f);
	if (dollars < (float)ReadIniInt("Tier3_Limit_Dollars", 500))  return ReadIniRate("Tier3_Tax_Percentage", 0.75f);
	if (dollars < (float)ReadIniInt("Tier4_Limit_Dollars", 2500)) return ReadIniRate("Tier4_Tax_Percentage", 0.90f);
	return ReadIniRate("Tier5Plus_Tax_Percentage", 0.75f);
}

// --- BANK CHECK LOGIC ---
struct BankCoord { float x; float y; float z; };
bool IsNearBankTeller()
{
	Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false);

	BankCoord banks[] = {
		{ -308.5f, 776.0f, 118.7f },    // Valentine
		{ 1294.1f, -1303.1f, 77.0f },   // Rhodes
		{ 2644.2f, -1292.5f, 52.25f },  // Saint Denis
		{ -813.3f, -1277.5f, 43.6f },   // Blackwater
		{ -3666.1f, -2626.6f, -13.6f }, // Armadillo
		{ 2931.3f, 1283.2f, 44.6f }     // Annesburg
	};

	for (const auto& bank : banks)
	{
		float dist = MISC::GET_DISTANCE_BETWEEN_COORDS(playerPos.x, playerPos.y, playerPos.z, bank.x, bank.y, bank.z, true);

		if (dist <= 5.0f)
		{
			return true;
		}
	}
	return false;
}

// --- GAMBLING CHECK LOGIC ---
struct MinigameCoord { float x; float y; float z; float radius; };

bool IsNearTownGamblingZone()
{
	Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false);

	MinigameCoord tables[] = {
		{ -305.89f, 800.73f, 118.97f, 10.0f },    // Valentine Poker
		{ -242.97f, 770.72f, 118.08f, 10.0f },    // Valentine FFF
		{ -326.31f, -357.87f, 88.06f, 10.0f },    // Flatneck Station Poker
		{ 1339.42f, -1370.74f, 84.30f, 10.0f },   // Rhodes Blackjack
		{ -1827.21f, -434.19f, 159.86f, 10.0f },  // Strawberry FFF
		{ 2629.98f, -1225.09f, 53.38f, 10.0f },   // Saint Denis Poker
		{ 2515.73f, -1244.68f, 50.59f, 10.0f },   // Saint Denis Dominoes
		{ 2939.35f, 521.22f, 45.33f, 10.0f },     // Van Horn Trading Post Poker
		{ 2945.68f, 499.13f, 45.75f, 10.0f },     // Van Horn Trading Post FFF
		{ 1522.66f, 431.93f, 90.68f, 10.0f },     // Emerald Ranch Dominoes
		{ -813.94f, -1317.59f, 43.69f, 10.0f },   // Blackwater Poker
		{ -814.60f, -1323.90f, 47.88f, 10.0f },   // Blackwater Blackjack
		{ -937.78f, -1389.71f, 50.58f, 10.0f },   // Blackwater Dominoes
		{ -5511.49f, -2913.04f, 1.64f, 10.0f }    // Tumbleweed Poker
	};

	for (const auto& table : tables)
	{
		float dist = MISC::GET_DISTANCE_BETWEEN_COORDS(playerPos.x, playerPos.y, playerPos.z, table.x, table.y, table.z, true);
		if (dist <= table.radius) return true;
	}
	return false;
}

bool IsNearCampGamblingZone()
{
	Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false);

	MinigameCoord camps[] = {
		// Centralized camp coordinates with a 50m radius to blanket all dynamic minigames
		{ -135.45f, -33.05f, 96.07f, 50.0f },     // Horseshoe Overlook
		{ 685.46f, -1251.99f, 44.26f, 50.0f },    // Clemens Point
		{ 1898.31f, -1881.65f, 42.30f, 50.0f },   // Shady Belle
		{ 2251.36f, -764.31f, 42.89f, 50.0f },    // Lakay
		{ 2359.57f, 1353.58f, 105.96f, 50.0f }    // Beaver Hollow
	};

	for (const auto& camp : camps)
	{
		float dist = MISC::GET_DISTANCE_BETWEEN_COORDS(playerPos.x, playerPos.y, playerPos.z, camp.x, camp.y, camp.z, true);
		if (dist <= camp.radius) return true;
	}
	return false;
}

// Global state trackers
int g_prev_loop_balance = -1;
int g_escrow_balance = 0;
static unsigned long long s_lastDeathTime = 0;
static bool g_transactionProcessed = false;

void update()
{
	// 1. Death Tracker Failsafe
	if (ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID())) {
		s_lastDeathTime = GetTimeMS();
	}

	// 2. Mission State Tracking
	static unsigned long long s_lastMissionEndTime = 0;
	static bool s_wasInMission = false;
	bool currentMissionState = MISC::GET_MISSION_FLAG();

	if (s_wasInMission && !currentMissionState) {
		s_lastMissionEndTime = GetTimeMS();
		Log("STATE: Mission ended. Starting 15-second tax shield override.");
	}
	s_wasInMission = currentMissionState;

	bool recentlyInMission = (GetTimeMS() - s_lastMissionEndTime) < 15000;

	// 3. Proximity Checks
	bool inMenu = !PLAYER::IS_PLAYER_CONTROL_ON(PLAYER::PLAYER_ID());
	bool inBank = IsNearBankTeller();
	bool inTownGambling = IsNearTownGamblingZone();
	bool inCampGambling = IsNearCampGamblingZone();

	// --- GAMBLING SHIELD LOGIC ---
	// Town tables: Always shielded (Protects the Swanson Poker mission)
	// Camp tables: Shielded ONLY if not in a mission and did not just finish one
	bool isCampShieldActive = inCampGambling && !currentMissionState && !recentlyInMission;
	bool isGamblingShieldActive = inTownGambling || isCampShieldActive;

	// If the player is in a menu, near a bank, OR the valid gambling shield is active, ignore transaction
	if (inMenu || inBank || isGamblingShieldActive)
	{
		if (g_prev_loop_balance != -1)
		{
			int current_balance = MONEY::_MONEY_GET_CASH_BALANCE();
			int diff = current_balance - g_prev_loop_balance;

			if (diff > 0)
			{
				if (inBank) {
					Log("UNTAXED: Bank Proximity Active - Transaction ignored. Amount: %s", FormatMoney(diff).c_str());
				}
				else if (isGamblingShieldActive) {
					Log("UNTAXED: Valid Gambling Proximity Active - Transaction ignored. Amount: %s", FormatMoney(diff).c_str());
				}
				else if (inMenu) {
					Log("UNTAXED: Menu/Catalog Active - Transaction ignored. Amount: %s", FormatMoney(diff).c_str());
				}
			}
			g_prev_loop_balance = current_balance;
		}
		else
		{
			g_prev_loop_balance = MONEY::_MONEY_GET_CASH_BALANCE();
			Log("BALANCE: Captured initial cash balance: %s", FormatMoney(g_prev_loop_balance).c_str());
		}
		return;
	}

	// 4. Normal Execution Flow
	if (g_prev_loop_balance == -1) {
		g_prev_loop_balance = MONEY::_MONEY_GET_CASH_BALANCE();
		Log("BALANCE: Captured initial cash balance: %s", FormatMoney(g_prev_loop_balance).c_str());
		return;
	}

	int current_balance = MONEY::_MONEY_GET_CASH_BALANCE();
	int balance_diff = current_balance - g_prev_loop_balance;

	// 5. Escrow Capture & Expenses (Global)
	if (balance_diff < 0) {
		if (current_balance == 0) {
			if ((GetTimeMS() - s_lastDeathTime) > 15000) {
				g_escrow_balance = std::abs(balance_diff);
				Log("ESCROW: Total confiscation detected. Exact amount stored: %s", FormatMoney(g_escrow_balance).c_str());
			}
			else {
				Log("ESCROW: Player died. Escrow capture bypassed.");
			}
		}
		else {
			// --- Log standard purchases/expenses ---
			Log("EXPENSE: Expense or purchase detected. Amount: %s", FormatMoney(balance_diff).c_str());
		}
	}

	// 6. Handle Income (Global)
	else if (balance_diff > 0) {
		int taxable_income = balance_diff;

		// Failsafe A: Checkpoint Retry / Death Protection
		if ((GetTimeMS() - s_lastDeathTime) < 30000) {
			Log("ESCROW: Recent death detected. Shielding refund of %s from taxes.", FormatMoney(taxable_income).c_str());
			taxable_income = 0;
		}

		// Failsafe B: EXACT Match Escrow Refund
		if (g_escrow_balance > 0 && taxable_income > 0) {
			if (taxable_income == g_escrow_balance) {
				Log("ESCROW: Exact refund match detected. Shielding %s from taxes.", FormatMoney(g_escrow_balance).c_str());
				taxable_income = 0; // Shield the refund
				g_escrow_balance = 0; // Clear escrow
			}
			else {
				Log("ESCROW: Income (%s) does not exactly match escrow (%s). Taxing normally.", FormatMoney(taxable_income).c_str(), FormatMoney(g_escrow_balance).c_str());
			}
		}

		// Process Taxes
		if (taxable_income > 0) {
			float V = (float)taxable_income / 100.0f;
			Log("TAXED: Gross payout detected: %s", FormatMoney(taxable_income).c_str());

			float tax_rate = get_tax_rate(V);
			float keep_multiplier = 1.0f - tax_rate;
			int deflated_payout = (int)((V * keep_multiplier) * 100.0f);

			// Check if the original gross payout was a clean dollar amount (cents ending in 00)
			if (taxable_income % 100 == 0) {

				// Round the deflated payout to the nearest 100 cents (whole dollar)
				deflated_payout = (int)(std::round((float)deflated_payout / 100.0f) * 100.0f);

				// Optional Failsafe: Ensure heavy taxation doesn't round them down to $0.00
				// if they were supposed to earn a few cents. 
				if (deflated_payout == 0 && taxable_income > 0) {
					deflated_payout = 100; // Give them at least $1.00
				}

				Log("TAXED: Original payout was a flat dollar. Snapping deflated payout to flat dollar: %s", FormatMoney(deflated_payout).c_str());
			}

			int amount_to_remove = taxable_income - deflated_payout;

			if (amount_to_remove > 0) {
				if (!g_transactionProcessed) {
					MONEY::_MONEY_DECREMENT_CASH_BALANCE(amount_to_remove);
					UIFEED::_UI_FEED_CLEAR_ALL_CHANNELS();

					float actual_earned = (float)deflated_payout / 100.0f;

					char ui_buffer[256];
					sprintf_s(ui_buffer, "+ %s", FormatMoney(deflated_payout).c_str());

					Log("TAXED: Bracket Tax Rate: %.0f%% | Mod Deduction: %s | Player Keeps: %s",
						(tax_rate * 100.0f), FormatMoney(amount_to_remove).c_str(), FormatMoney(deflated_payout).c_str());

					std::string feedIcon;
					if (actual_earned >= 100.0f) {
						feedIcon = "money_moneystack";
					}
					else if (actual_earned < 1.0f) {
						feedIcon = "money_coinstack";
					}
					else {
						feedIcon = "money_moneyclip";
					}

					DisplayItemFeedTicker(
						std::string(ui_buffer),
						"inventory_items",
						feedIcon,
						4000,
						"Transaction_Feed_Sounds",
						"Transaction_Positive",
						"COLOR_WHITE",
						0
					);

					Log("TAXED: Transaction complete. New wallet balance: %s", FormatMoney(MONEY::_MONEY_GET_CASH_BALANCE()).c_str());
					g_transactionProcessed = true;
				}
			}
		}
	}
	else {
		g_transactionProcessed = false;
	}

	g_prev_loop_balance = MONEY::_MONEY_GET_CASH_BALANCE();
}

void main_loop() {
	while (true) {
		update();
		// WAIT(0) is now global to catch all free-roam transactions instantly
		WAIT(0);
	}
}

void ScriptMain() {
	// Seed srand using chrono as well
	srand((unsigned int)GetTimeMS());

	g_prev_loop_balance = -1;
	g_escrow_balance = 0;
	g_transactionProcessed = false;
	InitSettings();
	main_loop();
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		scriptRegister(hInstance, ScriptMain);
	}
	return TRUE;
}
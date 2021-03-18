#include "SADXModLoader.h"
#include "mod-loader-common/ModLoaderCommon/IniFile.hpp"
#include <sstream>

constexpr int default_minutes{ 3 };
constexpr int default_seconds{ 3 };

struct level_time {
	level_time() = default;
	/// \param time_string minutes:seconds
	level_time(const std::string& time_string) {
		if (time_string.empty()) {
			return;
		}
		std::istringstream ss{ time_string };
		std::string value;
		std::getline(ss, value, ':');
		minutes = std::stoi(value);
		std::getline(ss, value, ':');
		seconds = std::stoi(value);
	}
	int minutes{ default_minutes };
	int seconds{ default_seconds };
};

struct stage {
	stage() = default;
	stage(const IniGroup* ini_group) {
		if (ini_group) {
			level_c = ini_group->getString("Level C");
			level_b = ini_group->getString("Level B");
			level_a = ini_group->getString("Level A");
		}
	}
	level_time level_c;
	level_time level_b;
	level_time level_a;
};

struct bosses {
	bosses() = default;
	bosses(const IniGroup* ini_group) {
		if (ini_group) {
			e101_beta = ini_group->getString("E-101 Beta");
			e101_mk2 = ini_group->getString("E-101 Mk II");
		}
	}
	level_time e101_beta;
	level_time e101_mk2;
};

struct config {
	stage final_egg;
	stage emerald_coast;
	stage windy_valley;
	stage red_mountain;
	stage hot_shelter;
	bosses bosses;
};

config cfg;

/// param minutes ignored
/// param seconds ignored
void apply_gamma_time(const char minutes, const char seconds) {
	// get current stage
	stage* stg{};
	switch (CurrentLevel) {
	case LevelIDs::LevelIDs_FinalEgg:
		stg = &cfg.final_egg;
		break;
	case LevelIDs::LevelIDs_EmeraldCoast:
		stg = &cfg.emerald_coast;
		break;
	case LevelIDs::LevelIDs_WindyValley:
		stg = &cfg.windy_valley;
		break;
	case LevelIDs::LevelIDs_RedMountain:
		stg = &cfg.red_mountain;
		break;
	case LevelIDs::LevelIDs_HotShelter:
		stg = &cfg.hot_shelter;
		break;
	case LevelIDs::LevelIDs_E101:
		TimeMinutes = cfg.bosses.e101_beta.minutes;
		TimeSeconds = cfg.bosses.e101_beta.seconds;
		return;
	case LevelIDs::LevelIDs_E101R:
		TimeMinutes = cfg.bosses.e101_mk2.minutes;
		TimeSeconds = cfg.bosses.e101_mk2.seconds;
		return;
	default:
		TimeMinutes = default_minutes;
		TimeSeconds = default_seconds;
		return;
	}
	// get current level
	struct emblems {
		bool c{ false };
		bool b{ false };
		bool a{ false };
	};
	emblems emblems;
	emblems.c = GetLevelEmblemCollected(
		&SaveFile,
		Characters::Characters_Gamma,
		CurrentLevel,
		2
	);
	emblems.b = GetLevelEmblemCollected(
		&SaveFile,
		Characters::Characters_Gamma,
		CurrentLevel,
		1
	);
	emblems.a = GetLevelEmblemCollected(
		&SaveFile,
		Characters::Characters_Gamma,
		CurrentLevel,
		0
	);
	// if c emblem is not unlocked, use timer for level c
	if (!emblems.c) {
		TimeMinutes = stg->level_c.minutes;
		TimeSeconds = stg->level_c.seconds;
	}
	// else, if b emblem is not unlocked, use timer for level b
	else if (!emblems.b) {
		TimeMinutes = stg->level_b.minutes;
		TimeSeconds = stg->level_b.seconds;
	}
	// else use timer for level a
	else {
		TimeMinutes = stg->level_a.minutes;
		TimeSeconds = stg->level_a.seconds;
	}
}

extern "C" {
	__declspec(dllexport) void __cdecl Init(
		const char* path,
		const HelperFunctions& helper_functions
	) {
		// replace call to SetTime with my function
		WriteCall(reinterpret_cast<void*>(0x414872), &apply_gamma_time);

		// load config file
		IniFile config_file(std::string{ path } + "\\config.ini");
		cfg = {
			config_file.getGroup("Final Egg"),
			config_file.getGroup("Emerald Coast"),
			config_file.getGroup("Windy Valley"),
			config_file.getGroup("Red Mountain"),
			config_file.getGroup("Hot Shelter"),
			config_file.getGroup("Bosses")
		};
	}

	__declspec(dllexport) ModInfo SADXModInfo { ModLoaderVer };
}

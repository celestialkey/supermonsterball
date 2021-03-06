#include "player.h"
#include "world.h"

using namespace std;


static uint32_t g_levelExperience[39] = {
	1000, 2000, 3000, 4000, 5000,
	6000, 7000, 8000, 10000, 12000,
	14000, 16000, 18000, 20000, 23000,
	26000, 30000, 35000, 40000, 45000,
	50000, 55000, 60000, 65000, 70000,
	75000, 80000, 85000, 90000, 95000,
	100000, 120000, 150000, 200000, 250000,
	300000, 430000, 600000, 800000
};

static vector<LevelUpItem> g_levelItems[39] = {
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 10)}, // Level 1
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 10)}, // Level 2
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_MEGA_SEED, 10)}, // Level 3
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20)}, // Level 4
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_MEGA_SEED, 20),
		LevelUpItem(ITEM_STANDARD_HEAL, 20)}, // Level 5
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_STANDARD_HEAL, 10)}, // Level 6
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_STANDARD_HEAL, 10)}, // Level 7
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_STANDARD_HEAL, 10),
		LevelUpItem(ITEM_MEGA_SEED, 10)}, // Level 8
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_STANDARD_HEAL, 10)}, // Level 9
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_SUPER_BALL, 20),
		LevelUpItem(ITEM_STANDARD_HEAL, 20), LevelUpItem(ITEM_MEGA_SEED, 20)}, // Level 10
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 10), LevelUpItem(ITEM_SUPER_BALL, 10),
		LevelUpItem(ITEM_STANDARD_HEAL, 10)}, // Level 11
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 10), LevelUpItem(ITEM_SUPER_BALL, 10),
		LevelUpItem(ITEM_STANDARD_HEAL, 10)}, // Level 12
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 10), LevelUpItem(ITEM_SUPER_BALL, 10),
		LevelUpItem(ITEM_STANDARD_HEAL, 10)}, // Level 13
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 10), LevelUpItem(ITEM_SUPER_BALL, 10),
		LevelUpItem(ITEM_STANDARD_HEAL, 10)}, // Level 14
	vector<LevelUpItem>{LevelUpItem(ITEM_STANDARD_BALL, 20), LevelUpItem(ITEM_SUPER_BALL, 20),
		LevelUpItem(ITEM_SUPER_HEAL, 20), LevelUpItem(ITEM_MEGA_SEED, 20)}, // Level 15
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 20), LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 16
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 20), LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 17
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 20), LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 18
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 20), LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 19
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 20), LevelUpItem(ITEM_UBER_BALL, 20),
		LevelUpItem(ITEM_SUPER_HEAL, 20), LevelUpItem(ITEM_MEGA_SEED, 20)}, // Level 20
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 10), LevelUpItem(ITEM_UBER_BALL, 10),
		LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 21
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 10), LevelUpItem(ITEM_UBER_BALL, 10),
		LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 22
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 10), LevelUpItem(ITEM_UBER_BALL, 10),
		LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 23
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 10), LevelUpItem(ITEM_UBER_BALL, 10),
		LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 24
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 20), LevelUpItem(ITEM_UBER_BALL, 20),
		LevelUpItem(ITEM_SUPER_HEAL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 20),
		LevelUpItem(ITEM_MEGA_SEED, 20)}, // Level 25
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 26
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 27
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 28
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_SUPER_HEAL, 10)}, // Level 29
	vector<LevelUpItem>{LevelUpItem(ITEM_SUPER_BALL, 30), LevelUpItem(ITEM_UBER_BALL, 30),
		LevelUpItem(ITEM_SUPER_HEAL, 30), LevelUpItem(ITEM_KEG_OF_HEALTH, 30),
		LevelUpItem(ITEM_MEGA_SEED, 20)}, // Level 30
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 31
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 32
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 33
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 34
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 50), LevelUpItem(ITEM_KEG_OF_HEALTH, 50),
		LevelUpItem(ITEM_MEGA_SEED, 20)}, // Level 35
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 36
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 37
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 20), LevelUpItem(ITEM_KEG_OF_HEALTH, 10)}, // Level 38
	vector<LevelUpItem>{LevelUpItem(ITEM_UBER_BALL, 100), LevelUpItem(ITEM_KEG_OF_HEALTH, 100),
		LevelUpItem(ITEM_MEGA_SEED, 20)} // Level 39
};

static PowerUpCost g_powerUpCost[39] = {
	PowerUpCost(1, 100), PowerUpCost(1, 100), PowerUpCost(1, 100), PowerUpCost(1, 100), PowerUpCost(1, 100),
	PowerUpCost(1, 200), PowerUpCost(1, 200), PowerUpCost(1, 200), PowerUpCost(1, 200), PowerUpCost(1, 200),
	PowerUpCost(2, 300), PowerUpCost(2, 300), PowerUpCost(2, 400), PowerUpCost(2, 400), PowerUpCost(2, 500),
	PowerUpCost(3, 500), PowerUpCost(3, 700), PowerUpCost(3, 700), PowerUpCost(3, 1000), PowerUpCost(3, 1000),
	PowerUpCost(4, 1500), PowerUpCost(4, 1500), PowerUpCost(4, 2000), PowerUpCost(4, 2000), PowerUpCost(4, 2500),
	PowerUpCost(5, 2500), PowerUpCost(5, 3000), PowerUpCost(5, 3000), PowerUpCost(5, 4000), PowerUpCost(5, 4000),
	PowerUpCost(6, 5000), PowerUpCost(6, 5000), PowerUpCost(7, 6000), PowerUpCost(7, 6000), PowerUpCost(8, 7000),
	PowerUpCost(8, 7000), PowerUpCost(9, 8000), PowerUpCost(9, 8000), PowerUpCost(9, 9000)
};

uint32_t Player::GetTotalExperienceNeededForCurrentLevel()
{
	uint32_t level = GetLevel();
	if (level < 1)
		level = 1;
	if (level >= 40)
		level = 40;
	uint32_t total = 0;
	for (uint32_t i = 0; i < (level - 1); i++)
		total += g_levelExperience[i];
	return total;
}


uint32_t Player::GetTotalExperienceNeededForNextLevel()
{
	uint32_t level = GetLevel();
	if (level < 1)
		level = 1;
	if (level >= 40)
		return GetTotalExperienceNeededForCurrentLevel();
	uint32_t total = 0;
	for (uint32_t i = 0; i < level; i++)
		total += g_levelExperience[i];
	return total;
}


vector<LevelUpItem> Player::GetItemsOnLevelUp(uint32_t level)
{
	if ((level >= 40) || (level == 0))
		return vector<LevelUpItem>{};
	return g_levelItems[level - 1];
}


PowerUpCost Player::GetPowerUpCost(uint32_t level)
{
	if ((level >= 40) || (level == 0))
		return PowerUpCost(0, 0);
	return g_powerUpCost[level - 1];
}


bool Player::IsMapTileTraversable(int32_t x, int32_t y)
{
	switch (GetMapTile(x, y))
	{
	case TILE_GRASS:
	case TILE_CITY:
	case TILE_DESERT:
		return true;
	default:
		return false;
	}
}


uint32_t Player::GetReputationRequirementForLevel(uint32_t level)
{
	switch (level)
	{
	case 1:
		return 0;
	case 2:
		return 2000;
	case 3:
		return 5000;
	case 4:
		return 10000;
	case 5:
		return 15000;
	case 6:
		return 20000;
	case 7:
		return 25000;
	case 8:
		return 30000;
	case 9:
		return 40000;
	default:
		return 50000;
	}
}


uint32_t Player::GetPitLevelByReputation(uint32_t reputation)
{
	for (uint32_t level = 10; level > 1; level--)
	{
		if (reputation >= GetReputationRequirementForLevel(level))
			return level;
	}
	return 1;
}


uint64_t Player::GetChallengeResponseValue(uint64_t challenge)
{
	challenge ^= 0xc0decafefeedfaceLL;
	uint64_t mix = (challenge * 25214903917LL) + 11;
	challenge = (challenge >> 17) | (challenge << 47);
	challenge += mix;
	return challenge;
}


uint32_t Player::GetEncounterValidationValue(int32_t x, int32_t y)
{
	uint64_t seed = (((uint64_t)x * 694847539LL) + ((uint64_t)y * 91939LL)) + 92893LL;
	return (uint32_t)(seed >> 16);
}

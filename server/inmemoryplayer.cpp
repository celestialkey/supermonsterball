//#define DEBUG_MAXED_PLAYER

#include <cstdlib>
#include <set>
#include "inmemoryplayer.h"
#include "world.h"

using namespace std;


InMemoryPlayer::InMemoryPlayer(const string& name): m_name(name)
{
	m_team = TEAM_UNASSIGNED;
#ifdef DEBUG_MAXED_PLAYER
	m_level = 40;
#else
	m_level = 5;
#endif
	m_xp = GetTotalExperienceNeededForCurrentLevel();
	m_powder = 0;
	m_x = SPAWN_X;
	m_y = SPAWN_Y;
	m_nextMonsterID = 1;

#ifdef DEBUG_MAXED_PLAYER
	m_inventory[ITEM_STANDARD_BALL] = 200;
	m_inventory[ITEM_SUPER_BALL] = 200;
	m_inventory[ITEM_UBER_BALL] = 200;
	m_inventory[ITEM_STANDARD_HEAL] = 200;
	m_inventory[ITEM_SUPER_HEAL] = 200;
	m_inventory[ITEM_KEG_OF_HEALTH] = 200;
	m_inventory[ITEM_MEGA_SEED] = 200;

	uint64_t id = 0xffff0000;
	for (auto& i : MonsterSpecies::GetAll())
	{
		shared_ptr<Monster> monster(new Monster(i, 0, 0, 0));
		monster->SetID(id++);
		monster->SetLevel(40);
		monster->SetIV(15, 15, 15);
		monster->SetSize(16);
		monster->SetCapture(true, ITEM_UBER_BALL);
		if ((monster->GetSpecies()->GetQuickMoves().size() != 0) && (monster->GetSpecies()->GetChargeMoves().size() != 0))
			monster->SetMoves(monster->GetSpecies()->GetQuickMoves()[0], monster->GetSpecies()->GetChargeMoves()[0]);
		monster->ResetHP();
		m_monsters.push_back(monster);

		m_seen[i->GetIndex()] = 1;
		m_captured[i->GetIndex()] = 1;
		m_treats[i->GetIndex()] = 2000;
	}
	m_powder = 1000000;
#else
	m_inventory[ITEM_STANDARD_BALL] = 20;
	m_inventory[ITEM_STANDARD_HEAL] = 20;
	m_inventory[ITEM_MEGA_SEED] = 10;
#endif
}


shared_ptr<Monster> InMemoryPlayer::GetMonsterByID(uint64_t id)
{
	for (auto& i : m_monsters)
	{
		if (i->GetID() == id)
			return i;
	}
	return nullptr;
}


uint32_t InMemoryPlayer::GetNumberCaptured(MonsterSpecies* species)
{
	auto i = m_captured.find(species->GetIndex());
	if (i == m_captured.end())
		return 0;
	return i->second;
}


uint32_t InMemoryPlayer::GetNumberSeen(MonsterSpecies* species)
{
	auto i = m_seen.find(species->GetIndex());
	if (i == m_seen.end())
		return 0;
	return i->second;
}


uint32_t InMemoryPlayer::GetTreatsForSpecies(MonsterSpecies* species)
{
	auto i = m_treats.find(species->GetBaseForm()->GetIndex());
	if (i == m_treats.end())
		return 0;
	return i->second;
}


uint32_t InMemoryPlayer::GetItemCount(ItemType type)
{
	auto i = m_inventory.find(type);
	if (i == m_inventory.end())
		return 0;
	return i->second;
}


bool InMemoryPlayer::UseItem(ItemType type)
{
	auto i = m_inventory.find(type);
	if (i == m_inventory.end())
		return false;
	if (i->second == 0)
		return false;
	i->second--;
	return true;
}


void InMemoryPlayer::ReportLocation(int32_t x, int32_t y)
{
	m_x = x;
	m_y = y;
}


vector<MonsterSighting> InMemoryPlayer::GetMonstersInRange()
{
	vector<SpawnPoint> spawns = World::GetWorld()->GetSpawnPointsInRange(m_x, m_y);
	vector<MonsterSighting> result;
	for (auto& i : spawns)
	{
		shared_ptr<Monster> monster = World::GetWorld()->GetMonsterAt(i.x, i.y, m_level);
		if (monster)
		{
			// Ensure the monster hasn't already been encountered
			bool valid = true;
			auto j = m_recentEncounters.find(monster->GetSpawnTime());
			if (j != m_recentEncounters.end())
			{
				for (auto& k : j->second)
				{
					if ((k->GetSpawnX() == i.x) && (k->GetSpawnY() == i.y))
					{
						// Already finished encounter with this one
						valid = false;
						break;
					}
				}
			}

			if (!valid)
				continue;

			MonsterSighting sighting;
			sighting.species = monster->GetSpecies();
			sighting.x = i.x;
			sighting.y = i.y;
			result.push_back(sighting);
		}
	}
	return result;
}


shared_ptr<Monster> InMemoryPlayer::StartWildEncounter(int32_t x, int32_t y)
{
	uint32_t dist = abs(x - m_x) + abs(y - m_y);
	if (dist > CAPTURE_RADIUS)
		return nullptr;

	m_encounter = World::GetWorld()->GetMonsterAt(x, y, m_level);
	m_encounter->SetID(m_nextMonsterID++);
	m_seedGiven = false;
	return m_encounter;
}


bool InMemoryPlayer::GiveSeed()
{
	if (!m_encounter)
		return false;
	if (m_seedGiven)
		return false;
	if (!UseItem(ITEM_MEGA_SEED))
		return false;
	m_seedGiven = true;
	return true;
}


void InMemoryPlayer::EndEncounter(bool caught, ItemType ball)
{
	if (!m_encounter)
		return;

	m_encounter->SetCapture(caught, ball);
	m_recentEncounters[m_encounter->GetSpawnTime()].push_back(m_encounter);

	// Clear out old encounters
	set<uint32_t> toDelete;
	for (auto& i : m_recentEncounters)
	{
		if (i.first < (m_encounter->GetSpawnTime() - 2))
			toDelete.insert(i.first);
	}
	for (auto i : toDelete)
	{
		m_recentEncounters.erase(i);
	}

	m_encounter.reset();
}


void InMemoryPlayer::EarnExperience(uint32_t xp)
{
	m_xp += xp;
	while ((m_level < 40) && (m_xp >= GetTotalExperienceNeededForNextLevel()))
	{
		for (auto& i : GetItemsOnLevelUp(m_level))
			m_inventory[i.type] += i.count;
		m_level++;
	}
}


BallThrowResult InMemoryPlayer::ThrowBall(ItemType type)
{
	if (!m_encounter)
		return THROW_RESULT_RUN_AWAY_AFTER_ONE;
	if ((type != ITEM_STANDARD_BALL) && (type != ITEM_SUPER_BALL) && (type != ITEM_UBER_BALL))
	{
		EndEncounter(false);
		return THROW_RESULT_RUN_AWAY_AFTER_ONE;
	}

	if (!UseItem(type))
	{
		EndEncounter(false);
		return THROW_RESULT_RUN_AWAY_AFTER_ONE;
	}

	BallThrowResult result = m_encounter->GetThrowResult(type, m_seedGiven);
	m_seedGiven = false;
	switch (result)
	{
	case THROW_RESULT_CATCH:
		if (GetNumberCaptured(m_encounter->GetSpecies()) == 0)
			EarnExperience(1400);
		else
			EarnExperience(400);
		m_seen[m_encounter->GetSpecies()->GetIndex()]++;
		m_captured[m_encounter->GetSpecies()->GetIndex()]++;
		m_treats[m_encounter->GetSpecies()->GetBaseForm()->GetIndex()] += 3;
		m_powder += 100;
		m_monsters.push_back(m_encounter);
		EndEncounter(true, type);
		break;
	case THROW_RESULT_RUN_AWAY_AFTER_ONE:
	case THROW_RESULT_RUN_AWAY_AFTER_TWO:
	case THROW_RESULT_RUN_AWAY_AFTER_THREE:
		m_seen[m_encounter->GetSpecies()->GetIndex()]++;
		EndEncounter(false, type);
		break;
	default:
		break;
	}

	return result;
}


void InMemoryPlayer::RunFromEncounter()
{
	EndEncounter(false);
}


bool InMemoryPlayer::PowerUpMonster(std::shared_ptr<Monster> monster)
{
	if (monster->GetLevel() >= GetLevel())
		return false;
	if (monster->GetLevel() >= 40)
		return false;

	if (GetTreatsForSpecies(monster->GetSpecies()) < GetPowerUpCost(monster->GetLevel()).treats)
		return false;
	if (GetPowder() < GetPowerUpCost(monster->GetLevel()).powder)
		return false;

	m_treats[monster->GetSpecies()->GetBaseForm()->GetIndex()] -= GetPowerUpCost(monster->GetLevel()).treats;
	m_powder -= GetPowerUpCost(monster->GetLevel()).powder;
	monster->PowerUp();
	return true;
}


bool InMemoryPlayer::EvolveMonster(std::shared_ptr<Monster> monster)
{
	if (monster->GetSpecies()->GetEvolutions().size() == 0)
		return false;
	if (GetTreatsForSpecies(monster->GetSpecies()) < monster->GetSpecies()->GetEvolutionCost())
		return false;

	m_treats[monster->GetSpecies()->GetBaseForm()->GetIndex()] -= monster->GetSpecies()->GetEvolutionCost();
	monster->Evolve();

	if (GetNumberCaptured(monster->GetSpecies()) == 0)
		EarnExperience(2000);
	else
		EarnExperience(1000);
	m_seen[monster->GetSpecies()->GetIndex()]++;
	m_captured[monster->GetSpecies()->GetIndex()]++;
	m_treats[monster->GetSpecies()->GetBaseForm()->GetIndex()]++;
	return true;
}


void InMemoryPlayer::TransferMonster(std::shared_ptr<Monster> monster)
{
	for (auto i = m_monsters.begin(); i != m_monsters.end(); ++i)
	{
		if ((*i)->GetID() == monster->GetID())
		{
			m_treats[monster->GetSpecies()->GetBaseForm()->GetIndex()]++;
			m_monsters.erase(i);
			break;
		}
	}
}


void InMemoryPlayer::SetMonsterName(std::shared_ptr<Monster> monster, const string& name)
{
	monster->SetName(name);
}


uint8_t InMemoryPlayer::GetMapTile(int32_t x, int32_t y)
{
	return World::GetWorld()->GetMapTile(x, y);
}


bool InMemoryPlayer::IsStopAvailable(int32_t x, int32_t y)
{
	if (GetMapTile(x, y) != TILE_STOP)
		return false;
	for (auto& i : m_recentStopsVisited)
	{
		if ((i.x == x) && (i.y == y) && ((time(NULL) - i.visitTime) < STOP_COOLDOWN))
			return false;
	}
	return true;
}


map<ItemType, uint32_t> InMemoryPlayer::GetItemsFromStop(int32_t x, int32_t y)
{
	if (!IsStopAvailable(x, y))
		return map<ItemType, uint32_t>();

	// Clear out old visits from recent list
	for (size_t i = 0; i < m_recentStopsVisited.size(); )
	{
		if ((m_recentStopsVisited[i].visitTime - time(NULL)) > STOP_COOLDOWN)
		{
			m_recentStopsVisited.erase(m_recentStopsVisited.begin() + i);
			continue;
		}
		i++;
	}

	map<ItemType, uint32_t> itemWeights;
	itemWeights[ITEM_STANDARD_BALL] = 20;
	itemWeights[ITEM_MEGA_SEED] = 3;
	if (GetLevel() >= 5)
		itemWeights[ITEM_STANDARD_HEAL] = 8;
	if (GetLevel() >= 10)
		itemWeights[ITEM_SUPER_BALL] = 7;
	if (GetLevel() >= 15)
		itemWeights[ITEM_SUPER_HEAL] = 3;
	if (GetLevel() >= 20)
		itemWeights[ITEM_UBER_BALL] = 3;
	if (GetLevel() >= 25)
		itemWeights[ITEM_KEG_OF_HEALTH] = 1;
	uint32_t totalWeight = 0;
	for (auto& i : itemWeights)
		totalWeight += i.second;

	map<ItemType, uint32_t> result;
	for (size_t count = 0; ; count++)
	{
		if (count >= 3)
		{
			if ((rand() % 10) > 3)
				break;
		}

		uint32_t value = rand() % totalWeight;
		uint32_t cur = 0;
		for (auto& i : itemWeights)
		{
			if (value < (cur + i.second))
			{
				result[i.first]++;
				break;
			}
			cur += i.second;
		}
	}

	for (auto& i : result)
		m_inventory[i.first] += i.second;

	// Add this visit to the recent list so that it can't be used until the cooldown expires
	RecentStopVisit visit;
	visit.x = x;
	visit.y = y;
	visit.visitTime = time(NULL);
	m_recentStopsVisited.push_back(visit);
	return result;
}


void InMemoryPlayer::SetTeam(Team team)
{
	m_team = team;
}


Team InMemoryPlayer::GetPitTeam(int32_t x, int32_t y)
{
	return World::GetWorld()->GetPitTeam(x, y);
}


uint32_t InMemoryPlayer::GetPitReputation(int32_t x, int32_t y)
{
	return World::GetWorld()->GetPitReputation(x, y);
}


vector<shared_ptr<Monster>> InMemoryPlayer::GetPitDefenders(int32_t x, int32_t y)
{
	return World::GetWorld()->GetPitDefenders(x, y);
}


bool InMemoryPlayer::AssignPitDefender(int32_t x, int32_t y, shared_ptr<Monster> monster)
{
	return World::GetWorld()->AssignPitDefender(x, y, m_team, monster);
}


bool InMemoryPlayer::StartPitBattle(int32_t x, int32_t y, vector<shared_ptr<Monster>> monsters)
{
	Team pitTeam = GetPitTeam(x, y);
	if ((x == PIT_OF_DOOM_X) && (y == PIT_OF_DOOM_Y))
	{
		if (m_level < 40)
		{
			printf("Player %s trying to battle Pit of Doom below level 40.\n", m_name.c_str());
			return false;
		}
	}
	else
	{
		if (pitTeam == TEAM_UNASSIGNED)
		{
			printf("Player %s trying to battle pit, but pit has no defenders.\n", m_name.c_str());
			return false;
		}
		if ((pitTeam == m_team) && (GetPitReputation(x, y) >= MAX_PIT_REPUTATION))
		{
			printf("Player %s trying to battle pit, but pit is already max reputation.\n", m_name.c_str());
			return false;
		}
	}

	if (m_team == TEAM_UNASSIGNED)
	{
		printf("Player %s trying to battle pit, but has no team.\n", m_name.c_str());
		return false;
	}
	if (monsters.size() == 0)
	{
		printf("Player %s trying to battle pit, but has no valid attackers.\n", m_name.c_str());
		return false;
	}

	set<uint64_t> seen;
	for (auto& i : monsters)
	{
		if (seen.count(i->GetID()) > 0)
			return false;
		if (i->GetCurrentHP() == 0)
			return false;
		if (i->IsDefending())
			return false;
		seen.insert(i->GetID());
	}

	vector<shared_ptr<Monster>> defenders = GetPitDefenders(x, y);
	if (defenders.size() == 0)
		return false;

	shared_ptr<PitBattle> battle(new PitBattle(monsters, defenders, pitTeam == m_team, x, y, nullptr));
	m_battle = battle;
	return true;
}


vector<shared_ptr<Monster>> InMemoryPlayer::GetPitBattleDefenders()
{
	if (m_battle)
		return m_battle->GetDefenders();
	return vector<shared_ptr<Monster>>();
}


void InMemoryPlayer::SetAttacker(shared_ptr<Monster> monster)
{
	if (m_battle)
		m_battle->SetAttacker(monster);
}


PitBattleStatus InMemoryPlayer::StepPitBattle()
{
	if (m_battle)
		return m_battle->Step();

	PitBattleStatus status;
	status.state = PIT_BATTLE_WAITING_FOR_ACTION;
	status.charge = 0;
	status.attackerHP = 0;
	status.defenderHP = 0;
	return status;
}


void InMemoryPlayer::SetPitBattleAction(PitBattleAction action)
{
	if (m_battle)
		m_battle->SetAction(action);
}


uint32_t InMemoryPlayer::EndPitBattle()
{
	if (!m_battle)
		return 0;

	uint32_t reputationChange = m_battle->GetReputationChange();
	if (reputationChange == 0)
		return 0;

	if (m_battle->IsTraining())
	{
		reputationChange = World::GetWorld()->AddPitReputation(m_battle->GetPitX(), m_battle->GetPitY(),
			reputationChange);
	}
	else
	{
		reputationChange = World::GetWorld()->RemovePitReputation(m_battle->GetPitX(), m_battle->GetPitY(),
			reputationChange);
	}

	EarnExperience(reputationChange / 2);
	return reputationChange;
}


void InMemoryPlayer::HealMonster(std::shared_ptr<Monster> monster, ItemType type)
{
	if ((type != ITEM_STANDARD_HEAL) && (type != ITEM_SUPER_HEAL) && (type != ITEM_KEG_OF_HEALTH))
		return;
	if (monster->GetCurrentHP() == monster->GetMaxHP())
		return;
	if (!UseItem(type))
		return;

	if (type == ITEM_STANDARD_HEAL)
		monster->Heal(20);
	else if (type == ITEM_SUPER_HEAL)
		monster->Heal(60);
	else if (type == ITEM_KEG_OF_HEALTH)
		monster->Heal(1000);
}


void InMemoryPlayer::TravelToPitOfDoom()
{
	m_x = SPAWN_X;
	m_y = SPAWN_Y;
}


string InMemoryPlayer::GetLevel40Flag()
{
	if (m_level < 40)
		return "You are not level 40!";
	return "Level 40 flag";
}


string InMemoryPlayer::GetCatchEmAllFlag()
{
	for (auto& i : MonsterSpecies::GetAll())
	{
		if (GetNumberCaptured(i) == 0)
			return "You need to go catch 'em all.";
	}
	return "Catch 'em all flag";
}

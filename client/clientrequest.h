#pragma once

#include "clientsocket.h"
#include "request.pb.h"
#include "player.h"
#include "world.h"

struct AllPlayerInfo
{
	uint32_t level, xp, powder, x, y;
	Team team;

	std::vector<std::shared_ptr<Monster>> monsters;
	std::map<uint32_t, uint32_t> seen;
	std::map<uint32_t, uint32_t> captured;
	std::map<uint32_t, uint32_t> treats;

	std::map<ItemType, uint32_t> inventory;
};

class ClientRequest
{
	static ClientRequest* m_requests;
	ClientSocket* m_ssl;
	uint64_t m_id;
	std::string m_name;
	uint64_t m_challenge;

	void WriteRequest(request::Request_RequestType type, const std::string& msg);
	std::string ReadResponse();

public:
	ClientRequest(ClientSocket* ssl);
	~ClientRequest();

	static ClientRequest* GetClient() { return m_requests; }
	uint64_t GetID() const { return m_id; }
	uint64_t GetChallenge() const { return m_challenge; }

	request::LoginResponse_AccountStatus Login(const std::string username, const std::string& password);
	request::RegisterResponse_RegisterStatus Register(const std::string username, const std::string& password);
	request::GetPlayerDetailsResponse GetPlayerDetails();
	std::vector<std::shared_ptr<Monster>> GetMonsterList();
	request::GetMonstersSeenAndCapturedResponse GetMonstersSeenAndCaptured();
	std::map<uint32_t, uint32_t> GetTreats();
	std::map<ItemType, uint32_t> GetInventory();
	std::vector<MonsterSighting> GetMonstersInRange(int32_t x, int32_t y);
	std::shared_ptr<Monster> StartEncounter(int32_t x, int32_t y);
	bool GiveSeed();
	BallThrowResult ThrowBall(ItemType ball, std::shared_ptr<Monster> monster);
	void RunFromEncounter();
	bool PowerUpMonster(std::shared_ptr<Monster> monster);
	bool EvolveMonster(std::shared_ptr<Monster> monster);
	void TransferMonster(std::shared_ptr<Monster> monster);
	void SetMonsterName(std::shared_ptr<Monster> monster, const std::string& name);
	void GetMapTiles(int32_t x, int32_t y, uint8_t* data);
	std::vector<RecentStopVisit> GetRecentStops();
	std::map<ItemType, uint32_t> GetItemsFromStop(int32_t x, int32_t y);
	void SetTeam(Team team);
	PitStatus GetPitStatus(int32_t x, int32_t y);
	bool AssignPitDefender(int32_t x, int32_t y, std::shared_ptr<Monster> monster);
	bool StartPitBattle(int32_t x, int32_t y, std::vector<std::shared_ptr<Monster>> monsters,
		std::vector<std::shared_ptr<Monster>>& defenders);
	void SetAttacker(std::shared_ptr<Monster> monster);
	PitBattleStatus StepPitBattle(std::vector<std::shared_ptr<Monster>> defenders);
	void SetPitBattleAction(PitBattleAction action);
	uint32_t EndPitBattle();
	void HealMonster(std::shared_ptr<Monster> monster, ItemType item, std::map<ItemType, uint32_t>& inventory);
	void TravelToPitOfDoom();
	std::string GetLevel40Flag();
	std::string GetCatchEmAllFlag();
	AllPlayerInfo GetAllPlayerInfo(bool provideChallengeResponse = false, uint64_t responseValue = 0);
};

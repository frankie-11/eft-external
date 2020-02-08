#pragma once
#include <string>
#include "math.hpp"
#include <vector>
#include <list>

#include "visuals.h"


#include <d3dx9math.h>
#pragma comment (lib, "d3dx9.lib")



// This is retarded but fuck it, deal with it, all objects should have their full struct
// in the namespace EFTStructs.
struct EFTOffsets
{
	static constexpr uint64_t offs_gameObjectManager = 0x15181E8;

	uint64_t gameObjectManager = 0x0;
	uint64_t gameWorld = 0x0;
	uint64_t localGameWorld = 0x0;
	uint64_t fpsCamera = 0x0;
	uint64_t OpticCamera = 0x0;

	struct
	{
		static constexpr uint64_t itemList = 0x48; //UPDATED 1/11/2020
		static constexpr uint64_t registeredPlayers = 0x68; //UPDATED 1/11/2020
		static constexpr uint64_t m_LocalPlayerID = 0x30;
	} localGameWorld_offsets;

	struct
	{
		static constexpr uint64_t length = 0x10; //good
		static constexpr uint64_t stringBase = 0x14; //good
	} unicodeString;

	struct
	{
		static constexpr uint64_t information = 0x28; //updated
		static constexpr uint64_t id = 0x10; //updated
	} profile;

	struct
	{
		static constexpr uint64_t playerName = 0x10; //updated
	} information;

	struct
	{
		static constexpr uint64_t angles_0 = 0x1D0;
		static constexpr uint64_t angles_1 = 0x1D8;
		static constexpr uint64_t position = 0x1E0;
	} movementContext;

	struct
	{
		static constexpr uint64_t m_pPlayerProfile = 0x03A0;
		static constexpr uint64_t movementContext = 0x38;
		static constexpr uint64_t proceduralWeaponAnimation = 0x70; //updayed 1/11/2020
		static constexpr uint64_t playerBody = 0x88; //updated 1/11/2020
		static constexpr uint64_t m_pHealthController = 0x3D0; //updated 1/11/2020
		static constexpr uint64_t profile = 0x3A0; //updayed 1/11/2020

	} Player;
};

namespace EFTStructs
{
	struct BaseObject
	{
		uint64_t previousObjectLink; //0x0000
		uint64_t nextObjectLink; //0x0008
		uint64_t object; //0x0010
	};

	struct GameObjectManager
	{
		uint64_t lastTaggedObject; //0x0000
		uint64_t taggedObjects; //0x0008
		uint64_t lastActiveObject; //0x0010
		uint64_t activeObjects; //0x0018
	}; //Size: 0x0010

	class ListInternal
	{
	public:
		char pad_0x0000[0x20]; //0x0000
		uintptr_t* firstEntry; //0x0020 
	}; //Size=0x0028

	class List
	{
	public:
		char pad_0x0000[0x10]; //0x0000
		ListInternal* listBase; //0x0010 
		__int32 itemCount; //0x0018 
	}; //Size=0x001C
}

struct EFTPlayer
{
	uintptr_t	 instance;
	FVector		 headPos;
	FVector		 location;
};

struct EFTExtract
{
	uintptr_t	 instance;
	std::string  name;
	FVector		 location;
};

/*
struct EFTLoot
{
	uintptr_t	instance;
	std::string name;
	Vector3		origin;
};*/


enum BodyParts : uint64_t
{
	Head = 0x20,
	Thorax = 0x28,
	Stomach = 0x30,
	LeftArm = 0x38,
	RightArm = 0x40,
	LeftLeg = 0x48,
	RightLeg = 0x50,
	value_max
};


class EFTData
{
public:
	static EFTData* Instance();

	bool InitOffsets();

	FVector GetPosition(uint64_t transform);

	bool Read();

	D3DXMATRIX viewMatrix;
	EFTPlayer localPlayer;
	std::vector<EFTPlayer> players;
	std::vector<EFTExtract> extracts;

	int          playercount;


	bool IsAiming(uint64_t instance);

	uint64_t get_mpcamera(uint64_t instance);
	D3DXMATRIX getoptic_matrix(uint64_t instance);
	bool open_extract(uint64_t extract);


	EFTOffsets offsets;

	bool IsPlayer(uint64_t instance);
private:
	uint64_t matrix_list_base = 0;
	uint64_t dependency_index_table_base = 0;

	uint64_t GetObjectFromList(uint64_t list, uint64_t lastObject, const char* objectName);
	uint64_t getbone_matrix(uint64_t instance);





	//std::vector<std::wstring> names;
};

enum Bones : int
{
	HumanBase = 0,
	HumanPelvis = 14,
	HumanLThigh1 = 15,
	HumanLThigh2 = 16,
	HumanLCalf = 17,
	HumanLFoot = 18,
	HumanLToe = 19,
	HumanRThigh1 = 20,
	HumanRThigh2 = 21,
	HumanRCalf = 22,
	HumanRFoot = 23,
	HumanRToe = 24,
	HumanSpine1 = 29,
	HumanSpine2 = 36,
	HumanSpine3 = 37,
	HumanLCollarbone = 89,
	HumanLUpperarm = 90,
	HumanLForearm1 = 91,
	HumanLForearm2 = 92,
	HumanLForearm3 = 93,
	HumanLPalm = 94,
	HumanRCollarbone = 110,
	HumanRUpperarm = 111,
	HumanRForearm1 = 112,
	HumanRForearm2 = 113,
	HumanRForearm3 = 114,
	HumanRPalm = 115,
	HumanNeck = 132,
	HumanHead = 133
};


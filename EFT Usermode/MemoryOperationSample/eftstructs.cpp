#include "eftstructs.h"
#include "xorstr.hpp"
#include "visuals.h"
#include <xmmintrin.h>  
#include <emmintrin.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include "Overlay.h"
#include "PMemHelper.h"


std::list<uint64_t> bodypart = { BodyParts::Head, BodyParts::Thorax, BodyParts::Stomach, BodyParts::LeftArm, BodyParts::RightArm, BodyParts::LeftLeg, BodyParts::RightLeg };

EFTData* EFTData::Instance()
{
	static EFTData instance;
	return &instance;
}

/* All one time initialization in here*/
bool EFTData::InitOffsets()
{
	this->offsets.gameObjectManager = memio->read<uint64_t>(memio->get_process_peb() + this->offsets.offs_gameObjectManager);
	

	//printf("GOM: 0x%X\n", this->offsets.gameObjectManager);

	// Read pointer to activeObjects and lastActiveObject with 1 read, then find game world and local game world.
	auto active_objects = memio->read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastActiveObject));
	if (!active_objects[0] || !active_objects[1])
		return false;

	//printf("ActiveObjects: 0x%X\n", active_objects);

	if (!(this->offsets.gameWorld = GetObjectFromList(active_objects[1], active_objects[0], _xor_("GameWorld"))))
	return false;

	//printf("this->offsets.gameWorld: 0x%X\n", this->offsets.gameWorld);

	// Find fps camera.
	this->offsets.localGameWorld = memio->ReadChain(this->offsets.gameWorld, { 0x30, 0x18, 0x28 });

	//printf("localgameWorld: 0x%X\n", this->offsets.localGameWorld);

	// Get tagged objects and find fps camera.
	auto tagged_objects = memio->read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastTaggedObject));
	if (!tagged_objects[0] || !tagged_objects[1])
		return false;

	if (!(this->offsets.fpsCamera = GetObjectFromList(tagged_objects[1], tagged_objects[0], _xor_("FPS Camera"))))
		return false;



	return true;
}



FVector EFTData::GetPosition(uint64_t transform)
{
	auto transform_internal = memio->read<uint64_t>(transform + 0x10);

	auto matrices = memio->read<uint64_t>(transform_internal + 0x38);
	auto index = memio->read<int>(transform_internal + 0x40);

	memio->readbuffer((uintptr_t)(matrices + 0x18), &matrix_list_base, sizeof(matrix_list_base));

	memio->readbuffer((uintptr_t)(matrices + 0x20), &dependency_index_table_base, sizeof(dependency_index_table_base));

	static auto get_dependency_index = [this](uint64_t base, int32_t index) 
	{
		memio->readbuffer((uintptr_t)(base + index * 4), &index, sizeof(index));
		return index;
	};

	static auto get_matrix_blob = [this](uint64_t base, uint64_t offs, float* blob, uint32_t size) {
		memio->readbuffer((uintptr_t)(base + offs), blob, size);
	};

	int32_t index_relation = get_dependency_index(dependency_index_table_base, index);

	FVector ret_value;
	{
		float* base_matrix3x4 = (float*)malloc(64),
			* matrix3x4_buffer0 = (float*)((uint64_t)base_matrix3x4 + 16),
			* matrix3x4_buffer1 = (float*)((uint64_t)base_matrix3x4 + 32),
			* matrix3x4_buffer2 = (float*)((uint64_t)base_matrix3x4 + 48);

		get_matrix_blob(matrix_list_base, index * 48, base_matrix3x4, 16);

		__m128 xmmword_1410D1340 = { -2.f, 2.f, -2.f, 0.f };
		__m128 xmmword_1410D1350 = { 2.f, -2.f, -2.f, 0.f };
		__m128 xmmword_1410D1360 = { -2.f, -2.f, 2.f, 0.f };

		while (index_relation >= 0) 
		{
			uint32_t matrix_relation_index = 6 * index_relation;

			// paziuret kur tik 3 nureadina, ten translationas, kur 4 = quatas ir yra rotationas.
			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index, matrix3x4_buffer2, 16);
			__m128 v_0 = *(__m128*)matrix3x4_buffer2;

			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index + 32, matrix3x4_buffer0, 16);
			__m128 v_1 = *(__m128*)matrix3x4_buffer0;

			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index + 16, matrix3x4_buffer1, 16);
			__m128i v9 = *(__m128i*)matrix3x4_buffer1;

			__m128* v3 = (__m128*)base_matrix3x4; // r10@1
			__m128 v10; // xmm9@2
			__m128 v11; // xmm3@2
			__m128 v12; // xmm8@2
			__m128 v13; // xmm4@2
			__m128 v14; // xmm2@2
			__m128 v15; // xmm5@2
			__m128 v16; // xmm6@2
			__m128 v17; // xmm7@2

			v10 = _mm_mul_ps(v_1, *v3);
			v11 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 0));
			v12 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 85));
			v13 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -114));
			v14 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -37));
			v15 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -86));
			v16 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 113));

			v17 = _mm_add_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v11, (__m128)xmmword_1410D1350), v13),
								_mm_mul_ps(_mm_mul_ps(v12, (__m128)xmmword_1410D1360), v14)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), -86))),
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v15, (__m128)xmmword_1410D1360), v14),
								_mm_mul_ps(_mm_mul_ps(v11, (__m128)xmmword_1410D1340), v16)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 85)))),
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v12, (__m128)xmmword_1410D1340), v16),
								_mm_mul_ps(_mm_mul_ps(v15, (__m128)xmmword_1410D1350), v13)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 0))),
						v10)),
				v_0);

			*v3 = v17;

			index_relation = get_dependency_index(dependency_index_table_base, index_relation);
		}

		ret_value = *(FVector*)base_matrix3x4;
		delete[] base_matrix3x4;
	}

	return ret_value;
}


bool EFTData::Read()
{
	this->players.clear();


	// Accumulate players.
	{

		uint64_t onlineusers = memio->read<uint64_t>(this->offsets.localGameWorld + this->offsets.localGameWorld_offsets.registeredPlayers);

		if (!onlineusers)
			return false;

		uint64_t list_base = memio->read<uint64_t>(onlineusers + offsetof(EFTStructs::List, listBase));
		int player_count = memio->read<int>(onlineusers + offsetof(EFTStructs::List, itemCount));


		if (player_count <= 0 || !list_base)
			return false;

		constexpr auto BUFFER_SIZE = 128;

		uint64_t player_buffer[BUFFER_SIZE];
		memio->readbuffer(list_base + offsetof(EFTStructs::ListInternal, firstEntry), player_buffer, sizeof(uint64_t) * player_count);
	
		EFTPlayer player;

		for (int i = 0; i < player_count; i++)
		{
			player.instance = player_buffer[i];
			this->playercount = player_count;


			uint64_t bone_matrix = this->getbone_matrix(player.instance);

			if (bone_matrix)
			{
		
				uint64_t bone = memio->ReadChain(bone_matrix, { 0x20, 0x10, 0x38 });
				player.location = memio->read<FVector>(bone + 0xB0);

				player.headPos = GetPosition(memio->read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanHead * 8))));
				

			}

			// Leave this at the end to have all the data.
			if (memio->read<int>(player.instance + 0x18))
			{
				this->localPlayer = player;
				this->localPlayer.location = player.location;
			}
	
			this->players.emplace_back(player);
		}
	}

	this->extracts.clear();
	// Accumulate Extractions.
	{
		uint64_t exit_controller = memio->read<uint64_t>(this->offsets.localGameWorld + 0x18);

		if (!exit_controller)
		return false;

		uint64_t exit_point = memio->read<uint64_t>(exit_controller + 0x20);
		int exit_count = memio->read<int>(exit_point + 0x18);

		if (exit_count <= 0 || !exit_point)
			return false;

		constexpr auto BUFFER_SIZE = 24;

		uint64_t extract_buffer[BUFFER_SIZE];
		memio->readbuffer(exit_point + offsetof(EFTStructs::ListInternal, firstEntry), extract_buffer, sizeof(uint64_t) * exit_count);

		EFTExtract extract;

		for (int i = 0; i < exit_count; i++)
		{
			extract.instance = extract_buffer[i];

			if (!EFTData::Instance()->open_extract(extract.instance))
				continue;

			uint64_t transform = memio->ReadChain(extract.instance, { 0x10, 0x30, 0x30, 0x8, 0x28 });
			extract.location = GetPosition(transform);

			uint64_t extract_name = memio->ReadChain(extract.instance, { 0x58, 0x10 });

			if (extract_name )
			{
				int32_t nameLength = memio->read<int32_t>(extract_name + this->offsets.unicodeString.length);
				extract.name = memio->GetUnicodeString(extract_name + this->offsets.unicodeString.stringBase, nameLength);
				
			}

			this->extracts.emplace_back(extract);
			
		}

	}
	// Get view matrix.
	{
		uint64_t temp = this->offsets.fpsCamera;
		if (!(temp = memio->read<uint64_t>(temp + 0x30)) || !(temp = memio->read<uint64_t>(temp + 0x18)))
		   return false;

	//	printf("temp: 0x%X\n", temp);
		D3DXMATRIX temp_matrix;
		memio->readbuffer(temp + 0x00D8, &temp_matrix, sizeof(temp_matrix));
		D3DXMatrixTranspose(&this->viewMatrix, &temp_matrix);
	}
	return true;
}


uint64_t EFTData::GetObjectFromList(uint64_t listPtr, uint64_t lastObjectPtr, const char* objectName)
{
	using EFTStructs::BaseObject;
	char name[256];
	uint64_t classNamePtr = 0x0;

	BaseObject activeObject = memio->read<BaseObject>(listPtr);
	BaseObject lastObject = memio->read<BaseObject>(lastObjectPtr);

	if (activeObject.object != 0x0)
	{
		while (activeObject.object != 0 && activeObject.object != lastObject.object)
		{
			classNamePtr = memio->read<uint64_t>(activeObject.object + 0x60);
			memio->readbuffer(classNamePtr + 0x0, &name, sizeof(name));
			if (strcmp(name, objectName) == 0)
			{
				return activeObject.object;
			}

			activeObject = memio->read<BaseObject>(activeObject.nextObjectLink);
		}
	}
	if (lastObject.object != 0x0)
	{
		classNamePtr = memio->read<uint64_t>(lastObject.object + 0x60);
		memio->readbuffer(classNamePtr + 0x0, &name, 256);
		if (strcmp(name, objectName) == 0)
		{
			return lastObject.object;
		}
	}

	return uint64_t();
}


bool EFTData::IsAiming(uint64_t	 instance)
{
	uint64_t m_pbreath = memio->ReadChain(instance, { this->offsets.Player.proceduralWeaponAnimation, 0x28});
	return memio->read<bool>(m_pbreath + 0x88);
}

uint64_t EFTData::get_mpcamera(uint64_t instance)
{
	static std::vector<uint64_t> temp{ this->offsets.Player.proceduralWeaponAnimation, 0x88, 0x20 };

	return memio->ReadChain(instance, temp);
}

uint64_t EFTData::getbone_matrix(uint64_t instance)
{
	static std::vector<uint64_t> temp{ this->offsets.Player.playerBody, 0x28, 0x28, 0x10 };

	return memio->ReadChain(instance, temp);
}

bool EFTData::IsPlayer(uint64_t instance)
{
	static std::vector<uint64_t> tempchain{ this->offsets.Player.profile, this->offsets.profile.information };

	uint64_t information = memio->ReadChain(instance, tempchain);

	if (memio->read<int32_t>(information + 0x54) != 0)
		return true;

	return false;
}

D3DXMATRIX EFTData::getoptic_matrix(uint64_t instance)
{
	D3DXMATRIX temp_matrix;
	D3DXMATRIX outmatrix;

	static std::vector<uint64_t> tempchain{ this->offsets.Player.proceduralWeaponAnimation,0x88, 0x20, 0x28, 0x30 };

	uint64_t temp = memio->ReadChain(instance, tempchain);

	//printf(_xor_("temp : 0x%X\n"), temp);
	memio->readbuffer(temp + 0x00D8, &temp_matrix, sizeof(temp_matrix));
	D3DXMatrixTranspose(&outmatrix, &temp_matrix);

	return outmatrix;
}


bool EFTData::open_extract(uint64_t extract)
{
	if (!extract)
		return false;

	int32_t status = memio->read<int32_t>(extract + 0xA8); /* NotOpen = 1// Imcpmpleterequirement = 2 // Countdown = 3 // Open = 4 // Pending = 5 // Awaot ,amial Activation = 6*/

	if (status == 4)
		return true;


	return false;
}


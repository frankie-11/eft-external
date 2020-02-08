#include "visuals.h"
#include "Overlay.h"
#include "xorstr.hpp"
#include "PMemHelper.h"
#include <string>
#include "math.hpp"

PRENDER* PRENDER::Instance()
{
	static PRENDER instance;
	return &instance;
}


void PRENDER::Render()
{

	auto& local_player = EFTData::Instance()->localPlayer;

	float distance;
	float distance2;
	float MaxDrawDistance = 300.f;
	D3DCOLOR color = 0;


	for (auto& extract : EFTData::Instance()->extracts)
	{
		if (!extract.instance)
			continue;

	
		distance2 = local_player.location.Distance(extract.location);

		D2D1_POINT_2F screen_pos;
		if (WorldToScreenv2(extract.location, screen_pos))
		{
			String((int)screen_pos.x, (int)screen_pos.y, D3DCOLOR_ARGB(255, 199, 45, 199), pFont, true, _xor_("%0.2fm").operator const char* (), distance2);
			String((int)screen_pos.x, (int)screen_pos.y + 13, D3DCOLOR_ARGB(199, 45, 236, 199), pFont, true, _xor_("%s").operator const char* (), extract.name);

		}
	}

	for (auto& player : EFTData::Instance()->players)
	{
		if (!player.instance)
			continue;

		if (player.instance == local_player.instance)
			continue;

		//EFTData::Instance()->DrawSkeleton(player.instance);

		distance = local_player.location.Distance(player.location);

		if (distance > MaxDrawDistance)
			continue;

		if (distance <= 150.f)
			color = D3DCOLOR_ARGB(255, 243, 77, 77); //color red, if less than 150m
		else if (distance > 150.f && distance <= 300.f)
			color = D3DCOLOR_ARGB(255, 130, 236, 23); //color yellow, if less than 300m and greater than 150m
		

		D2D1_POINT_2F screen_pos;
		if (WorldToScreenv2(player.location, screen_pos))
		{
			String((int)screen_pos.x, (int)screen_pos.y, color, pFont, true, _xor_("%0.2fm").operator const char* (), distance);

			if(EFTData::Instance()->IsPlayer(player.instance))
			String((int)screen_pos.x, (int)screen_pos.y + 13, D3DCOLOR_ARGB(250, 255, 91, 91), pFont2, true, _xor_("3").operator const char* ());
		}

		if (WorldToScreenv2(player.headPos, screen_pos))
		{
			Circle((int)screen_pos.x, (int)screen_pos.y, 4, 0, 1, true, 32, D3DCOLOR_ARGB(255, 255, 255, 255));
		}
		
	}
}



bool PRENDER::WorldToScreenv2(const FVector& point3D, D2D1_POINT_2F& point2D)
{
	D3DXVECTOR3 _point3D = D3DXVECTOR3(point3D.x, point3D.z, point3D.y);

	auto& matrix = EFTData::Instance()->viewMatrix;

	if (EFTData::Instance()->IsAiming(EFTData::Instance()->localPlayer.instance) && EFTData::Instance()->get_mpcamera(EFTData::Instance()->localPlayer.instance))
	{
    	 matrix = EFTData::Instance()->getoptic_matrix(EFTData::Instance()->localPlayer.instance);
	}

	D3DXVECTOR3 translationVector = D3DXVECTOR3(matrix._41, matrix._42, matrix._43);
	D3DXVECTOR3 up = D3DXVECTOR3(matrix._21, matrix._22, matrix._23);
	D3DXVECTOR3 right = D3DXVECTOR3(matrix._11, matrix._12, matrix._13);

	float w = D3DXVec3Dot(&translationVector, &_point3D) + matrix._44;

	if (w < 0.098f)
		return false;

	float y = D3DXVec3Dot(&up, &_point3D) + matrix._24;
	float x = D3DXVec3Dot(&right, &_point3D) + matrix._14;


	if (EFTData::Instance()->IsAiming(EFTData::Instance()->localPlayer.instance) && EFTData::Instance()->get_mpcamera(EFTData::Instance()->localPlayer.instance))
	{
		uint64_t chain = memio->ReadChain(EFTData::Instance()->offsets.fpsCamera, { 0x30, 0x18 });

			x /= memio->read<float>(chain + 0x12C);

			if(x < 2.f)
			x /= memio->read<float>(chain + 0xAC);

			y /= memio->read<float>(chain + 0x118);

			if(y < 2.f)
			y /= memio->read<float>(chain + 0x98);
	}

	point2D.x = (1920 / 2) * (1.f + x / w);
	point2D.y = (1080 / 2) * (1.f - y / w);

	return true;
}


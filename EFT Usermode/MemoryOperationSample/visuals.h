#pragma once
#include <Windows.h>
#include <d2d1.h>
#include "math.hpp"

#include "eftstructs.h"
#include <list>

class PRENDER
{
public:
	static PRENDER* Instance();

	void Render();
	bool WorldToScreenv2(const FVector& point3D, D2D1_POINT_2F& point2D);
private:
	
};


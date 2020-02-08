#include "math.hpp"
#include <cmath>
#include <cfloat>

#define M_PI       3.14159265358979323846   // pi

float DegToRad(float x)
{
	return x * ((float)M_PI / 180.f);
}

float RadToDeg(float x)
{
	return x * (180.f / (float)M_PI);
}

float DistancePointToLine(FVector Point, FVector LineOrigin, FVector Dir)
{
	FVector PointDir = Point - LineOrigin;

	float TempOffset = (PointDir * Dir) / Dir.GetMagnitudeSqr(); // Calculates the Offset for: LineOrigin + TempOffset*Dir; - if Dir is already normalized you can skip the dividing
	if (TempOffset < 0.000001f)
		return FLT_MAX;

	FVector PerpendicularPoint = LineOrigin + (Dir * TempOffset);

	return (Point - PerpendicularPoint).GetLength();
}

Vector4::Vector4()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 1.0f;
	pointer = false;
}

Vector4::Vector4(float x, float y, float z, float w)
{
	this->x = x; this->y = y; this->z = z; this->w = w;
	pointer = false;
}

Vector4::Vector4(float x, float y, float z)
{
	this->x = x; this->y = y; this->z = z; this->w = 1.0f;
	pointer = false;
}

Vector4::Vector4(float* x, float* y, float* z, float* w)
{
	this->px = x; this->py = y; this->pz = z; this->pw = w;
	this->x = *x; this->y = *y; this->z = *z; this->w = *w;
	pointer = true;
}

Vector4 Vector4::operator= (const Vector4& v)
{
	if (pointer)
	{
		*px = x = v.x;
		*py = y = v.y;
		*pz = z = v.z;
		*pw = w = v.w;
	}
	else
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}
}

const Vector4 Vector4::operator* (const float& scalar) const
{
	return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

const Vector4 Vector4::operator+ (const Vector4& v) const
{
	Vector4 vec;
	vec.x = this->x + v.x;
	vec.y = this->y + v.y;
	vec.z = this->z + v.z;
	vec.w = this->w + v.w;
	return vec;
}

float Vector3::Distance(Vector3 v)
{
	return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
}

Vector3 Vector3::operator+(Vector3 v)
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

float Vector3::Dot(Vector3 v)
{
	return x * v.x + y * v.y + z * v.z;
}

Vector3 Vector3::operator-(Vector3 v)
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

FVector::FVector() : x(0.f), y(0.f), z(0.f) {}

FVector::FVector(float x, float y, float z) : x(x), y(y), z(z) {}

FVector::FVector(const FVector& other) : x(other.x), y(other.y), z(other.z) {}

FVector FVector::operator+(const FVector& other) const
{
	return FVector(x + other.x, y + other.y, z + other.z);
}

FVector FVector::operator-(const FVector& other) const
{
	return FVector(x - other.x, y - other.y, z - other.z);
}

FVector FVector::operator*(const float other) const
{
	return FVector(x * other, y * other, z * other);
}

float FVector::operator*(const FVector& other) const
{
	return (x * other.x + y * other.y + z * other.z);
}

bool FVector::operator==(const FVector& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

bool FVector::operator!=(const FVector& other) const
{
	return x != other.x || y != other.y || z != other.z;
}

FVector& FVector::operator=(const FVector& other)
{
	x = other.x;
	y = other.y;
	z = other.z;

	return *this;
}

FVector& FVector::operator+=(const FVector& other)
{
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

FVector& FVector::operator-=(const FVector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

FVector& FVector::operator*=(const float other)
{
	x *= other;
	y *= other;
	z *= other;

	return *this;
}

float& FVector::operator[](size_t i)
{
	return reinterpret_cast<float*>(this)[i];
}

const float& FVector::operator[](size_t i) const
{
	return reinterpret_cast<const float*>(this)[i];
}

float FVector::GetLength() const
{
	return sqrt(x * x + y * y + z * z);
}

float FVector::Distance(FVector v)
{
	return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
}

float FVector::Distance2(FVector a, FVector b)

{

	float diff_x = a.x - b.x;

	float diff_y = a.y - b.y;

	float diff_z = a.z - b.z;

	return float(sqrtf(diff_x * diff_x + diff_y * diff_y + diff_z * diff_z));

}
float FVector::GetMagnitudeSqr()
{
	return (x * x + y * y + z * z);
}

FRotator FVector::VectorAngles() const
{
	return FRotator(atan2(-x, z), atan2(y, sqrt(x * x + z * z)), 0.f);
}

FRotator::FRotator() : pitch(0.f), yaw(0.f), roll(0.f) {}

FRotator::FRotator(float pitch, float yaw, float roll) : pitch(pitch), yaw(yaw), roll(roll) {}

FRotator::FRotator(const FRotator& other) : pitch(other.pitch), yaw(other.yaw), roll(other.roll) {}

void FRotator::ToSourceAngles()
{
	yaw = -yaw + 90.f;
	Normalize();
}

void FRotator::ToUnityAngles()
{
	ToSourceAngles(); // Toggle behaviour.
}

void FRotator::Normalize()
{
	yaw = remainder(yaw, static_cast<float>(2 * M_PI));
	pitch = remainder(pitch, static_cast<float>(2 * M_PI));
}

FVector FRotator::AngleVector()
{
	float sp, sy, cp, cy;

	sy = sin(yaw / 180.f * (float)M_PI);
	cy = cos(yaw / 180.f * (float)M_PI);

	sp = sin(pitch / 180.f * (float)M_PI);
	cp = cos(pitch / 180.f * (float)M_PI);

	return FVector(cp * cy, cp * sy, -sp);
}

void FRotator::AngleVectors(FVector* x, FVector* y, FVector* z)
{
	float radPitch = (pitch * static_cast<float>(M_PI / 180.f));
	float radYaw = (yaw * static_cast<float>(M_PI / 180.f));
	float radRoll = (roll * static_cast<float>(M_PI / 180.f));

	float SP = sin(radPitch);
	float CP = cos(radPitch);
	float SY = sin(radYaw);
	float CY = cos(radYaw);
	float SR = sin(radRoll);
	float CR = cos(radRoll);


	x->x = CP * CY;
	x->y = CP * SY;
	x->z = SP;

	y->x = SR * SP * CY - CR * SY;
	y->y = SR * SP * SY + CR * CY;
	y->z = -SR * CP;

	z->x = -(CR * SP * CY + SR * SY);
	z->y = CY * SR - CR * SP * SY;
	z->z = CR * CP;
}

FQuat FQuat::operator*(const FQuat& other)
{
	FQuat ret;

	auto& q1 = *this;
	auto& q2 = other;

	ret.x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
	ret.y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
	ret.z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
	ret.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;

	return ret;
}
//
//FMatrix FMatrix::operator*(const FMatrix & other)
//{
//	FMatrix result;
//	for (int i = 0; i < 4; i++)
//	{
//		for (int j = 0; j < 4; j++)
//		{
//			result[i][j] = 0;
//
//			for (int k = 0; k < 4; k++)
//			{
//				result[i][j] += (*this)[i][k] * other[k][j];
//			}
//		}
//	}
//
//	return result;
//}

FVector FMatrix::operator*(const FVector& vec)
{
	/*
	FVector pOut;
	int a[3][3] = { { 2,4,3 },{ 1,5,7 },{ 0,2,3 } };
	int b[] = { 2,5,6 };
	FVector out;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out[i] += (a[i][j] * b[j]);
		}
	}

	return pOut;
	*/


	FMatrix m;

	m[3][0] = vec.x;
	m[3][1] = vec.y;
	m[3][2] = vec.z;

	m[0][0] = 1.f;
	m[1][1] = 1.f;
	m[2][2] = 1.f;


	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;

	auto result = m * (*this);

	return FVector(result[3][0], result[3][1], result[3][2]);

}

FMatrix FMatrix::operator*(const FMatrix& pM2)
{
	auto pM1 = *this;

	FMatrix pOut;
	pOut[0][0] = pM1[0][0] * pM2[0][0] + pM1[0][1] * pM2[1][0] + pM1[0][2] * pM2[2][0] + pM1[0][3] * pM2[3][0];
	pOut[0][1] = pM1[0][0] * pM2[0][1] + pM1[0][1] * pM2[1][1] + pM1[0][2] * pM2[2][1] + pM1[0][3] * pM2[3][1];
	pOut[0][2] = pM1[0][0] * pM2[0][2] + pM1[0][1] * pM2[1][2] + pM1[0][2] * pM2[2][2] + pM1[0][3] * pM2[3][2];
	pOut[0][3] = pM1[0][0] * pM2[0][3] + pM1[0][1] * pM2[1][3] + pM1[0][2] * pM2[2][3] + pM1[0][3] * pM2[3][3];
	pOut[1][0] = pM1[1][0] * pM2[0][0] + pM1[1][1] * pM2[1][0] + pM1[1][2] * pM2[2][0] + pM1[1][3] * pM2[3][0];
	pOut[1][1] = pM1[1][0] * pM2[0][1] + pM1[1][1] * pM2[1][1] + pM1[1][2] * pM2[2][1] + pM1[1][3] * pM2[3][1];
	pOut[1][2] = pM1[1][0] * pM2[0][2] + pM1[1][1] * pM2[1][2] + pM1[1][2] * pM2[2][2] + pM1[1][3] * pM2[3][2];
	pOut[1][3] = pM1[1][0] * pM2[0][3] + pM1[1][1] * pM2[1][3] + pM1[1][2] * pM2[2][3] + pM1[1][3] * pM2[3][3];
	pOut[2][0] = pM1[2][0] * pM2[0][0] + pM1[2][1] * pM2[1][0] + pM1[2][2] * pM2[2][0] + pM1[2][3] * pM2[3][0];
	pOut[2][1] = pM1[2][0] * pM2[0][1] + pM1[2][1] * pM2[1][1] + pM1[2][2] * pM2[2][1] + pM1[2][3] * pM2[3][1];
	pOut[2][2] = pM1[2][0] * pM2[0][2] + pM1[2][1] * pM2[1][2] + pM1[2][2] * pM2[2][2] + pM1[2][3] * pM2[3][2];
	pOut[2][3] = pM1[2][0] * pM2[0][3] + pM1[2][1] * pM2[1][3] + pM1[2][2] * pM2[2][3] + pM1[2][3] * pM2[3][3];
	pOut[3][0] = pM1[3][0] * pM2[0][0] + pM1[3][1] * pM2[1][0] + pM1[3][2] * pM2[2][0] + pM1[3][3] * pM2[3][0];
	pOut[3][1] = pM1[3][0] * pM2[0][1] + pM1[3][1] * pM2[1][1] + pM1[3][2] * pM2[2][1] + pM1[3][3] * pM2[3][1];
	pOut[3][2] = pM1[3][0] * pM2[0][2] + pM1[3][1] * pM2[1][2] + pM1[3][2] * pM2[2][2] + pM1[3][3] * pM2[3][2];
	pOut[3][3] = pM1[3][0] * pM2[0][3] + pM1[3][1] * pM2[1][3] + pM1[3][2] * pM2[2][3] + pM1[3][3] * pM2[3][3];

	return pOut;
}



FMatrix FTransform::ToMatrixWithScale()
{
	FMatrix OutMatrix = {};

	OutMatrix[3][0] = Translation.x;
	OutMatrix[3][1] = Translation.y;
	OutMatrix[3][2] = Translation.z;

	float x2 = Rotation.x + Rotation.x;
	float y2 = Rotation.y + Rotation.y;
	float z2 = Rotation.z + Rotation.z;
	{
		float xx2 = Rotation.x * x2;
		float yy2 = Rotation.y * y2;
		float zz2 = Rotation.z * z2;
		OutMatrix[0][0] = (1.0f - (yy2 + zz2)) * Scale3D.x;
		OutMatrix[1][1] = (1.0f - (xx2 + zz2)) * Scale3D.y;
		OutMatrix[2][2] = (1.0f - (xx2 + yy2)) * Scale3D.z;
	}
	{
		float yz2 = Rotation.y * z2;
		float wx2 = Rotation.w * x2;

		OutMatrix[2][1] = (yz2 - wx2) * Scale3D.z;
		OutMatrix[1][2] = (yz2 + wx2) * Scale3D.y;
	}
	{
		float xy2 = Rotation.x * y2;
		float wz2 = Rotation.w * z2;

		OutMatrix[1][0] = (xy2 - wz2) * Scale3D.y;
		OutMatrix[0][1] = (xy2 + wz2) * Scale3D.x;
	}
	{
		float xz2 = Rotation.x * z2;
		float wy2 = Rotation.w * y2;

		OutMatrix[2][0] = (xz2 + wy2) * Scale3D.z;
		OutMatrix[0][2] = (xz2 - wy2) * Scale3D.x;
	}

	OutMatrix[0][3] = 0.0f;
	OutMatrix[1][3] = 0.0f;
	OutMatrix[2][3] = 0.0f;
	OutMatrix[3][3] = 1.0f;

	return OutMatrix;
}

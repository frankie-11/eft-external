#pragma once
// Libraries
#include <math.h>
#include <assert.h>

class FVector;
class FRotator;
class Vector3;

float DegToRad(float x);
float RadToDeg(float x);
float DistancePointToLine(FVector Point, FVector LineOrigin, FVector Dir);



    // Class Vector2
    // This class represents a 2D vector.
    class Vector2 {

    public:

        // -------------------- Attributes -------------------- //

        // Components of the vector
        float x, y;


        // -------------------- Methods -------------------- //

        // Constructor
        Vector2(float x = 0, float y = 0) : x(x), y(y) {}

        // Constructor
        Vector2(const Vector2& vector) : x(vector.x), y(vector.y) {}

        // + operator
        Vector2 operator+(const Vector2& v) const {
            return Vector2(x + v.x, y + v.y);
        }

        // += operator
        Vector2& operator+=(const Vector2& v) {
            x += v.x; y += v.y;
            return *this;
        }

        // - operator
        Vector2 operator-(const Vector2& v) const {
            return Vector2(x - v.x, y - v.y);
        }

        // -= operator
        Vector2& operator-=(const Vector2& v) {
            x -= v.x; y -= v.y;
            return *this;
        }

        // = operator
        Vector2& operator=(const Vector2& vector) {
            if (&vector != this) {
                x = vector.x;
                y = vector.y;
            }
            return *this;
        }

        // == operator
        bool operator==(const Vector2& v) const {
            return x == v.x && y == v.y;
        }

        // * operator
        Vector2 operator*(float f) const {
            return Vector2(f * x, f * y);
        }

        // *= operator
        Vector2& operator*=(float f) {
            x *= f; y *= f;
            return *this;
        }

        // / operator
        Vector2 operator/(float f) const {
            assert(f != 0);
            float inv = 1.f / f;
            return Vector2(x * inv, y * inv);
        }

        // /= operator
        Vector2& operator/=(float f) {
            assert(f != 0);
            float inv = 1.f / f;
            x *= inv; y *= inv;
            return *this;
        }

        // - operator
        Vector2 operator-() const {
            return Vector2(-x, -y);
        }

        // [] operator
        float& operator[](int i) {
            assert(i >= 0 && i <= 1);
            switch (i) {
            case 0: return x;
            case 1: return y;
            }
            return y;
        }

        // Normalize the vector and return it
        Vector2 normalize() {
            float l = length();
            assert(l > 0);
            x /= l;
            y /= l;
            return *this;
        }

        // Clamp the vector values between 0 and 1
        Vector2 clamp01() {
            if (x > 1.f) x = 1.f;
            else if (x < 0.f) x = 0.f;
            if (y > 1.f) y = 1.f;
            else if (y < 0.f) y = 0.f;
            return *this;
        }

        // Return the squared length of the vector
        float lengthSquared() const { return x * x + y * y; }

        // Return the length of the vector
        float length() const { return sqrt(lengthSquared()); }
    };


//Vector3
class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	float x;
	float y;
	float z;

	float Dot(Vector3 v);
	Vector3 operator+(Vector3 v);
	Vector3 operator-(Vector3 v);
	float Distance(Vector3 v);
};

class Vector4
{
public:
    float x, z, y;

	Vector4();
	Vector4(float x, float y, float z, float w);
	Vector4(float x, float y, float z);
	Vector4(float* x, float* y, float* z, float* w);
	~Vector4() {}

	Vector4 operator= (const Vector4& v);
	const Vector4 operator* (const float& scalar) const;
	const Vector4 operator+ (const Vector4& v) const;

	//inline void show() { std::cout << this->x << " " << this->y << " " << this->z << " " << this->w << std::endl; }

private:

	float w;

	// only to allow do that: m(0) = Vector4(1,1,1,1) //
	float* px;
	float* py;
	float* pz;
	float* pw;

	bool pointer;  // to check what constructor was called //
};

class FVector
{
public:
	float x, z, y;

	FVector();
	FVector(float x, float y, float z);
	FVector(const FVector& other);

	FVector operator+ (const FVector& other) const;
	FVector operator- (const FVector& other) const;
	FVector operator* (const float other) const;
	float operator* (const FVector& other) const;

	bool operator == (const FVector& other) const;
	bool operator != (const FVector& other) const;

	FVector& operator= (const FVector& other);
	FVector& operator+= (const FVector& other);
	FVector& operator-= (const FVector& other);
	FVector& operator*= (const float other);

	float& operator[](size_t i);
	const float& operator[](size_t i) const;

	float GetLength() const;
    float Distance(FVector v);
    float Distance2(FVector a, FVector b);
    float GetMagnitudeSqr();

	FRotator VectorAngles() const;
};

class FMatrix
{
public:
	FMatrix() : m{
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f } }
	{
	}

	FMatrix(const FMatrix&) = default;


	float* operator[](size_t i) { return m[i]; }
	const float* operator[](size_t i) const { return m[i]; }


	FVector operator*(const FVector& vec);
	FMatrix operator*(const FMatrix& other);
	float m[4][4];
};

class FRotator
{
public:
	float yaw;
	float pitch;
	float roll;

	FRotator();
	FRotator(float pitch, float yaw, float roll);
	FRotator(const FRotator& other);

	void ToSourceAngles();
	void ToUnityAngles();
	void Normalize();
	FVector AngleVector();
	void AngleVectors(FVector* x, FVector* y, FVector* z);
};

struct FQuat
{
	float x;
	float y;
	float z;
	float w;

	FQuat operator*(const FQuat& other);
};

struct FTransform
{
public:
	FQuat Rotation;
	FVector Translation;
private:
	float pad0;
public:
	FVector Scale3D;
private:
	float pad1;

public:
	FMatrix ToMatrixWithScale();

};

struct FBoxSphereBounds
{
	FVector	Origin;
	FVector BoxExtent;
	float SphereRadius;
};

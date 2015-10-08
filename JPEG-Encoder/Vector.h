#pragma once
#include <memory>

class Vector;
typedef std::shared_ptr<Vector> VectorPtr;

class Vector
{
	float data[3];

public:
	Vector();
	Vector(float v1, float v2, float v3);
	~Vector();

	Vector& operator+=(const Vector& vec);

	inline float operator[](int idx) const
	{
		return data[idx];
	}
};

typedef Vector Pixel;
typedef VectorPtr PixelPtr;
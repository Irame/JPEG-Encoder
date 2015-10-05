#pragma once
class Vector
{
	float data[3];

public:
	Vector();
	Vector(float v1, float v2, float v3);
	~Vector();

	Vector& operator+=(const Vector& vec);

	const float* getData() const;
};
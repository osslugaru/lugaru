#ifndef FRUSTUM_H
#define FRUSTUM_H

class FRUSTUM {
public:
	float frustum[6][4];
	void GetFrustum();
	int CubeInFrustum(float, float, float, float);
	int CubeInFrustum(float, float, float, float, float);
	int SphereInFrustum(float, float, float, float);
};

#endif
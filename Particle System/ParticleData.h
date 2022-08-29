/*
Contains definitions for particle and vertex data structures.
*/

#ifndef PARTICLE_H
#define PARTICLE_H

#include <d3dx9.h>		// Direct 3D library (for all Direct 3D funtions).

// data structure for a particle
struct Particle
{
	int			id_;			// used to distinguish between particles belonging to different subsystems
	int			lifetime_;		// how many frames is the particle rendered
	D3DXVECTOR3 position_;		// the current position of the particle
	D3DXVECTOR3 origin_;		// the origin of the particle (the position where it was originally created)
	D3DXVECTOR3 velocity_;		// the velocity of the particle
	D3DXCOLOR	colour_;		// the current colour of the particle
	D3DXVECTOR3	acceleration_;	// acceleration of the particle (calculated from the velocity taking environmental influence into account)
	float		time_;			// how long the particle is alive
	float		size_;			// the size of the particle
};

// A structure for point sprites.
struct POINTVERTEX
{
    D3DXVECTOR3 position_;		// X, Y, Z position of the point (sprite).
	DWORD color_;				// the colour of the particle
};

// The structure of a vertex in our vertex buffer...
#define D3DFVF_POINTVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

#define SAFE_DELETE(p)       {if(p) {delete (p);     (p)=NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p) {delete[] (p);   (p)=NULL;}}
#define SAFE_RELEASE(p)      {if(p) {(p)->Release(); (p)=NULL;}}

#define reset_particle(p) SecureZeroMemory(&p, sizeof(Particle));

#endif
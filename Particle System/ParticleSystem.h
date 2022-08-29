/*
The base class for all particle systems. Mostly copied from the blackboard example
*/

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <d3dx9.h>		// Direct 3D library (for all Direct 3D funtions).
#include <vector>
#include <algorithm>
#include <functional>
#include "ParticleData.h"
#include "Helpers.h"

class ParticleSystem
{
public:
	int maxParticles_;						// The maximum number of particles in this particle system.
	int startParticles_;					// Number of particles to start in each batch.

	int particlesAlive_;					// The number of particles that are currently alive.
	int maxLifetime_;					    // The start age of each particle (count down from this, kill particle when zero).

	LPDIRECT3DTEXTURE9 particleTexture_;	// The texture for the points.				
	D3DXVECTOR3 origin_;					// Vectors for origin of the particle system.

	int startTimer_;						// Count-down timer, start another particle when zero.		
	int startInterval_;		     			// Interval between starting a new particle (used to initialise 'start_timer_').
	float timeIncrement_;					// Used to increase the value of 'time'for each particle - used to calculate vertical position.

	float maxParticleSize_;					// Size of the point.

	ParticleSystem(void);
	~ParticleSystem(void);
	HRESULT initialise(LPDIRECT3DDEVICE9 device);
	virtual void update(void) = 0;			// Specific implementations to provide this - this is to update the positions of the particles.
	virtual void render(void);								

private:
	
	class isParticleDead			// This is a private class, only available inside 'PARTICLE_SYSTEM_BASE' - functor to determine if a particle is alive or dead.
	{	
		public:	
			bool operator()(const Particle &p)
			{
				return p.lifetime_ == 0;
			}
	};

protected:
	std::vector<Particle>	particles_;
	LPDIRECT3DVERTEXBUFFER9 points_;  // Vertex buffer for the points.
	LPDIRECT3DDEVICE9		renderTarget_;
		
	std::vector<Particle>::iterator findNextDeadParticle();
	virtual void startParticles();

	// Specific implemention to define to policy for starting/creating a single particle.
	virtual void startSingleParticle(std::vector<Particle>::iterator &) = 0;
};

#endif
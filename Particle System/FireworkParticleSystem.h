/*
Inherits from ParticleSystem and is the base class for all fireworks particle systems.
Adds randomization of particles, a new render implementation and a reset function
*/

#ifndef FIREWORK_PARTICLE_SYSTEM_H
#define FIREWORK_PARTICLE_SYSTEM_H

#include "ParticleSystem.h"
#include "EnvironmentalConstants.h"

class Projectile; // forward declaration


class FireworkParticleSystem : public ParticleSystem
{
public:
	FireworkParticleSystem(void);
	~FireworkParticleSystem(void);
	
	virtual void render(void);	
	virtual void reset(void);

	D3DXCOLOR baseColour_;				// the base colour of the particles for this system
	int fadeOutTime_;					// the particel should start fading out when there is only this much lifetime left
	D3DXVECTOR3 maxColourDivergence_;	// the colour of single particles can divert this much from the base colour (r,g,b)
	float maxLifetimeDivergence_;		// the actual lifetime of the particles can divert this much from the maximal lifetime value
	float maxSizeDivergence_;			// the actual size of the particles can divert this much from the base size value
	float maxVelocityDivergence_;		// the actual launch velocity of the particles can divert this much from the base value
	float launchVelocity_;				// the base velocity with which a particle is launched

	// for particle systems depending on position/velocity of the projectile
	void setProjectile(Projectile* projectile){sourceObject_ = projectile;}

protected:
	// for convencience, randomize specific parameters
	int getRandomLifetime(void);
	void getRandomColour(D3DXCOLOR* particleColour);
	float getRandomSize(void);
	float getRandomVelocity(void);

	Projectile* sourceObject_; 
};

#endif
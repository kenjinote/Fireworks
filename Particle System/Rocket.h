/*
The rocket class manages the associated particle systems according to its current state.
*/

#ifndef ROCKET_H
#define ROCKET_H

#include "FireworkParticleSystem.h"
#include "Projectile.h"
#include "ProjectileTrace.h"
#include "EffectSphere.h"

// enumeration describing the current state of the rocket
enum RocketState
{
	Ready,		// the rocket has been created and is ready to be fired
	Flying,		// the rocket has been fired but not yet exploded
	Exploded	// the rocket has exploded
};

class Rocket
{
public:
	Rocket(D3DXVECTOR3 startPosition, Projectile* projectile, ProjectileTrace* jet, FireworkParticleSystem* effect);
	Rocket();
	~Rocket(void);

	void initialise(LPDIRECT3DDEVICE9 device);
	void fire();
	void update();
	void render();
	void reset();

	D3DXVECTOR3 startPosition_;			// the current position of the rocket (identical to position of the projectile particle)

	Projectile* projectile_;			// the actual rocket (a system that will fire a single particle)
	ProjectileTrace* trace_;			// emitted while the rocket is flying
	FireworkParticleSystem* effect_;	// started when the rocket explodes
private:
	RocketState state_;
};

#endif

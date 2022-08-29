/*
Particle system that emits particles in all directions with each of these particles leaving a trail of sub particles.
*/

#ifndef EFFECT_RAYS_H
#define EFFECT_RAYS_H

#include "FireworkParticleSystem.h"

class EffectRays : public FireworkParticleSystem
{
public:
	EffectRays() : FireworkParticleSystem()
	{
	}

	~EffectRays(void)
	{
	}

	virtual void update(void)
	{
		// Start particles, if necessary...
		if(!exploded_)
		{
			// make sure to only start the main particles once
			startParticles();
			exploded_= true;
		}

		// Update the particles that are still alive...
		for (std::vector<Particle>::iterator p(particles_.begin()); p != particles_.end(); ++p)
		{
			if (p -> lifetime_ > 0)	// Update only if this particle is alive.
			{
				// Calculate the new position of the particle...

				p -> position_.y +=	p -> velocity_.y * timeIncrement_;
				p -> position_.x += p -> velocity_.x * timeIncrement_;
				p -> position_.z += p -> velocity_.z * timeIncrement_;

				// update acceleration/velocity of the particle

				// update velocity
				p -> velocity_.x += p -> acceleration_.x * timeIncrement_;
				p -> velocity_.y += p -> acceleration_.y * timeIncrement_;
				p -> velocity_.z += p -> acceleration_.z * timeIncrement_;

				// update acceleration (this is not physically correct but takes air drag into account to some extent)
				p -> acceleration_.x = AIR_DRAG * p -> velocity_.x;
				p -> acceleration_.y = AIR_DRAG * p -> velocity_.y + EARTH_GRAVITY;
				p -> acceleration_.z = AIR_DRAG * p -> velocity_.z;

				p -> time_ += timeIncrement_;
				--(p -> lifetime_);

				// update alpha value
				if(p->lifetime_ < fadeOutTime_)
				{
					p->colour_.a -= static_cast<float>(1)/fadeOutTime_;
				}

				if(p->id_ == 1)
				{
					p->size_ -= static_cast<float>(1)/subParticleMaxLifetime_;
				}


				if (p -> lifetime_ == 0)	// Has this particle come to the end of it's life?
				{
					--particlesAlive_;		// If so, terminate it.
				}else if(p ->id_ == 0)
				{
					startSingleSubParticle(findNextDeadParticle(), p);
				}
			}
		}

		// Create a pointer to the first vertex in the buffer
		// Also lock it, so nothing else can touch it while the values are being inserted.
		POINTVERTEX *points;
		points_ -> Lock(0, 0, (void**)&points, 0);

		// Fill the vertex buffers with data...
		int P(0);

		// Now update the vertex buffer - after the update has been
		// performed, just in case this particle has died in the process.

		for (std::vector<Particle>::iterator p(particles_.begin()); p != particles_.end(); ++p)
		{
			if (p -> lifetime_ > 0)
			{
				points[P].position_.y = p -> position_.y;
				points[P].position_.x = p -> position_.x;
				points[P].position_.z = p -> position_.z;

				points[P].color_ = p -> colour_;
				++P;
			}
		}

		points_ -> Unlock();
	}

	bool exploded_;	 //particles already started?

	float subParticleMaxSize_;
	float subParticleLaunchVelocity_;
	D3DXCOLOR subParticleBaseColour_;				// the base colour of the particles for this system
	int subParticleFadeOutTime_;					// the particel should start fading out when there is only this much lifetime left
	D3DXVECTOR3 subParticleMaxColourDivergence_;	// the colour of single particles can divert this much from the base colour (r,g,b)
	float subParticleMaxLifetimeDivergence_;		// the actual lifetime of the particles can divert this much from the maximal lifetime value
	float subParticleMaxSizeDivergence_;			// the actual size of the particles can divert this much from the base size value
	float subParticleMaxVelocityDivergence_;		// the actual launch velocity of the particles can divert this much from the base value
	int subParticleMaxLifetime_;

	virtual void reset(void)
	{
		FireworkParticleSystem::reset();
		exploded_ = false;
	}


private:
	virtual void startSingleParticle(std::vector<Particle>::iterator& p)
	{
		if (p == particles_.end()) return;	// Safety net - if there are no dead particles, don't start any new ones...

		p -> id_ = 0;

		// Reset the particle's time (for calculating it's position with s = ut+0.5t*t)
		p -> time_ = 0;

		// Calculate random angles that will determine the direction in which to emit the particles
		float angleHorizontal = (float)(D3DXToRadian(random_number()));
		float angleVertical = (float)(D3DXToRadian(random_number()));

		// set particle starting positions to the origin
		p -> position_.x = origin_.x;
		p -> position_.y = origin_.y;
		p -> position_.z = origin_.z;

		float particleLaunchVelocity = getRandomVelocity();

		// Calculate start velocity for the particle using the previously calculated angles
		p->velocity_.x = particleLaunchVelocity * (float)sin(angleVertical) * (float)cos(angleHorizontal);
		p->velocity_.y = particleLaunchVelocity * (float)cos(angleVertical);
		p->velocity_.z = particleLaunchVelocity * (float)sin(angleVertical) * (float)sin(angleHorizontal);
		
		// Calculate start acceleration
		p -> acceleration_.x = AIR_DRAG * p -> velocity_.x;
		p -> acceleration_.y = AIR_DRAG * p -> velocity_.y + EARTH_GRAVITY;
		p -> acceleration_.z = AIR_DRAG * p -> velocity_.z;

		// set the colour for the particle
		getRandomColour(&(p->colour_));
		// set lifetime 
		p -> lifetime_ = getRandomLifetime();
		// set particle size
		p -> size_ = getRandomSize();

		++particlesAlive_;
	}

	virtual void startParticles()
	{
		// Number of particles to start in this batch...
		for (int i(0); i < startParticles_; ++i)
		{
			if (particlesAlive_ < maxParticles_) startSingleParticle(findNextDeadParticle());
		}
	}

	void startSingleSubParticle(std::vector<Particle>::iterator& p, std::vector<Particle>::iterator& sourceParticle)
	{
		if (p == particles_.end()) return;	// Safety net - if there are no dead particles, don't start any new ones...

		p -> id_ = 1;	// sub particle

		// Reset the particle's time (for calculating it's position with s = ut+0.5t*t)
		p -> time_ = 0;

		// set particle starting positions to the origin
		p -> position_.x = sourceParticle->position_.x;
		p -> position_.y = sourceParticle->position_.y;
		p -> position_.z = sourceParticle->position_.z;

		float particleLaunchVelocity = subParticleLaunchVelocity_ - (static_cast<float>(random_number(0, 200)) - 100.0f) * 0.01f * subParticleMaxVelocityDivergence_;

		// Calculate start velocity for the particle using the previously calculated angles
		
		D3DXVECTOR3 normalisedSourceVelocity;
		D3DXVec3Normalize(&normalisedSourceVelocity, &(sourceParticle->velocity_));

		// these sub particles don't have a velocity of their own, they simply get placed at the current position of the respective main
		// particle and are subject to environmental influence from there on
		p->velocity_.x = 0;
		p->velocity_.y = 0;
		p->velocity_.z = 0;

		// Calculate start acceleration
		p -> acceleration_.x = AIR_DRAG * p -> velocity_.x;
		p -> acceleration_.y = AIR_DRAG * p -> velocity_.y + EARTH_GRAVITY;
		p -> acceleration_.z = AIR_DRAG * p -> velocity_.z;

		// set the colour for the particle
		p->colour_.a = subParticleBaseColour_.a;
		p->colour_.r = subParticleBaseColour_.r;
		p->colour_.g = subParticleBaseColour_.g;
		p->colour_.b = subParticleBaseColour_.b;

		// set the lifetime for the sub particle
		if(sourceParticle->lifetime_ >= subParticleMaxLifetime_)
			p -> lifetime_ = subParticleMaxLifetime_;
		else
			p -> lifetime_ = sourceParticle->lifetime_;	// it looks better when the sub paticle does not live longer than the source particle
		
		// set particle size
		p -> size_ = subParticleMaxSize_ - static_cast<float>(random_number(0, 100)) * 0.01f * subParticleMaxSizeDivergence_; 

		++particlesAlive_;
	}
};

#endif
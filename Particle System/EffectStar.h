/*
Particle system that emits particles in straight rays resembling a star.
*/

#ifndef EFFECT_STAR_H
#define EFFECT_STAR_H

#include "FireworkParticleSystem.h"

class EffectStar : public FireworkParticleSystem
{
public:
	EffectStar() : FireworkParticleSystem(), numberOfRays_(0), rayParticleCounter_(0), particlesPerRay_(0), angleHorizontal_(0), angleVertical_(0)
	{
	}

	~EffectStar(void)
	{
	}

	virtual void update(void)
	{
		// Start particles, if necessary...
		startParticles();

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

				if (p -> lifetime_ == 0)	// Has this particle come to the end of it's life?
				{
					--particlesAlive_;		// If so, terminate it.
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

	int numberOfRays_;
private:
	int particlesPerRay_;
	int rayParticleCounter_;
	float angleHorizontal_;
	float angleVertical_;

	virtual void startSingleParticle(std::vector<Particle>::iterator& p)
	{
		if (p == particles_.end()) return;	// Safety net - if there are no dead particles, don't start any new ones...

		if(particlesAlive_ == 0)
		{
			calculateParticlesPerRay();
		}

		if(rayParticleCounter_ == particlesPerRay_)
		{
			rayParticleCounter_ = 0;
		}

		// calculate a new angle for every ray of the star but use the same angle for all particles of a specific ray
		if(rayParticleCounter_ == 0)
		{
			// Calculate random angles that will determine the direction in which to emit the particles
			angleHorizontal_ = (float)(D3DXToRadian(random_number()));
			angleVertical_ = (float)(D3DXToRadian(random_number()));
		}

		++rayParticleCounter_;

		// Reset the particle's time (for calculating it's position with s = ut+0.5t*t)
		p -> time_ = 0;

		// set particle starting positions to the origin
		p -> position_.x = origin_.x;
		p -> position_.y = origin_.y;
		p -> position_.z = origin_.z;

		float particleLaunchVelocity = getRandomVelocity();

		// Calculate start velocity for the particle using the previously calculated angles
		p->velocity_.x = particleLaunchVelocity * (float)sin(angleVertical_) * (float)cos(angleHorizontal_);
		p->velocity_.y = particleLaunchVelocity * (float)cos(angleVertical_);
		p->velocity_.z = particleLaunchVelocity * (float)sin(angleVertical_) * (float)sin(angleHorizontal_);
		
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

	// determine the number of particles that will be placed on each ray
	void calculateParticlesPerRay()
	{
		if(numberOfRays_ > 0)
			particlesPerRay_ = maxParticles_ / numberOfRays_;
		else
			particlesPerRay_ = 0;
	}
};

#endif
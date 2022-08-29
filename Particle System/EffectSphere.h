/*
Particle system, that emits particles in all directions from a common origin.
*/

#ifndef EFFECT_SPHERE_H
#define EFFECT_SPHERE_H

#include "FireworkParticleSystem.h"

class EffectSphere : public FireworkParticleSystem
{
public:
	EffectSphere() : FireworkParticleSystem()
	{
	}

	~EffectSphere(void)
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

private:
	virtual void startSingleParticle(std::vector<Particle>::iterator& p)
	{
		if (p == particles_.end()) return;	// Safety net - if there are no dead particles, don't start any new ones...

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
};

#endif
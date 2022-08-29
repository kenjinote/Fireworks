/*
Particle system that emits particles from a moving projectile in the opposite direction of that projectile.
*/

#ifndef JET_H
#define JET_H

#include "FireworkParticleSystem.h"
#include "Projectile.h"


class ProjectileTrace : public FireworkParticleSystem
{
public:
	ProjectileTrace() : FireworkParticleSystem()
	{
	}

	~ProjectileTrace(void)
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

				// Vertical distance.
				float s = (p -> velocity_.y * p -> time_) + (EARTH_GRAVITY * p -> time_ * p -> time_);

				// the position is calculated in relation to the particle's origin
				p -> position_.y = s + p -> origin_.y;									
				p -> position_.x = (p -> velocity_.x * p -> time_) + p -> origin_.x;
				p -> position_.z = (p -> velocity_.z * p -> time_) + p -> origin_.z;

				p -> time_ += timeIncrement_;
				--(p -> lifetime_);

				if (p -> lifetime_ == 0)	// Has this particle come to the end of it's life?
				{
					--particlesAlive_;		// If so, terminate it.
				}
			}
		}

		// move the origin according to the movement of the source object (projectile)
		origin_ = *(sourceObject_->getProjectilePosition());

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

		// set the origin for the particle to the current position of the particle system (later on will change)
		p -> origin_ = origin_;

		// get the flying direction of the projectile in order to emit the trace particles in the opposite direction
		D3DXVECTOR3 normalizedSourceDirection;
		D3DXVec3Normalize(&normalizedSourceDirection, sourceObject_->getProjectileMoveDirection());

		// Now calculate the particle's horizontal and depth components.
		// Emit the particles in the opposite direction to the direction of the source object
		p -> velocity_.x = launchVelocity_ * -(normalizedSourceDirection.x);
		p -> velocity_.y = launchVelocity_ * -(normalizedSourceDirection.y);
		p -> velocity_.z = launchVelocity_ * -(normalizedSourceDirection.z);

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
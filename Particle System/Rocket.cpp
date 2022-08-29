#include "Rocket.h"


Rocket::Rocket(D3DXVECTOR3 startPosition, Projectile* projectile, ProjectileTrace* trace, FireworkParticleSystem* effect) : startPosition_(startPosition), state_(Ready), projectile_(projectile), trace_(trace), effect_(effect)
{

}

Rocket::Rocket() : state_(Ready), projectile_(nullptr), trace_(nullptr), effect_(nullptr)
{
}

Rocket::~Rocket(void)
{
}

// starts the rocket
void Rocket::fire(void)
{
	state_ = Flying;
}

// called every frame
void Rocket::update(void)
{
	switch(state_)
	{
	case Flying:
		projectile_->update();
		if(projectile_->isExploded())
		{
			// set the origin of the effect to the last position of the projectile
			effect_->origin_ = *(projectile_->getProjectilePosition());
			state_ = Exploded;
		}
		trace_->update();
		break;
	case Exploded:
		effect_->update();
		break;
	}
}

void Rocket::initialise(LPDIRECT3DDEVICE9 device)
{
	// initialise the associated particle systems
	projectile_ -> initialise(device);
	trace_ -> initialise(device);
	effect_ -> initialise(device);

	// some further initialising
	projectile_ -> origin_ = startPosition_;
	trace_ -> origin_ = startPosition_;
	effect_ -> origin_ = startPosition_;

	trace_ -> setProjectile(projectile_);
	effect_ -> setProjectile(projectile_);
}

// resets the rocket to be fired another time
void Rocket::reset()
{
	state_ = Ready;

	projectile_->reset();
	trace_->reset();
	effect_->reset();

	projectile_ -> origin_ = startPosition_;
	trace_ -> origin_ = startPosition_;
	effect_ -> origin_ = startPosition_;
}

// render the particle systems
void Rocket::render(void)
{
	switch(state_)
	{
	case Flying:
		projectile_->render();
		trace_->render();
		break;
	case Exploded:
		effect_->render();
		break;
	}
}

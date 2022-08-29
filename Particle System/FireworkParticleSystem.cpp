#include "FireworkParticleSystem.h"


FireworkParticleSystem::FireworkParticleSystem(void) : ParticleSystem(), 
	baseColour_(1.0f,1.0f,1.0f,1.0f),
	fadeOutTime_(0),
	maxColourDivergence_(0,0,0),
	maxLifetimeDivergence_(0),
	maxSizeDivergence_(0),
	maxVelocityDivergence_(0),
	launchVelocity_(0)
{
}


FireworkParticleSystem::~FireworkParticleSystem(void)
{
}

// virtual function
void FireworkParticleSystem::render(void)
{
	// Enable point sprites, and set the size of the point.
	renderTarget_  -> SetRenderState(D3DRS_POINTSPRITEENABLE, true);
	renderTarget_  -> SetRenderState(D3DRS_POINTSCALEENABLE,  true);

	// Disable z buffer while rendering the particles. Makes rendering quicker and
	// stops any visual (alpha) 'artefacts' on screen while rendering.
	renderTarget_ -> SetRenderState(D3DRS_ZENABLE, false);
		    
	// Scale the points according to distance...
	renderTarget_ -> SetRenderState(D3DRS_POINTSIZE,     FtoDW(maxParticleSize_));

	renderTarget_ -> SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(0.00f));
	renderTarget_ -> SetRenderState(D3DRS_POINTSCALE_A,  FtoDW(0.00f));
	renderTarget_ -> SetRenderState(D3DRS_POINTSCALE_B,  FtoDW(0.00f));
	renderTarget_ -> SetRenderState(D3DRS_POINTSCALE_C,  FtoDW(1.00f));

	// Now select the texture for the points...
	// Use texture colour and alpha components.
	renderTarget_ -> SetTexture(0, particleTexture_);
	renderTarget_ -> SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	renderTarget_ -> SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	renderTarget_ -> SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	
	// use the diffuse color of the particle
	renderTarget_ -> SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	renderTarget_ -> SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);

	// modulate the alpha values of the particle and the texture 
	renderTarget_ -> SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	renderTarget_ -> SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	renderTarget_ -> SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	
	// Render the contents of the vertex buffer.
	renderTarget_ -> SetStreamSource(0, points_, 0, sizeof(POINTVERTEX));
	renderTarget_ -> SetFVF(D3DFVF_POINTVERTEX);

	for(unsigned int i = 0; i < particles_.size(); ++i)
	{
		if(particles_[i].lifetime_ > 0)
		{
			// render the particle in its specific size
			renderTarget_ -> SetRenderState(D3DRS_POINTSIZE, FtoDW(particles_[i].size_));
			renderTarget_ -> DrawPrimitive(D3DPT_POINTLIST, i, 1);
		}
	}

	// Reset the render states.
	renderTarget_ -> SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	renderTarget_ -> SetRenderState(D3DRS_POINTSCALEENABLE,  false);
	renderTarget_ -> SetRenderState(D3DRS_ALPHABLENDENABLE,  false);
	renderTarget_ -> SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

	renderTarget_ -> SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
}

// returns the lifetime for a particle taking the allowed divergence into account
int FireworkParticleSystem::getRandomLifetime(void)
{
	return maxLifetime_ - random_number(0, static_cast<unsigned int>(maxLifetimeDivergence_));
}

// sets the colour for a particle using the set values for divergence
void FireworkParticleSystem::getRandomColour(D3DXCOLOR* particleColour)
{
	particleColour -> a = baseColour_.a;

	particleColour -> r = baseColour_.r + (static_cast<float>(random_number(0, 200)) - 100.0f) * 0.01f * maxColourDivergence_.x;
	particleColour -> g = baseColour_.g + (static_cast<float>(random_number(0, 200)) - 100.0f) * 0.01f * maxColourDivergence_.y;
	particleColour -> b = baseColour_.b + (static_cast<float>(random_number(0, 200)) - 100.0f) * 0.01f * maxColourDivergence_.z;

	if(particleColour -> r > 1.0f)
		particleColour -> r = 1.0f;
	else if(particleColour -> r < 0.0f)
		particleColour -> r = 0.0f;

	if(particleColour -> g > 1.0f)
		particleColour -> g = 1.0f;
	else if(particleColour -> g < 0.0f)
		particleColour -> g = 0.0f;

	if(particleColour -> b > 1.0f)
		particleColour -> b = 1.0f;
	else if(particleColour -> b < 0.0f)
		particleColour -> b = 0.0f;
}

// sets the size for a particle taking the allowed divergence into account
float FireworkParticleSystem::getRandomSize(void)
{
	return maxParticleSize_ - static_cast<float>(random_number(0, 100)) * 0.01f * maxSizeDivergence_; 
}

// sets the launch velocity for a particle taking the allowed divergence into account
float FireworkParticleSystem::getRandomVelocity(void)
{
	return launchVelocity_ - (static_cast<float>(random_number(0, 200)) - 100.0f) * 0.01f * maxVelocityDivergence_; 
}

void FireworkParticleSystem::reset(void)
{
	particlesAlive_ = 0;
	startTimer_ = 0;
}
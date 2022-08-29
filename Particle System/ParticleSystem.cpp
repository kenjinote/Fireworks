#include "ParticleSystem.h"


ParticleSystem::ParticleSystem(void) : maxParticles_(0), startParticles_(0), particlesAlive_(0), maxLifetime_(0), origin_(D3DXVECTOR3(0, 0, 0)), points_(NULL), maxParticleSize_(1.0f)
{
}


ParticleSystem::~ParticleSystem(void)
{
	SAFE_RELEASE(points_);
}

HRESULT ParticleSystem::initialise(LPDIRECT3DDEVICE9 device)
{
	// Store the render target for later use...
	renderTarget_ = device;
			
	Particle p;
	reset_particle(p);
	particles_.resize(maxParticles_, p);	// Create a vector of empty particles - make 'max_particles_' copies of particle 'p'.

	// Create a vertex buffer for the particles (each particule represented as an individual vertex).
	int buffer_size = maxParticles_ * sizeof(POINTVERTEX);

	// The data in the buffer doesn't exist at this point, but the memory space
	// is allocated and the pointer to it (g_pPointBuffer) also exists.
	if (FAILED(device -> CreateVertexBuffer(buffer_size, 0, D3DFVF_POINTVERTEX, D3DPOOL_DEFAULT, &points_, NULL)))
	{
		return E_FAIL; // Return if the vertex buffer culd not be created.
	}

	return S_OK;
}

// virtual function
// this is pretty much a default implementation for rendering
void ParticleSystem::render()
{
	// Enable point sprites, and set the size of the point.
	renderTarget_	-> SetRenderState(D3DRS_POINTSPRITEENABLE, true);
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

	renderTarget_ -> SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	renderTarget_ -> SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);

	renderTarget_ -> SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	renderTarget_ -> SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

	// Render the contents of the vertex buffer.
	renderTarget_ -> SetStreamSource(0, points_, 0, sizeof(POINTVERTEX));
	renderTarget_ -> SetFVF(D3DFVF_POINTVERTEX);
	renderTarget_ -> DrawPrimitive(D3DPT_POINTLIST, 0, particlesAlive_);

	// Reset the render states.
	renderTarget_ -> SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	renderTarget_ -> SetRenderState(D3DRS_POINTSCALEENABLE,  false);
	renderTarget_ -> SetRenderState(D3DRS_ALPHABLENDENABLE,  false);
	renderTarget_ -> SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	renderTarget_ -> SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
}

std::vector<Particle>::iterator ParticleSystem::findNextDeadParticle()
{
	return std::find_if(particles_.begin(), particles_.end(), isParticleDead());
}

// virtual function
void ParticleSystem::startParticles()
{
	// Only start a new particle when the time is right and there are enough dead (inactive) particles.
	if (startTimer_ == 0 && particlesAlive_ < maxParticles_)
	{
		// Number of particles to start in this batch...
		for (int i(0); i < startParticles_; ++i)
		{
			if (particlesAlive_ < maxParticles_) startSingleParticle(findNextDeadParticle());
		}

		// Reset the start timer for the next batch of particles.
		startTimer_ = startInterval_;
	}
	else
	{
		// Otherwise decrease the start timer.
		--startTimer_;
	}
}

		
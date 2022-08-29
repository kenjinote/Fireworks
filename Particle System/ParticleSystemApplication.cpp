/*
The whole application strongly builds upon the particle system example that is available on Blackboard.
In this file, DirectX is set up and the different rockets/particle systems are created.
*/

#include <Windows.h>			// Windows library (for window functions, menus, dialog boxes, etc)
#include "ParticleSystem.h"
#include "Rocket.h"
#include "Helpers.h"
#include <vector>
#include "EffectStar.h"
#include "EffectCone.h"
#include "EffectMultiSphere.h"
#include "EffectRays.h"
#include <thread>
#include "FireworksTimer.h"

using namespace std;


//---------------------------------------------------------------------------------------------------------------------------------
// Global variables

LPDIRECT3D9             d3d		= NULL;	// Used to create the device
LPDIRECT3DDEVICE9       device	= NULL;	// The rendering device

// these particle systems are the effects that will be shown as the rockets explode
EffectSphere effectSpheres[6];
EffectStar effectStars[5];
EffectCone effectCones[2];
EffectMultiSphere effectMultiSpheres[1];
EffectRays effectRays[1];

// these particle systems are the same for every rocket
Projectile projectiles[15];		// the projectile particle system fires just a single particle depicting the actual rocket
ProjectileTrace traces[15];		// the projectile trace simulates spark of the rocket's jet/thruster

// each rocket hold three particle systems: a Projectile, a ProjectileTrace and an effect
Rocket rockets[15];

// start times for the different rockets, will be used by the FireworksTimer
float rocketStartTimes[15] = {2000.0f, 
							  4000.0f, 4000.0f, 4000.0f, 4000.0f, 
							  6000.0f, 6500.0f, 7000.0f, 7500.0f, 8000.0f,
							  10000.0f, 10000.0f, 10000.0f,
							  12000.0f,
							  16000.0f};

int numberOfRockets = 15;

// pointers to the different textures that can be used for the point sprites
LPDIRECT3DTEXTURE9	particle_circle		= NULL;		// mostly used
LPDIRECT3DTEXTURE9	particle_star		= NULL;		// rarely used
LPDIRECT3DTEXTURE9	particle_diamond	= NULL;		// currently not used 

// used for communication with the timer thread
bool doRun; // set to false whent he application is about to shut down

//---------------------------------------------------------------------------------------------------------------------------------
// Initialise Direct 3D.
// Requires a handle to the window in which the graphics will be drawn.

HRESULT SetupD3D(HWND hWnd)
{
	// Create the D3D object.
    if (NULL == (d3d = Direct3DCreate9(D3D_SDK_VERSION))) return E_FAIL;

    // Set up the structure used to create the device
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	 // Check for hardware T&L
	D3DCAPS9 D3DCaps;
	d3d -> GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps);

	// use hardware vertex processing and a pure device if possible
	DWORD vertexProcessing = 0;
	if (D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
	{
		vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		
		if (D3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE) 
		{
			vertexProcessing |= D3DCREATE_PUREDEVICE;
		}
	}else 
	{
		vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

    // Create the device
    if (FAILED(d3d -> CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, vertexProcessing, &d3dpp, &device)))
    {
        return E_FAIL;
    }
    
	// Enable the Z buffer, since we're dealing with 3D geometry.
	device -> SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Release (delete) all the resources used by this program.
// Only release things if they are valid (i.e. have a valid pointer).
// If not, the program will crash at this point.

void CleanUp()
{
	SAFE_RELEASE(device);
    SAFE_RELEASE(d3d);
	SAFE_RELEASE(particle_circle);
	SAFE_RELEASE(particle_star);
	SAFE_RELEASE(particle_diamond);
}

//-----------------------------------------------------------------------------
// Initialise the geometry and meshes for this demonstration.

HRESULT SetupGeometry()
{
	// placeholder
	return S_OK;
}

//-----------------------------------------------------------------------------
// Set up the view - the camera and projection matrices.

void SetupViewMatrices()
{
	// Set up the view matrix.
	// This defines which way the 'camera' will look at, and which way is up.

    D3DXVECTOR3 vCamera(0.0f, 0.0f, -700);
    D3DXVECTOR3 vLookat(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVector(0.0f, 1.0f, 0.0f);
    D3DXMATRIX matView;
    D3DXMatrixLookAtLH(&matView, &vCamera, &vLookat, &vUpVector);
    device -> SetTransform(D3DTS_VIEW, &matView);

	// Set up the projection matrix.
	// This transforms 2D geometry into a 3D space.
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4, 800.0f/600.0f, 1.0f, 800.0f);
    device -> SetTransform(D3DTS_PROJECTION, &matProj);
}

//------------------------------------------------------------------------------
// Initialise one light.

void SetupLights()
{
	// Define a light - possesses only a diffuse colour.
    D3DLIGHT9 Light1;
    ZeroMemory(&Light1, sizeof(D3DLIGHT9));
    Light1.Type = D3DLIGHT_DIRECTIONAL;

	Light1.Diffuse.r = 1.0f;
	Light1.Diffuse.g = 1.0f;
	Light1.Diffuse.b = 1.0f;

	Light1.Position.x = 0.0f;
	Light1.Position.y = 0.0f;
	Light1.Position.z = -600.0f;

	Light1.Direction.x = 0.0f;
	Light1.Direction.y = 0.0f;
	Light1.Direction.z = 1.0f;

	Light1.Attenuation0 = 1.0f; 
	Light1.Attenuation1 = 0.0f; 
	Light1.Attenuation2 = 0.0f;

    Light1.Range = 2000.0f;

	// Select and enable the light.
    device -> SetLight(0, &Light1);
    device -> LightEnable(0, true);
}

//-----------------------------------------------------------------------------
// Render the scene.

void render()
{
    // Clear the backbuffer to a blue colour, also clear the Z buffer at the same time.
    device -> Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(10, 10, 10), 1.0f, 0);

    // Begin the scene
    if (SUCCEEDED(device -> BeginScene()))
    {
		// lighting is not needed
		device->SetRenderState( D3DRS_LIGHTING, FALSE );

		// Render the rockets
		for(int i = 0; i < numberOfRockets; ++i)
		{
			rockets[i].render();
		}

        device -> EndScene();
    }

    // Present the backbuffer to the display.
    device -> Present(NULL, NULL, NULL, NULL);
}


//-----------------------------------------------------------------------------
// The window's message handling function.

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
		{
			// terminate the timer thread
			doRun = false;

            PostQuitMessage(0);
            return 0;
		}
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// WinMain() - The application's entry point.

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    // Register the window class
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "PSystem", NULL};
    RegisterClassEx(&wc);

	// Create the application's window
	HWND hWnd = CreateWindow("PSystem", "Fireworks", WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// seed the random number generator
	seedRandomNumberGenerator();

    // Initialize Direct3D
    if (SUCCEEDED(SetupD3D(hWnd)))
    {
        // Create the scene geometry
        if (SUCCEEDED(SetupGeometry()))
        {
            // Show the window
            ShowWindow(hWnd, SW_SHOWDEFAULT);
            UpdateWindow(hWnd);

			// Set up the light.
			SetupLights();

			void SetupParticleSystems();
			// initialise the different particle systems that will be used in the scene
			SetupParticleSystems();

			// start the timer that will fire the rockets at predefined times
			doRun = true;
			thread timer = thread(FireworksTimer(&rockets[0], numberOfRockets), &rocketStartTimes[0], &doRun);

            // Enter the message loop
            MSG msg;
            ZeroMemory(&msg, sizeof(msg));
            while (msg.message != WM_QUIT)
            {
                if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
				{
					SetupViewMatrices();

					// update the rockets
					for(int i = 0; i < numberOfRockets; ++i)
					{
						rockets[i].update();
					}

					// render the scene
					render();
				}
            }

			// wait for timer thread to finish
			timer.join();	
        }
    }

	CleanUp();

    UnregisterClass("PSystem", wc.hInstance);
    return 0;
}


//-----------------------------------------------------------------------------
// Initialise the parameters for the particle system.

void SetupParticleSystems()
{
	D3DXCreateTextureFromFile(device, "particle_circle.png", &particle_circle);
	D3DXCreateTextureFromFile(device, "particle_star.png", &particle_star);
	D3DXCreateTextureFromFile(device, "particle_diamond.png", &particle_diamond);

	//----------------------------------------------------------------------------------------
	// setup projectiles

	// set all parameters of the projectiles to default values
	for(int i = 0; i < numberOfRockets; ++i)
	{
		projectiles[i].baseColour_		= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		projectiles[i].launchAngle_		= 0.0f;
		projectiles[i].launchVelocity_	= 90.0f;
		projectiles[i].maxParticles_	= 1;
		projectiles[i].origin_			= D3DXVECTOR3(0, 0, 0);
		projectiles[i].startInterval_	= 1;
		projectiles[i].startTimer_		= 0;
		projectiles[i].timeIncrement_	= 0.08f;
		projectiles[i].maxLifetime_		= 90;
		projectiles[i].startParticles_	= 1; 
		projectiles[i].maxParticleSize_	= 16.0f;
		projectiles[i].particleTexture_ = particle_circle;
	}

	// manually adjust launch angles and lifetime for the rockets

	// effect spheres
	projectiles[1].launchAngle_		= 0;
	projectiles[1].maxLifetime_		= 90;

	projectiles[1].launchAngle_		= 15;
	projectiles[1].maxLifetime_		= 80;

	projectiles[2].launchAngle_		= 5;
	projectiles[2].maxLifetime_		= 85;

	projectiles[3].launchAngle_		= -5;
	projectiles[3].maxLifetime_		= 90;

	projectiles[4].launchAngle_		= -15;
	projectiles[4].maxLifetime_		= 95;

	// effect stars
	projectiles[5].launchAngle_		= 30;
	projectiles[5].maxLifetime_		= 80;

	projectiles[6].launchAngle_		= -10;
	projectiles[6].maxLifetime_		= 90;

	projectiles[7].launchAngle_		= 10;
	projectiles[7].maxLifetime_		= 90;

	projectiles[8].launchAngle_		= -30;
	projectiles[8].maxLifetime_		= 80;

	projectiles[9].launchAngle_		= 0;
	projectiles[9].maxLifetime_		= 110;

	// cones
	projectiles[10].launchAngle_		= 20;
	projectiles[10].maxLifetime_		= 90;

	projectiles[11].launchAngle_		= -20;
	projectiles[11].maxLifetime_		= 90;

	// rays
	projectiles[12].launchAngle_		= 0;
	projectiles[12].maxLifetime_		= 100;

	// multisphere
	projectiles[13].launchAngle_		= 0;
	projectiles[13].maxLifetime_		= 100;

	// sphere
	projectiles[14].launchAngle_		= 0;
	projectiles[14].maxLifetime_		= 90;

	//----------------------------------------------------------------------------------------
	// setup projectile traces

	// set all parameters of the projectile traces to default values
	for(int i = 0; i < numberOfRockets; ++i)
	{
		traces[i].baseColour_		= D3DXCOLOR(1.0f, 0.5f, 0.0f, 1.0f);
		traces[i].launchVelocity_	= 10.0f;
		traces[i].maxParticles_		= 300;
		traces[i].origin_			= D3DXVECTOR3(0, 0, 0);
		traces[i].startInterval_	= 0;
		traces[i].startTimer_		= 0;
		traces[i].timeIncrement_	= 0.08f;
		traces[i].maxLifetime_		= 5;
		traces[i].startParticles_	= 1;
		traces[i].maxParticleSize_	= 4.0f;
		traces[i].particleTexture_	= particle_circle;
	}

	//-------------------------------------------------------------------------------
	// effect spheres

	effectSpheres[0].fadeOutTime_			= 30;					
	effectSpheres[0].maxColourDivergence_.x	= 0.0f;	
	effectSpheres[0].maxColourDivergence_.y	= 0.5f;	
	effectSpheres[0].maxColourDivergence_.z	= 0.25f;	
	effectSpheres[0].maxLifetimeDivergence_	= 5;		
	effectSpheres[0].maxSizeDivergence_		= 5.0f;	
	effectSpheres[0].maxVelocityDivergence_	= 5.0f;
	effectSpheres[0].baseColour_			= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectSpheres[0].launchVelocity_		= 70.0f;
	effectSpheres[0].maxParticles_			= 1000;
	effectSpheres[0].origin_				= D3DXVECTOR3(0, 0, 0);
	effectSpheres[0].startInterval_			= 1;
	effectSpheres[0].startTimer_			= 0;
	effectSpheres[0].timeIncrement_			= 0.08f;
	effectSpheres[0].maxLifetime_			= 80;
	effectSpheres[0].startParticles_		= 1000;
	effectSpheres[0].maxParticleSize_		= 12.0f;
	effectSpheres[0].particleTexture_		= particle_circle;

	effectSpheres[1].fadeOutTime_			= 15;					
	effectSpheres[1].maxColourDivergence_.x	= 0.25f;	
	effectSpheres[1].maxColourDivergence_.y	= 0.5f;	
	effectSpheres[1].maxColourDivergence_.z	= 0.25f;	
	effectSpheres[1].maxLifetimeDivergence_	= 5;		
	effectSpheres[1].maxSizeDivergence_		= 5.0f;	
	effectSpheres[1].maxVelocityDivergence_	= 3.0f;
	effectSpheres[1].baseColour_			= D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	effectSpheres[1].launchVelocity_		= 40.0f;
	effectSpheres[1].maxParticles_			= 800;
	effectSpheres[1].origin_				= D3DXVECTOR3(0, 0, 0);
	effectSpheres[1].startInterval_			= 1;
	effectSpheres[1].startTimer_			= 0;
	effectSpheres[1].timeIncrement_			= 0.08f;
	effectSpheres[1].maxLifetime_			= 80;
	effectSpheres[1].startParticles_		= 800;
	effectSpheres[1].maxParticleSize_		= 12.0f;
	effectSpheres[1].particleTexture_		= particle_circle;

	effectSpheres[2].fadeOutTime_			= 15;					
	effectSpheres[2].maxColourDivergence_.x	= 0.0f;	
	effectSpheres[2].maxColourDivergence_.y	= 0.5f;	
	effectSpheres[2].maxColourDivergence_.z	= 0.5f;	
	effectSpheres[2].maxLifetimeDivergence_	= 5;		
	effectSpheres[2].maxSizeDivergence_		= 5.0f;	
	effectSpheres[2].maxVelocityDivergence_	= 3.0f;
	effectSpheres[2].baseColour_			= D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);
	effectSpheres[2].launchVelocity_		= 40.0f;
	effectSpheres[2].maxParticles_			= 800;
	effectSpheres[2].origin_				= D3DXVECTOR3(0, 0, 0);
	effectSpheres[2].startInterval_			= 1;
	effectSpheres[2].startTimer_			= 0;
	effectSpheres[2].timeIncrement_			= 0.08f;
	effectSpheres[2].maxLifetime_			= 80;
	effectSpheres[2].startParticles_		= 800;
	effectSpheres[2].maxParticleSize_		= 12.0f;
	effectSpheres[2].particleTexture_		= particle_circle;

	effectSpheres[3].fadeOutTime_			= 15;					
	effectSpheres[3].maxColourDivergence_.x	= 0.5f;	
	effectSpheres[3].maxColourDivergence_.y	= 0.0f;	
	effectSpheres[3].maxColourDivergence_.z	= 0.5f;	
	effectSpheres[3].maxLifetimeDivergence_	= 5;		
	effectSpheres[3].maxSizeDivergence_		= 5.0f;	
	effectSpheres[3].maxVelocityDivergence_	= 3.0f;
	effectSpheres[3].baseColour_			= D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	effectSpheres[3].launchVelocity_		= 40.0f;
	effectSpheres[3].maxParticles_			= 800;
	effectSpheres[3].origin_				= D3DXVECTOR3(0, 0, 0);
	effectSpheres[3].startInterval_			= 1;
	effectSpheres[3].startTimer_			= 0;
	effectSpheres[3].timeIncrement_			= 0.08f;
	effectSpheres[3].maxLifetime_			= 80;
	effectSpheres[3].startParticles_		= 800;
	effectSpheres[3].maxParticleSize_		= 12.0f;
	effectSpheres[3].particleTexture_		= particle_circle;

	effectSpheres[4].fadeOutTime_			= 15;					
	effectSpheres[4].maxColourDivergence_.x	= 0.5f;	
	effectSpheres[4].maxColourDivergence_.y	= 0.5f;	
	effectSpheres[4].maxColourDivergence_.z	= 0.0f;	
	effectSpheres[4].maxLifetimeDivergence_	= 5;		
	effectSpheres[4].maxSizeDivergence_		= 5.0f;	
	effectSpheres[4].maxVelocityDivergence_	= 3.0f;
	effectSpheres[4].baseColour_			= D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f);
	effectSpheres[4].launchVelocity_		= 40.0f;
	effectSpheres[4].maxParticles_			= 800;
	effectSpheres[4].origin_				= D3DXVECTOR3(0, 0, 0);
	effectSpheres[4].startInterval_			= 1;
	effectSpheres[4].startTimer_			= 0;
	effectSpheres[4].timeIncrement_			= 0.08f;
	effectSpheres[4].maxLifetime_			= 80;
	effectSpheres[4].startParticles_		= 800;
	effectSpheres[4].maxParticleSize_		= 12.0f;
	effectSpheres[4].particleTexture_		= particle_circle;

	effectSpheres[5].fadeOutTime_			= 30;					
	effectSpheres[5].maxColourDivergence_.x	= 0.5f;	
	effectSpheres[5].maxColourDivergence_.y	= 0.0f;	
	effectSpheres[5].maxColourDivergence_.z	= 0.0f;	
	effectSpheres[5].maxLifetimeDivergence_	= 5;		
	effectSpheres[5].maxSizeDivergence_		= 5.0f;	
	effectSpheres[5].maxVelocityDivergence_	= 5.0f;
	effectSpheres[5].baseColour_			= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectSpheres[5].launchVelocity_		= 70.0f;
	effectSpheres[5].maxParticles_			= 1000;
	effectSpheres[5].origin_				= D3DXVECTOR3(0, 0, 0);
	effectSpheres[5].startInterval_			= 1;
	effectSpheres[5].startTimer_			= 0;
	effectSpheres[5].timeIncrement_			= 0.08f;
	effectSpheres[5].maxLifetime_			= 80;
	effectSpheres[5].startParticles_		= 1000;
	effectSpheres[5].maxParticleSize_		= 12.0f;
	effectSpheres[5].particleTexture_		= particle_circle;

	//--------------------------------------------------------------------------
	// effect stars

	effectStars[0].fadeOutTime_				= 15;					
	effectStars[0].maxColourDivergence_.x	= 0.75f;	
	effectStars[0].maxColourDivergence_.y	= 0.5f;	
	effectStars[0].maxColourDivergence_.z	= 0.0f;	
	effectStars[0].maxLifetimeDivergence_	= 5;		
	effectStars[0].maxSizeDivergence_		= 5.0f;	
	effectStars[0].maxVelocityDivergence_	= 20.0f;
	effectStars[0].numberOfRays_			= 50;
	effectStars[0].baseColour_				= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectStars[0].launchVelocity_			= 50.0f;
	effectStars[0].maxParticles_			= 1000;
	effectStars[0].origin_					= D3DXVECTOR3(0, 0, 0);
	effectStars[0].startInterval_			= 1;
	effectStars[0].startTimer_				= 0;
	effectStars[0].timeIncrement_			= 0.08f;
	effectStars[0].maxLifetime_				= 60;
	effectStars[0].startParticles_			= 1000;
	effectStars[0].maxParticleSize_			= 10.0f;
	effectStars[0].particleTexture_			= particle_circle;

	effectStars[1].fadeOutTime_				= 15;					
	effectStars[1].maxColourDivergence_.x	= 0.0f;	
	effectStars[1].maxColourDivergence_.y	= 0.5f;	
	effectStars[1].maxColourDivergence_.z	= 0.75f;	
	effectStars[1].maxLifetimeDivergence_	= 5;		
	effectStars[1].maxSizeDivergence_		= 5.0f;	
	effectStars[1].maxVelocityDivergence_	= 20.0f;
	effectStars[1].numberOfRays_			= 50;
	effectStars[1].baseColour_				= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectStars[1].launchVelocity_			= 50.0f;
	effectStars[1].maxParticles_			= 1000;
	effectStars[1].origin_					= D3DXVECTOR3(0, 0, 0);
	effectStars[1].startInterval_			= 1;
	effectStars[1].startTimer_				= 0;
	effectStars[1].timeIncrement_			= 0.08f;
	effectStars[1].maxLifetime_				= 60;
	effectStars[1].startParticles_			= 1000;
	effectStars[1].maxParticleSize_			= 10.0f;
	effectStars[1].particleTexture_			= particle_circle;

	effectStars[2].fadeOutTime_				= 15;					
	effectStars[2].maxColourDivergence_.x	= 0.75f;	
	effectStars[2].maxColourDivergence_.y	= 0.0f;	
	effectStars[2].maxColourDivergence_.z	= 0.5f;	
	effectStars[2].maxLifetimeDivergence_	= 5;		
	effectStars[2].maxSizeDivergence_		= 5.0f;	
	effectStars[2].maxVelocityDivergence_	= 20.0f;
	effectStars[2].numberOfRays_			= 50;
	effectStars[2].baseColour_				= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectStars[2].launchVelocity_			= 50.0f;
	effectStars[2].maxParticles_			= 1000;
	effectStars[2].origin_					= D3DXVECTOR3(0, 0, 0);
	effectStars[2].startInterval_			= 1;
	effectStars[2].startTimer_				= 0;
	effectStars[2].timeIncrement_			= 0.08f;
	effectStars[2].maxLifetime_				= 60;
	effectStars[2].startParticles_			= 1000;
	effectStars[2].maxParticleSize_			= 10.0f;
	effectStars[2].particleTexture_			= particle_circle;

	effectStars[3].fadeOutTime_				= 15;					
	effectStars[3].maxColourDivergence_.x	= 0.0f;	
	effectStars[3].maxColourDivergence_.y	= 0.75f;	
	effectStars[3].maxColourDivergence_.z	= 0.0f;	
	effectStars[3].maxLifetimeDivergence_	= 5;		
	effectStars[3].maxSizeDivergence_		= 5.0f;	
	effectStars[3].maxVelocityDivergence_	= 20.0f;
	effectStars[3].numberOfRays_			= 50;
	effectStars[3].baseColour_				= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectStars[3].launchVelocity_			= 50.0f;
	effectStars[3].maxParticles_			= 1000;
	effectStars[3].origin_					= D3DXVECTOR3(0, 0, 0);
	effectStars[3].startInterval_			= 1;
	effectStars[3].startTimer_				= 0;
	effectStars[3].timeIncrement_			= 0.08f;
	effectStars[3].maxLifetime_				= 60;
	effectStars[3].startParticles_			= 1000;
	effectStars[3].maxParticleSize_			= 10.0f;
	effectStars[3].particleTexture_			= particle_circle;

	effectStars[4].fadeOutTime_				= 15;					
	effectStars[4].maxColourDivergence_.x	= 0.5f;	
	effectStars[4].maxColourDivergence_.y	= 0.0f;	
	effectStars[4].maxColourDivergence_.z	= 0.0f;	
	effectStars[4].maxLifetimeDivergence_	= 5;		
	effectStars[4].maxSizeDivergence_		= 5.0f;	
	effectStars[4].maxVelocityDivergence_	= 20.0f;
	effectStars[4].numberOfRays_			= 60;
	effectStars[4].baseColour_				= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectStars[4].launchVelocity_			= 50.0f;
	effectStars[4].maxParticles_			= 1000;
	effectStars[4].origin_					= D3DXVECTOR3(0, 0, 0);
	effectStars[4].startInterval_			= 1;
	effectStars[4].startTimer_				= 0;
	effectStars[4].timeIncrement_			= 0.08f;
	effectStars[4].maxLifetime_				= 60;
	effectStars[4].startParticles_			= 1000;
	effectStars[4].maxParticleSize_			= 10.0f;
	effectStars[4].particleTexture_			= particle_circle;

	//------------------------------------------------------------------------
	// effect cones

	effectCones[0].launchAngle				= 45.0f;
	effectCones[0].fadeOutTime_				= 15;					
	effectCones[0].maxColourDivergence_.x	= 0.0f;	
	effectCones[0].maxColourDivergence_.y	= 0.5f;	
	effectCones[0].maxColourDivergence_.z	= 0.5f;	
	effectCones[0].maxLifetimeDivergence_	= 15;		
	effectCones[0].maxSizeDivergence_		= 5.0f;	
	effectCones[0].maxVelocityDivergence_	= 10.0f;
	effectCones[0].baseColour_				= D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	effectCones[0].launchVelocity_			= 50.0f;
	effectCones[0].maxParticles_			= 200;
	effectCones[0].origin_					= D3DXVECTOR3(0, 0, 0);
	effectCones[0].startInterval_			= 1;
	effectCones[0].startTimer_				= 0;
	effectCones[0].timeIncrement_			= 0.08f;
	effectCones[0].maxLifetime_				= 100;
	effectCones[0].startParticles_			= 200; 
	effectCones[0].maxParticleSize_			= 12.0f;
	effectCones[0].particleTexture_			= particle_star;

	effectCones[1].launchAngle				= -45.0f;
	effectCones[1].fadeOutTime_				= 15;					
	effectCones[1].maxColourDivergence_.x	= 0.0f;	
	effectCones[1].maxColourDivergence_.y	= 0.5f;	
	effectCones[1].maxColourDivergence_.z	= 0.5f;	
	effectCones[1].maxLifetimeDivergence_	= 15;		
	effectCones[1].maxSizeDivergence_		= 5.0f;	
	effectCones[1].maxVelocityDivergence_	= 10.0f;
	effectCones[1].baseColour_				= D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	effectCones[1].launchVelocity_			= 50.0f;
	effectCones[1].maxParticles_			= 200;
	effectCones[1].origin_					= D3DXVECTOR3(0, 0, 0);
	effectCones[1].startInterval_			= 1;
	effectCones[1].startTimer_				= 0;
	effectCones[1].timeIncrement_			= 0.08f;
	effectCones[1].maxLifetime_				= 100;
	effectCones[1].startParticles_			= 200; 
	effectCones[1].maxParticleSize_			= 12.0f;
	effectCones[1].particleTexture_			= particle_star;

	//-------------------------------------------------------------------------
	// effect multisphere

	effectMultiSpheres[0].subExplosionSize_					= 100;
	effectMultiSpheres[0].subParticleBaseColour_			= D3DXCOLOR(1.0f, 1.0f, 0.0f, 0.0f);;
	effectMultiSpheres[0].subParticleFadeOutTime_			= 50;
	effectMultiSpheres[0].subParticleMaxVelocityDivergence_	= 10.0f;
	effectMultiSpheres[0].subParticleLaunchVelocity_		= 30.0f;
	effectMultiSpheres[0].subParticleMaxColourDivergence_.x	= 0.25f;	
	effectMultiSpheres[0].subParticleMaxColourDivergence_.y	= 0.25f;	
	effectMultiSpheres[0].subParticleMaxColourDivergence_.z	= 0.0f;	
	effectMultiSpheres[0].subParticleMaxLifetimeDivergence_	= 5;
	effectMultiSpheres[0].subParticleMaxLifetime_			= 100;
	effectMultiSpheres[0].subParticleMaxSizeDivergence_		= 2.0f;
	effectMultiSpheres[0].subParticleMaxSize_				= 10.0f;
	
	effectMultiSpheres[0].fadeOutTime_						= 30;					
	effectMultiSpheres[0].maxColourDivergence_.x			= 0.25f;	
	effectMultiSpheres[0].maxColourDivergence_.y			= 0.25f;	
	effectMultiSpheres[0].maxColourDivergence_.z			= 0.0f;	
	effectMultiSpheres[0].maxLifetimeDivergence_			= 5;		
	effectMultiSpheres[0].maxSizeDivergence_				= 5.0f;	
	effectMultiSpheres[0].maxVelocityDivergence_			= 25.0f;

	effectMultiSpheres[0].baseColour_						= D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f);
	effectMultiSpheres[0].launchVelocity_					= 70.0f;
	effectMultiSpheres[0].maxParticles_						= 3030;
	effectMultiSpheres[0].origin_							= D3DXVECTOR3(0, 0, 0);
	effectMultiSpheres[0].startInterval_					= 1;
	effectMultiSpheres[0].startTimer_						= 0;
	effectMultiSpheres[0].timeIncrement_					= 0.08f;
	effectMultiSpheres[0].maxLifetime_						= 50;
	effectMultiSpheres[0].startParticles_					= 30; 
	effectMultiSpheres[0].maxParticleSize_					= 15.0f;
	effectMultiSpheres[0].particleTexture_					= particle_circle;

	//------------------------------------------------------------------------------------
	// effect rays

	effectRays[0].subParticleBaseColour_				= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);;
	effectRays[0].subParticleFadeOutTime_				= 30;
	effectRays[0].subParticleMaxVelocityDivergence_		= 0.0f;
	effectRays[0].subParticleLaunchVelocity_			= 30.0f;
	effectRays[0].subParticleMaxColourDivergence_.x		= 0.5f;	
	effectRays[0].subParticleMaxColourDivergence_.y		= 0.5f;	
	effectRays[0].subParticleMaxColourDivergence_.z		= 0.5f;	
	effectRays[0].subParticleMaxLifetimeDivergence_		= 0;
	effectRays[0].subParticleMaxLifetime_				= 30;
	effectRays[0].subParticleMaxSizeDivergence_			= 0.0f;
	effectRays[0].subParticleMaxSize_					= 4.0f;
	
	effectRays[0].fadeOutTime_							= 20;					
	effectRays[0].maxColourDivergence_.x				= 0.0f;	
	effectRays[0].maxColourDivergence_.y				= 0.5f;	
	effectRays[0].maxColourDivergence_.z				= 0.5f;	
	effectRays[0].maxLifetimeDivergence_				= 5;		
	effectRays[0].maxSizeDivergence_					= 5.0f;	
	effectRays[0].maxVelocityDivergence_				= 25.0f;

	effectRays[0].baseColour_							= D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	effectRays[0].launchVelocity_						= 70.0f;
	effectRays[0].maxParticles_							= 4650;
	effectRays[0].origin_								= D3DXVECTOR3(0, 0, 0);
	effectRays[0].startInterval_						= 1;
	effectRays[0].startTimer_							= 0;
	effectRays[0].timeIncrement_						= 0.08f;
	effectRays[0].maxLifetime_							= 60;
	effectRays[0].startParticles_						= 150; 
	effectRays[0].maxParticleSize_						= 20.0f;
	effectRays[0].particleTexture_						= particle_circle;

	//----------------------------------------------------------------------------------
	// equip rockets

	rockets[0].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[0].projectile_		= &projectiles[0];
	rockets[0].trace_			= &traces[0];
	rockets[0].effect_			= &effectSpheres[0];

	rockets[1].startPosition_	= D3DXVECTOR3(-75, -300, 0);
	rockets[1].projectile_		= &projectiles[1];
	rockets[1].trace_			= &traces[1];
	rockets[1].effect_			= &effectSpheres[1];

	rockets[2].startPosition_	= D3DXVECTOR3(-25, -300, 0);
	rockets[2].projectile_		= &projectiles[2];
	rockets[2].trace_			= &traces[2];
	rockets[2].effect_			= &effectSpheres[2];

	rockets[3].startPosition_	= D3DXVECTOR3(25, -300, 0);
	rockets[3].projectile_		= &projectiles[3];
	rockets[3].trace_			= &traces[3];
	rockets[3].effect_			= &effectSpheres[3];

	rockets[4].startPosition_	= D3DXVECTOR3(75, -300, 0);
	rockets[4].projectile_		= &projectiles[4];
	rockets[4].trace_			= &traces[4];
	rockets[4].effect_			= &effectSpheres[4];

	rockets[5].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[5].projectile_		= &projectiles[5];
	rockets[5].trace_			= &traces[5];
	rockets[5].effect_			= &effectStars[0];

	rockets[6].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[6].projectile_		= &projectiles[6];
	rockets[6].trace_			= &traces[6];
	rockets[6].effect_			= &effectStars[1];

	rockets[7].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[7].projectile_		= &projectiles[7];
	rockets[7].trace_			= &traces[7];
	rockets[7].effect_			= &effectStars[2];

	rockets[8].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[8].projectile_		= &projectiles[8];
	rockets[8].trace_			= &traces[8];
	rockets[8].effect_			= &effectStars[3];

	rockets[9].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[9].projectile_		= &projectiles[9];
	rockets[9].trace_			= &traces[9];
	rockets[9].effect_			= &effectStars[4];

	rockets[10].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[10].projectile_		= &projectiles[10];
	rockets[10].trace_			= &traces[10];
	rockets[10].effect_			= &effectCones[0];

	rockets[11].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[11].projectile_		= &projectiles[11];
	rockets[11].trace_			= &traces[11];
	rockets[11].effect_			= &effectCones[1];

	rockets[12].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[12].projectile_		= &projectiles[12];
	rockets[12].trace_			= &traces[12];
	rockets[12].effect_			= &effectRays[0];

	rockets[13].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[13].projectile_		= &projectiles[13];
	rockets[13].trace_			= &traces[13];
	rockets[13].effect_			= &effectMultiSpheres[0];

	rockets[14].startPosition_	= D3DXVECTOR3(0, -300, 0);
	rockets[14].projectile_		= &projectiles[14];
	rockets[14].trace_			= &traces[14];
	rockets[14].effect_			= &effectSpheres[5];

	//------------------------------------------------------------------------------------
	// initialise rockets

	for(int i = 0; i < numberOfRockets; ++i)
	{
		rockets[i].initialise(device);
	}

}
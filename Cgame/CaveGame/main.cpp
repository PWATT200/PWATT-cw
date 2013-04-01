/*
=================
main.cpp
Main entry point for the Card application
=================
*/

#include "GameConstants.h"
#include "GameResources.h"
#include "cD3DManager.h"
#include "cD3DXSpriteMgr.h"
#include "cD3DXTexture.h"
#include "cSprite.h"
#include "cExplosion.h"
#include "cXAudio.h"
#include "cD3DXFont.h"

using namespace std;

HINSTANCE hInst; // global handle to hold the application instance
HWND wndHandle; // global variable to hold the window handle

// Get a reference to the DirectX Manager
static cD3DManager* d3dMgr = cD3DManager::getInstance();

// Get a reference to the DirectX Sprite renderer Manager 
static cD3DXSpriteMgr* d3dxSRMgr = cD3DXSpriteMgr::getInstance();

RECT clientBounds;

TCHAR szTempOutput[30];

bool lookingRight = true;

bool mainMenu = true;
	

bool gHit = false;

D3DXVECTOR3 mainMenuClick;

list<cExplosion*> gExplode;

D3DXVECTOR2 warriorTrans = D3DXVECTOR2(300,300);

RECT StartButton;  
RECT QuitButton; 

/*
==================================================================
* LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
* LPARAM lParam)
* The window procedure
==================================================================
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Check any available messages from the queue
	switch (message)
	{

		case WM_KEYDOWN:
			{
				if (wParam == VK_LEFT)
				{
					lookingRight = false;
					warriorTrans.x -= 5.0f;
					return 0;
				}
				if (wParam == VK_RIGHT)
				{
					lookingRight = true;
					warriorTrans.x += 5.0f;
					return 0;
				}
				return 0;
			}
		if(mainMenu == true){
			
			case WM_LBUTTONDOWN:
				{
					POINT mouseXY;
					mouseXY.x = LOWORD(lParam);
					mouseXY.y = HIWORD(lParam);
				
					mainMenuClick = D3DXVECTOR3((float)mouseXY.x,(float)mouseXY.y, 0.0f);

					StringCchPrintf(szTempOutput, STRSAFE_MAX_CCH, TEXT("Mouse: lLastX=%d lLastY=%d\r\n"), LOWORD(lParam), HIWORD(lParam));
					OutputDebugString(szTempOutput);
					return 0;
				}
		}

		case WM_CLOSE:
			{
			// Exit the Game
				PostQuitMessage(0);
				 return 0;
			}

		case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
	}
	// Always return the message to the default window
	// procedure for further processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
==================================================================
* bool initWindow( HINSTANCE hInstance )
* initWindow registers the window class for the application, creates the window
==================================================================
*/
bool initWindow( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;
	// Fill in the WNDCLASSEX structure. This describes how the window
	// will look to the system
	wcex.cbSize = sizeof(WNDCLASSEX); // the size of the structure
	wcex.style = CS_HREDRAW | CS_VREDRAW; // the class style
	wcex.lpfnWndProc = (WNDPROC)WndProc; // the window procedure callback
	wcex.cbClsExtra = 0; // extra bytes to allocate for this class
	wcex.cbWndExtra = 0; // extra bytes to allocate for this instance
	wcex.hInstance = hInstance; // handle to the application instance
	wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MyWindowIcon)); // icon to associate with the application
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_GUNSIGHT));// the default cursor
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // the background color
	wcex.lpszMenuName = NULL; // the resource name for the menu
	wcex.lpszClassName = "Balloons"; // the class name being created
	wcex.hIconSm = LoadIcon(hInstance,"Balloon.ico"); // the handle to the small icon

	RegisterClassEx(&wcex);
	// Create the window
	wndHandle = CreateWindow("Balloons",			// the window class to use
							 "The Cave",			// the title bar text
							WS_OVERLAPPEDWINDOW,	// the window style
							CW_USEDEFAULT, // the starting x coordinate
							CW_USEDEFAULT, // the starting y coordinate
							800, // the pixel width of the window
							600, // the pixel height of the window
							NULL, // the parent window; NULL for desktop
							NULL, // the menu for the application; NULL for none
							hInstance, // the handle to the application instance
							NULL); // no values passed to the window
	// Make sure that the window handle that is created is valid
	if (!wndHandle)
		return false;
	// Display the window on the screen
	ShowWindow(wndHandle, SW_SHOW);
	UpdateWindow(wndHandle);
	return true;
}

/*
==================================================================
// This is winmain, the main entry point for Windows applications
==================================================================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	// Initialize the window
	if ( !initWindow( hInstance ) )
		return false;
	// called after creating the window
	if ( !d3dMgr->initD3DManager(wndHandle) )
		return false;
	if ( !d3dxSRMgr->initD3DXSpriteMgr(d3dMgr->getTheD3DDevice()))
		return false;

	// Grab the frequency of the high def timer
	__int64 freq = 0;				// measured in counts per second;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	float sPC = 1.0f / (float)freq;			// number of seconds per count

	__int64 currentTime = 0;				// current time measured in counts per second;
	__int64 previousTime = 0;				// previous time measured in counts per second;

	float numFrames   = 0.0f;				// Used to hold the number of frames
	float timeElapsed = 0.0f;				// cumulative elapsed time

	GetClientRect(wndHandle,&clientBounds);

	float fpsRate = 1.0f/25.0f;

	LPDIRECT3DSURFACE9 aSurface;				// the Direct3D surface
	LPDIRECT3DSURFACE9 theBackbuffer = NULL;  // This will hold the back buffer
	
	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	// Game Font
	cD3DXFont* caveMenuFont = new cD3DXFont(d3dMgr->getTheD3DDevice(),hInstance, "menuFont");

	// Create the background surface
	aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\menu.png");

	// Creat Warrior
	D3DXVECTOR3 warriorPos = D3DXVECTOR3(300,300,0);
	
	cSprite theWarrior(warriorPos,d3dMgr->getTheD3DDevice(),"Images\\warriorR.png");
	theWarrior.setTranslation(D3DXVECTOR2(5.0f,0.0f));

	while( msg.message!=WM_QUIT )
	{
		// Check the message queue
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			// Game code goes here
			QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
			float dt = (currentTime - previousTime)*sPC;

			

			if(mainMenu == false){
				aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\Nighttime.png");
					if(lookingRight == true){
						theWarrior.setTexture(d3dMgr->getTheD3DDevice(),"Images\\warriorR.png");
				
					}else {
						theWarrior.setTexture(d3dMgr->getTheD3DDevice(),"Images\\warriorL.png");
					}
			}else{
				aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\menu.png");
				SetRect(&StartButton, 300, 200, 500, 260); 
				SetRect(&QuitButton, 300, 300, 500, 360); 

			}

			// Accumulate how much time has passed.
			timeElapsed += dt;

			if(timeElapsed > fpsRate)
			{
				warriorPos = D3DXVECTOR3(warriorTrans.x,warriorTrans.y,0);
				theWarrior.setSpritePos(warriorPos);


				d3dMgr->beginRender();
				theBackbuffer = d3dMgr->getTheBackBuffer();
				d3dMgr->updateTheSurface(aSurface, theBackbuffer);
				d3dMgr->releaseTheBackbuffer(theBackbuffer);
				
				d3dxSRMgr->beginDraw();
				
				if (mainMenu == false)
					d3dxSRMgr->drawSprite(theWarrior.getTexture(),NULL,NULL,&theWarrior.getSpritePos(),0xFFFFFFFF);
			
				d3dxSRMgr->endDraw();
				
				d3dMgr->endRender();
				OutputDebugString("timeElapsed > fpsRate");
				timeElapsed = 0.0f;
			}

			previousTime = currentTime;
			StringCchPrintf(szTempOutput, 30, TEXT("dt=%f\n"), dt);
			OutputDebugString(szTempOutput);
			StringCchPrintf(szTempOutput, 30, TEXT("timeElapsed=%f\n"), timeElapsed);
			OutputDebugString(szTempOutput);
			StringCchPrintf(szTempOutput, 30, TEXT("previousTime=%u\n"), previousTime);
			OutputDebugString(szTempOutput);
			StringCchPrintf(szTempOutput, 30, TEXT("fpsRate=%f\n"), fpsRate);
			OutputDebugString(szTempOutput);
		}
	}
	d3dxSRMgr->cleanUp();
	d3dMgr->clean();
	return (int) msg.wParam;
}

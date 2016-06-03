#include "BaseSystem.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include "../EnGDemo/DemoScene.h"
#include "..\EnGineUtil\CVars.h"

BOOL		BaseSystem::DEBUG_RENDER_TARGETS = FALSE;

BaseSystem::BaseSystem()
{
	m_OpenGL		= NULL;
	m_pParserXML	= NULL;
	m_pScene		= NULL;
	m_pRndMng		= NULL;	
	m_pRC			= NULL;
	m_pProgLoading	= NULL;
	m_pSettingsXML	= NULL;
	m_fTimeToLoad	= 0.0f;
	m_fMaxTimeToLoad= 0.0f;
	m_uiLoadingID	= NULL;
	m_bLoadingFinish = true;
}

BaseSystem::BaseSystem( const BaseSystem& )
{
	
}

BaseSystem::~BaseSystem()
{
	
}

int BaseSystem::ReceiveMessage( BasicMsg *msg, const void *sender )
{
	return 1;
}

bool BaseSystem::InitSys()
{
	int screenWidth, screenHeight;
	BOOL result;
	RedirectIOToConsole();
	ifstream myReadFile;
	myReadFile.open("Data_Select.txt");
	char output[100];
	if (myReadFile.is_open()) 
	{
		while (!myReadFile.eof()) 
		{
			myReadFile >> output;
			if (output[0] != '#')
				break;
		}
	}
	myReadFile.close();
	
	string strDataPath(output);
	ENG::FULL_PATH = strDataPath;
	ENG::FULL_PATH_LOGGER_FILE = strDataPath + "Log\\Logger.txt";

	myReadFile.open(strDataPath + "case_load.txt");
	char outputCase[100];
	if (myReadFile.is_open()) 
	{
		while (!myReadFile.eof()) 
		{
			myReadFile >> outputCase;
			if (outputCase[0] != '#')
				break;
		}
	}
	myReadFile.close();

	string strCasePath(outputCase);
	ENG::FULL_PATH_XML_FILE = strDataPath + strCasePath;
	// Initialize the width and height of the screen to zero.
	screenWidth = 0;
	screenHeight = 0;

	if (!m_pParserXML)
		m_pParserXML = new ParserXML();
	if (!m_pSettingsXML)
		m_pSettingsXML = m_pParserXML->LoadFlie((ENG::FULL_PATH_XML_FILE).c_str(), TRUE);
	ENG::InitENG(m_pSettingsXML);
	
	
	//////////////////////////////////////////////////////////////////////////
	// Step 1 - Initialize OpenGL.
	m_OpenGL = new OGL4();	
	result = InitializeWindows(m_OpenGL, screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the window.", L"Error", MB_OK);
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Step 2 - Initialize Scene Objects and Manager.
	string strSceneName = "";
	m_pSettingsXML->ReadAttributeDef("SceneName", "", strSceneName);
	if (strSceneName == "Demo")
		m_pScene = new DemoScene();
	else
		m_pScene = new SceneBase();
	result = m_pScene->InitScene(m_pSettingsXML);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the scene.", L"Error", MB_OK);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Step 3 - Initialize Renderer.
	string strRendererXNL= "";
	CChunk* chRenderer = m_pSettingsXML->GetChunkUnique("Renderer");
	chRenderer->ReadAttributeDef("RendererPath", "", strRendererXNL);

	ParserXML pRendererXML;
	CChunk* pRendererChunk = pRendererXML.LoadFlie((ENG::FULL_PATH + strRendererXNL).c_str(), TRUE);
	m_pRndMng = new RenderManager();
	result = m_pRndMng->InitRenderer(m_OpenGL, pRendererChunk, m_pScene->GetCamera());
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the renderer.", L"Error", MB_OK);
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Step 3.1 - Set VAOs for all objects.
	vector<PBasicObj>& arrObjects =	m_pScene->GetObjects();
	UINT nObjs = arrObjects.size();
	CChunk* chObjects = m_pSettingsXML->GetChunkUnique("RenderContexts");
	for (UINT uiO = 0; uiO < nObjs; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		result = m_pRndMng->LoadObjectVAO(pObj);
		if(!result)
		{
			MessageBox(m_hwnd, L"Could not create object VAO for scene object.", L"Error", MB_OK);
			return false;
		}
	}

		
	
	// Step 4 - Init Renderer Contexts
	//////////////////////////////////////////////////////////////////////////
	TIC();
 	
	CChunk* chSyncRender = pRendererChunk->GetChunkUnique("SyncRender");	
	if (!chSyncRender)
		return false;
	PRINT_INFO("========| Loading Renderer Contexts");		
	string strType;
	CChunk* chRC = chSyncRender->GetChunkUnique("RenderContext");
	if (!chRC)
	{
		TOC("Loading Renderer Contexts Fail!!!");
		return false;
	}
	chRC->ReadChildValue("Type", strType);
	m_pRC = new RenderSequenceBasic();
	m_pRC->Init(m_pRndMng, m_OpenGL, chRC, arrObjects); 	
	
	TOC("Loading Renderer Contexts Completed... ");
	//////////////////////////////////////////////////////////////////////////
		
	// Step 5 - Init Renderer Contexts
	//////////////////////////////////////////////////////////////////////////
	TIC();
	m_pRndMng->InitVisualAids(m_pScene->GetVisAidMeshes());
	TOC("Loading Visual Aids Completed... ");
	//////////////////////////////////////////////////////////////////////////

	TIC();
	// Step 6 - Post Init Scene 
	//////////////////////////////////////////////////////////////////////////
	result = m_pScene->PostInitScene(m_pSettingsXML);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the scene.", L"Error", MB_OK);
		TOC("Loading Visual Aids Fail!!!");
		//////////////////////////////////////////////////////////////////////////
		return false;
	}	
	m_pRndMng->PostInitRenderer(m_pSettingsXML);	
	m_pRC->PostInit(); 		
	TOC("Post Init Renderer Completed... ");
	//////////////////////////////////////////////////////////////////////////

	
	return true;
}

bool BaseSystem::InitSysBeforeLoading()
{
	RedirectIOToConsole();
	m_bLoadingFinish = false;
	int screenWidth, screenHeight;
	BOOL result;
	ifstream myReadFile;
	myReadFile.open("Data_Select.txt");
	char output[100];
	if (myReadFile.is_open()) 
	{
		while (!myReadFile.eof()) 
		{
			myReadFile >> output;
			if (output[0] != '#')
				break;
		}
	}
	myReadFile.close();
	string strDataPath(output);
	ENG::FULL_PATH = strDataPath;
	ENG::FULL_PATH_LOGGER_FILE = strDataPath + "Log\\Logger.txt";
	ENG::FULL_PATH_XML_FILE = strDataPath + "Scripts\\settings.xml";
	// Initialize the width and height of the screen to zero.
	screenWidth = 0;
	screenHeight = 0;
	
	if (!m_pParserXML)
		m_pParserXML = new ParserXML();
	if (!m_pSettingsXML)
		m_pSettingsXML = m_pParserXML->LoadFlie((ENG::FULL_PATH_XML_FILE).c_str(), TRUE);
	ENG::InitENG(m_pSettingsXML);
	
	m_OpenGL = new OGL4();	
	result = InitializeWindows(m_OpenGL, screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the window.", L"Error", MB_OK);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Step 1 - Initialize OpenGL.
	ResourceMng::GetInstance()->SetOpenGL(m_OpenGL);
	
	//////////////////////////////////////////////////////////////////////////
	// Step 2 - Initialize loading scene
	CChunk* chLoading = m_pSettingsXML->GetChunkUnique("Loading");
	string strShaderName = "";
	chLoading->ReadChildValue("Shader", strShaderName);
	string strTextureName = "";
	chLoading->ReadChildValue("Texture", strTextureName);	
	chLoading->ReadChildValue("Time", m_fMaxTimeToLoad);

	m_pProgLoading = new GLSLProgram(m_OpenGL); 
	string strVert = strDataPath + strShaderName + ".vert";
	string strFrag = strDataPath + strShaderName + ".frag";
	if (!m_pProgLoading->Init("Load", strVert, strFrag))
	{
		PRINT_ERROR("Loading Shader Fail");
	}

	m_pRndMng = new RenderManager();
	m_pRndMng->InitRendererLite(m_OpenGL);


	
	/*TextureInfo texInfo;
	texInfo.m_eFilterType = E_TF_BILINEAR;
	texInfo.m_strPath = strDataPath + strTextureName;
	m_uiLoadingID = ResourceMng::GetInstance()->LoadTextureTGA(1, &texInfo);*/

	TextureInfo texInfo;
	texInfo.m_eFilterType = E_TF_BILINEAR;
	texInfo.m_strPath = strDataPath + strTextureName;
	texInfo.m_iWidth = 800;
	texInfo.m_iHeight = 600;
	m_uiLoadingID = new GLuint[1];
	m_uiLoadingID[0] = ResourceMng::GetInstance()->LoadTextureRaw(texInfo);

	return true;
}

bool BaseSystem::InitSysAfterLoading()
{
	// release loading
// 	delete m_pProgLoading;
// 	glDeleteTextures(1, m_uiLoadingID);

	//////////////////////////////////////////////////////////////////////////
	// Step 2 - Initialize Scene Objects and Manager.
	if (!m_pParserXML)
		m_pParserXML = new ParserXML();
	if (!m_pSettingsXML)
		m_pSettingsXML = m_pParserXML->LoadFlie((ENG::FULL_PATH_XML_FILE).c_str(), TRUE);
	
	string strSceneName = "";
	m_pSettingsXML->ReadAttributeDef("SceneName", "", strSceneName);
	if (strSceneName == "Demo")
		m_pScene = new DemoScene();
	else
		m_pScene = new SceneBase();
	bool result = m_pScene->InitScene(m_pSettingsXML);
 
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the scene.", L"Error", MB_OK);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Step 3 - Initialize Renderer.
	string strRendererXNL= "";
	CChunk* chRenderer = m_pSettingsXML->GetChunkUnique("Renderer");
	chRenderer->ReadAttributeDef("RendererPath", "", strRendererXNL);

	ParserXML pRendererXML;
	CChunk* pRendererChunk = pRendererXML.LoadFlie((ENG::FULL_PATH + strRendererXNL).c_str(), TRUE);
	if (!m_pRndMng)
		m_pRndMng = new RenderManager();
	result = m_pRndMng->InitRenderer(m_OpenGL, pRendererChunk, m_pScene->GetCamera());
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the renderer.", L"Error", MB_OK);
		return false;
	}	

	//////////////////////////////////////////////////////////////////////////
	// Step 3.1 - Set VAOs for all objects.
	vector<PBasicObj>& arrObjects =	m_pScene->GetObjects();
	UINT nObjs = arrObjects.size();
	CChunk* chObjects = m_pSettingsXML->GetChunkUnique("RenderContexts");
	for (UINT uiO = 0; uiO < nObjs; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		result = m_pRndMng->LoadObjectVAO(pObj);
		if(!result)
		{
			MessageBox(m_hwnd, L"Could not create object VAO for scene object.", L"Error", MB_OK);
			return false;
		}
		
	}


	// Step 4 - Init Renderer Contexts
	//////////////////////////////////////////////////////////////////////////
	TIC();

	CChunk* chSyncRender = pRendererChunk->GetChunkUnique("SyncRender");	
	if (!chSyncRender)
		return false;
	PRINT_INFO("========| Loading Renderer Contexts");		
	string strType;
	CChunk* chRC = chSyncRender->GetChunkUnique("RenderContext");
	if (!chRC)
	{
		TOC("Loading Renderer Contexts Fail!!!");
		return false;
	}
	chRC->ReadChildValue("Type", strType);
	m_pRC = new RenderSequenceBasic();
	m_pRC->Init(m_pRndMng, m_OpenGL, chRC, arrObjects); 	

	TOC("Loading Renderer Contexts Completed... ");
	//////////////////////////////////////////////////////////////////////////

	// Step 5 - Init Renderer Contexts
	//////////////////////////////////////////////////////////////////////////
	TIC();
	m_pRndMng->InitVisualAids(m_pScene->GetVisAidMeshes());
	TOC("Loading Visual Aids Completed... ");
	//////////////////////////////////////////////////////////////////////////

	TIC();
	// Step 6 - Post Init Scene 
	//////////////////////////////////////////////////////////////////////////
	result = m_pScene->PostInitScene(m_pSettingsXML);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the scene.", L"Error", MB_OK);
		TOC("Loading Visual Aids Fail!!!");
		//////////////////////////////////////////////////////////////////////////
		return false;
	}	
	m_pRndMng->PostInitRenderer(m_pSettingsXML);	
	m_pRC->PostInit(); 		
	
	TOC("Post Init Renderer Completed... ");
	//////////////////////////////////////////////////////////////////////////

	
	return true;
}

void BaseSystem::Shutdown()
{	
	if (m_pProgLoading)
		delete m_pProgLoading;
	if (m_uiLoadingID)
		glDeleteTextures(1, m_uiLoadingID);


	// Release all Vertex buffer Objects 
	vector<PBasicObj>& arrObjects =	m_pScene->GetObjects();
	UINT nObjs = arrObjects.size();
	for (UINT uiO = 0; uiO < nObjs; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		m_pRndMng->ReleaseObjectVAO(pObj);		
	}
	
	
	// Release the OpenGL Scene.
	if (m_pScene)
	{
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}
	if (m_pRndMng)
	{
		delete m_pRndMng;
		m_pRndMng = NULL;
	}
	// Release the OpenGL object.
	if(m_OpenGL)
	{
		m_OpenGL->Shutdown(m_hwnd);
		delete m_OpenGL;
		m_OpenGL = NULL;
	}

	if (m_pRC)
		delete m_pRC;
	if (m_pParserXML)
		delete m_pParserXML;

	MessageMng::MsgMng()->Kill();

	Logger::Kill();

	// Shutdown the window.
	ShutdownWindows();

	return;
}

void BaseSystem::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Update();
			if(!result)
			{
				done = true;
			}
		}

	}

	return;
} 

void BaseSystem::RunLoading()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = UpdateLoading();
			if(!result)
			{
				done = true;
			}
		}

	}

	return;
}

bool BaseSystem::Update()
{
	// Set focus state
	//================================
	HWND WINAPI myHandle = GetForegroundWindow();
	ENG::bFocused = m_hwnd == myHandle;	
	if (KEYDOWN(VK_ESCAPE))
		return false;
	VA->ClearAll();
	GLTEXT->ClearAll();
	ENG::GetInstance()->UpdateKeyboardAndMouse();
	SetRenderContext();	
	//================================

	// Camera
	//================================
	m_pScene->PlayCamera();
	
	// Update
	//================================
	m_pScene->Update();
	
	// Post Update
	//================================
	m_pScene->PostUpdate();
	
	

	//////////////////////////////////////////////////////////////////////////
	// RENDER ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	m_pRndMng->SetSceneObjects(&m_pScene->GetObjects());

	m_pRndMng->PreRenderUpdate(m_pScene->GetObjects());
	
	if (SceneBase::WIREFRAME_MODE)
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	m_pRC->Render();

	if (SceneBase::WIREFRAME_MODE)
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	if (DEBUG_RENDER_TARGETS)
		m_pRC->DebugRender();

	m_pRndMng->RenderFont(NULL);

	glFlush();
	m_OpenGL->SwapBuffersHDC();
	
	return true; 

}

bool BaseSystem::UpdateLoading()
{		
	if (m_bLoadingFinish)
		return Update();
	
	ENG::GetInstance()->CalculateFPS();
	
	RenderLoading();

	if (m_fTimeToLoad > m_fMaxTimeToLoad)
	{
		m_bLoadingFinish = true;
		InitSysAfterLoading();		
	}

	return true; 

}

bool BaseSystem::RenderLoading()
{
	// Set the color to clear the screen to.
	glClearColor(0.9333f, 0.9333f, 0.9333f, 1.0f); 	
	// Clear the screen and depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	m_pRndMng->RenderLoadingQuad(m_uiLoadingID[0], m_pProgLoading, m_fTimeToLoad / m_fMaxTimeToLoad, 800,600);
	m_fTimeToLoad += ENG::GetDT();
	glFlush();	
	m_OpenGL->SwapBuffersHDC();
	return true; 
}

bool BaseSystem::InitializeWindows(OGL4* OpenGL, int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;
	bool result;

	// Init Scene and OpenGL settings from XML

	// Get an external pointer to this object.	
	ApplicationHndl = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	std::wstring engName = ConvertToWs(ENG::APPLICATION_NAME);

	const char* cAppName = ENG::APPLICATION_NAME.c_str();// 
	m_applicationName =  ConvertToWs(ENG::APPLICATION_NAME);

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName.c_str();
	wc.cbSize        = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Create a temporary window for the OpenGL extension setup.
 	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName.c_str(), m_applicationName.c_str(), WS_POPUP,
 		0, 0, 640, 480, NULL, NULL, m_hinstance, NULL);

	if(m_hwnd == NULL)
	{
		return false;
	}

	// Don't show the window.
	ShowWindow(m_hwnd, SW_HIDE);

	// Initialize a temporary OpenGL window and load the OpenGL extensions.
	result = OpenGL->InitializeExtensions(m_hwnd);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the OpenGL extensions.", L"Error", MB_OK);
		return false;
	}

	// Release the temporary window now that the extensions have been initialized.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Determine the resolution of the clients desktop screen.
	screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	
	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if(ENG::FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		
		ENG::SCR_WIDTH = screenWidth;
		ENG::SCR_HEIGHT = screenHeight;
		ENG::SCR_HALF_WIDTH = screenWidth / 2;
		ENG::SCR_HALF_HEIGHT = screenHeight / 2;
		ENG::SCR_WID_INV = 1.0f / static_cast<float>(ENG::SCR_WIDTH);
		ENG::SCR_HEI_INV = 1.0f / static_cast<float>(ENG::SCR_HEIGHT);	

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth  = ENG::SCR_WIDTH;
		screenHeight = ENG::SCR_HEIGHT;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}
	ENG::SCR_POS_X = static_cast<float>(posX);
	ENG::SCR_POS_Y = static_cast<float>(posY);
	
	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName.c_str(), m_applicationName.c_str(), WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);
	if(m_hwnd == NULL)
	{
		return false;
	}

	// Initialize OpenGL now that the window has been created.
	result = m_OpenGL->InitializeOpenGL(m_hwnd, screenWidth, screenHeight, ENG::VERT_SYNC);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize OpenGL, check if video card supports OpenGL 4.0.", L"Error", MB_OK);
		return false;
	}

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	//SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	//ShowCursor(false);
	
	return true;
	

}

void BaseSystem::ShutdownWindows()
{
	// Show the mouse cursor.
	//ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(ENG::FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName.c_str(), m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHndl = NULL;

	return;
}

LRESULT CALLBACK BaseSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch(umsg)
	{
		// Check if a key has been pressed on the keyboard.
	case WM_KEYDOWN:
	{
		CvarMng::GetInstance()->HandleInput(wparam);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		ENG::MOUSE_LBUTTON = true;
		ENG::MOUSE_LBUTTONUP = true;
		return 0;
	}
	case WM_LBUTTONUP:
	{
		ENG::MOUSE_LBUTTON = false;
		return 0;
	}
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

bool BaseSystem::InitScene()
{

	return true;
}

void BaseSystem::SetRenderContext()
{	
	BOOL bStereo = m_pRndMng->GetStereo();

	if (KEYSTATE(VK_F4) == eKeyGoingDown)
	{	
		DEBUG_RENDER_TARGETS  = !DEBUG_RENDER_TARGETS;
	}	

	if (KEYSTATE(VK_F5) == eKeyGoingDown)
	{
		CBaseRT::bSTEREO = !CBaseRT::bSTEREO;
		IRenderTargetBase::s_STEREO = !IRenderTargetBase::s_STEREO;
		m_pRndMng->SetStereo(CBaseRT::bSTEREO);
	}
}

void BaseSystem::RedirectIOToConsole()
{
	//setenv("K_ASSERT_ON", "0", 0);
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&coninfo);
	coninfo.dwSize.Y = /*MAX_CONSOLE_LINES*/500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);
	
	//coninfo.wAttributes = BACKGROUND_BLUE;
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | BACKGROUND_RED);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );

	*stdout = *fp;

	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console

	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );

	*stderr = *fp;

	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	ios::sync_with_stdio();
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		// Check if the window is being closed.
	case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}
	
	default:
		{
			// All other messages pass to the message handler in the system class.
			return ApplicationHndl->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}
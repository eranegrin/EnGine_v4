#pragma once

#ifndef BASE_SYSTEM_H
#define BASE_SYSTEM_H

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <assert.h>
#include "OGL4.h"
#include "BaseRenderer.h"
#include "../EnGBasic/ParserXML.h"
#include "../EnGBasic/EnGBasic.h"
#include "../EnGBasic/SceneBase.h"

using namespace EnG;
// #define KEYDOWN(vkCode)			((GetAsyncKeyState(vkCode) & 0x8000) ? 1 : 0)
// #define KEYUP(vkCode)			((GetAsyncKeyState(vkCode) & 0x8000) ? 0 : 1)

class BaseSystem : public IMessager
{
public:
	BaseSystem();
	BaseSystem(const BaseSystem&);
	~BaseSystem();

	virtual int				ReceiveMessage(BasicMsg *msg, const void *sender);

	bool					InitSys();
	bool					InitSysBeforeLoading();
	bool					InitSysAfterLoading();
	bool					InitScene();
	void					Shutdown();
	void					Run();
	void					RunLoading();
	void					RedirectIOToConsole();
	LRESULT CALLBACK		MessageHandler(HWND, UINT, WPARAM, LPARAM);

protected:
	bool					Update();
	bool					UpdateLoading();
	bool					RenderLoading();
	bool					InitializeWindows(OGL4*, int&, int&);
	void					ShutdownWindows();
	
	
protected:
	void					SetRenderContext();
	wstring					m_applicationName;
	HINSTANCE				m_hinstance;
	HWND					m_hwnd;
	OGL4*					m_OpenGL;
	PParserXML				m_pParserXML;
	PSceneBase				m_pScene;
	PRenderManager			m_pRndMng;
	PBaseRenderer			m_pRC;	
	
	//////////////////////////////////////////////////////////////////////////
	// Loading Frame
	float					m_fMaxTimeToLoad;
	float					m_fTimeToLoad;
	bool					m_bLoadingFinish;
	GLuint*					m_uiLoadingID;
	GLSLProgram*			m_pProgLoading;
	CChunk*					m_pSettingsXML;

	//////////////////////////////////////////////////////////////////////////
	// Debug
	static BOOL				DEBUG_RENDER_TARGETS;

};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/////////////
// GLOBALS //
/////////////
static BaseSystem* ApplicationHndl = NULL;

#endif //BASE_SYSTEM_H
#pragma once

#ifndef BASE_RENDERER_H
#define BASE_RENDERER_H

#include "BaseRT.h"

class RenderSequenceBasic
{
public:
	RenderSequenceBasic();
	~RenderSequenceBasic();

	BOOL				Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	VOID				PostInit();
	VOID				Render();
	VOID				DebugRender();
	VOID				Update(void* pData = NULL);
protected:
	RenderManager*		m_pRndMng;					// Pointer to render manager
	OGL4*				m_pOpenGL;					// Pointer to OpenGL interface
	ResourceMng*		m_pResMng;					// Pointer to Resource manager
	string				m_strName;					// Name of the render context	
	vector<PBaseRT>		m_arrRenderTargets;			// array of render targets for this sequence
};

typedef RenderSequenceBasic* PBaseRenderer;

#endif


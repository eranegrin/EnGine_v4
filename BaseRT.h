#pragma once

#include "RenderManager.h"
#include "../EnGBasic/EnGBasic.h"

enum E_RENDER_TARGETS
{
	eRT_DEPTH,
	eRT_STENCIL,
	eRT_CAMERA_DEPTH,
	eRT_SCENE,
	eRT_SCENE_MS,
	eRT_RESOLVER,
	eRT_POST,
};
class CBaseRT
{
public:
	CBaseRT();
	virtual ~CBaseRT();

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>&	arrObjects);
	virtual VOID		InitClearOp(CChunk* pClearChunk);
	virtual VOID		PostInit();
	virtual VOID		Render() = 0;	
	virtual VOID		DebugRender();	
	BOOL				GetSamplerInfo(CChunk& pSampChunk, PBaseShaderInput pSampInfo);

	E_RENDER_TARGETS	m_eType;
	string				m_strType;	
	OGL4*				m_pGL;
	RenderManager*		m_pRndMng;					// Pointer to render manager
	ResourceMng*		m_pResMng;					// Pointer to Resource manager
	IRenderTargetBase*	m_pRenderTarget;
	
	static bool			bSTEREO;
	static int			iRTCounter;

protected:
	VOID				InitShaderInputs(PRenderedObj pRndObject, CChunk* pSaplersChunk);
	VOID				InitShaderSubroutines(PRenderedObj pRndObject, CChunk* pSubroutinesChunk);
	VOID				SetDefaultShaderSubroutines(PRenderedObj pRndObject, const vector<string>& arrSubRoutines);
	VOID				InitShaderShadowData(PRenderedObj pRndObject, CChunk* pShadowChunk);

};
typedef CBaseRT* PBaseRT;

class DepthRT : public CBaseRT
{
public:
	DepthRT() : CBaseRT() 
	{
		m_eType = eRT_DEPTH;
		m_strType = "Depth";
	}
	virtual ~DepthRT() { }	

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	virtual VOID		Render();	
	virtual VOID		DebugRender();	
	

};

class SceneRT : public CBaseRT
{
public:
	SceneRT() : CBaseRT() 
	{
		m_eType = eRT_SCENE_MS;
		m_strType = "Scene MS";
	}
	virtual ~SceneRT() { }

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	virtual VOID		Render();		
	

};

class SceneMSRT : public CBaseRT
{
public:
	SceneMSRT() : CBaseRT() 
	{
		m_eType = eRT_SCENE_MS;
		m_strType = "Scene MS";
	}
	virtual ~SceneMSRT() { }

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	virtual VOID		Render();		
	virtual VOID		DebugRender();


};


class ResolverRT : public CBaseRT
{
public:
	ResolverRT() : CBaseRT() 
	{
		m_eType = eRT_RESOLVER;
		m_strType = "Resolver";
	}
	virtual ~ResolverRT() { }

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	virtual VOID		Render();	

	string				m_strRightFBO;
};

class PostRT : public CBaseRT
{
public:
	PostRT() : CBaseRT() 
	{
		m_eType = eRT_POST;
		m_strType = "Post Process";
	}
	virtual ~PostRT() { }

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	virtual VOID		Render();	
	virtual VOID		DebugRender();	
};
class ReflectRT : public CBaseRT
{
public:
	ReflectRT() : CBaseRT() 
	{
		m_eType = eRT_STENCIL;
		m_strType = "Depth";
	}
	virtual ~ReflectRT() { }	

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	virtual VOID		Render();	

};

class CameraDepthRT : public CBaseRT
{
public:
	CameraDepthRT() : CBaseRT() 
	{
		m_eType = eRT_CAMERA_DEPTH;
		m_strType = "Water";
	}
	virtual ~CameraDepthRT() { }

	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
	virtual VOID		Render();	
	string				m_strFBONoWater;

};
// 
// class StencilRT : public CBaseRT
// {
// public:
// 	StencilRT () : CBaseRT() 
// 	{
// 		m_eType = eRT_STENCIL;
// 		m_strType = "Depth";
// 	}
// 	virtual ~StencilRT() { }	
// 
// 	virtual VOID		Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects);
// 	virtual VOID		Render();	
// 
// };

__inline	PBaseRT	AllocateRenderTarget(const string& strType)
{
	if (strType == "Depth")
	{
		return new DepthRT;
	}
// 	else if (strType == "Stencil")
// 	{
// 		return new StencilRT;
// 	}
	else if (strType == "CamDepth")
	{
		return new CameraDepthRT;
	}
	else if (strType == "Scene")
	{
		return new SceneRT;
	}
	else if (strType == "SceneMS")
	{
		return new SceneMSRT;
	}
	else if (strType == "Resolver")
	{
		return new ResolverRT;
	}
	else if (strType == "Reflect")
	{
		return new ReflectRT;
	}
	else if (strType == "Post")
	{
		return new PostRT;
	}
	return NULL;
}

__inline IRenderTargetBase* AllocateRenderTargetInput(E_RENDER_TARGETS	m_eType, OGL4* pGL)
{
	if(m_eType == eRT_SCENE_MS || m_eType == eRT_SCENE)
		return new RenderTargetScene(pGL);
	else if (m_eType == eRT_POST)
		return new RenderTargetPost(pGL);
	
	return new RenderTargetScene(pGL);
}
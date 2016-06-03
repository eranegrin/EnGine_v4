#include "BaseRenderer.h"


RenderSequenceBasic::RenderSequenceBasic()
	: m_pRndMng(NULL), m_pOpenGL(NULL), m_pResMng(NULL), m_strName("Basic Render")
{

}

RenderSequenceBasic::~RenderSequenceBasic()
{
	UINT nRT = m_arrRenderTargets.size();
	for (UINT uiRT = 0; uiRT < nRT;uiRT++)
		delete m_arrRenderTargets[uiRT];
}

BOOL RenderSequenceBasic::Init( RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects )
{
	if (!a_pChunk )
		return FALSE;
	m_pRndMng = pRndMng; 	
	m_pResMng = ResourceMng::GetInstance();
	m_pOpenGL = pGL;

	a_pChunk->ReadChildValue("Name", m_strName);	

	//const UINT nRT = a_pChunk->GetChunckCount("RT");
	UINT nRenderTargets = a_pChunk->GetChunckCount();
	for (UINT uiRT = 0; uiRT < nRenderTargets; uiRT++)
	{
		CChunk cRT = a_pChunk->GetChunkXMLByID(uiRT);
		string strCName = cRT.GetNode().name();		
		if (strCName.find_first_of('_') == 0)
			continue;

		int iCCount = cRT.GetChunckCount();
		if (cRT.GetChunckCount() <= 1)
			continue;
		
		string strType;
		cRT.ReadChildValue("Type", strType);
		PBaseRT pRT = AllocateRenderTarget(strType);
		if (pRT)
		{
			pRT->Init(m_pRndMng, pGL, &cRT, arrObjects);
			m_arrRenderTargets.push_back(pRT);
		}
	}	
	return TRUE;
}


VOID RenderSequenceBasic::PostInit()
{
	UINT nRT = m_arrRenderTargets.size();
	for (UINT uiRT = 0; uiRT < nRT ; uiRT++)
	{
		m_arrRenderTargets[uiRT]->PostInit();
		GL_ERROR();
	}
}

VOID RenderSequenceBasic::Render( )
{
	UINT nRT = m_arrRenderTargets.size();
	for (UINT uiRT = 0; uiRT < nRT ; uiRT++)
	{
		m_arrRenderTargets[uiRT]->Render();
		GL_ERROR();
	}
	/*m_pRndMng->RenderFont();*/
}

VOID RenderSequenceBasic::DebugRender()
{
	UINT nRT = m_arrRenderTargets.size();
	for (UINT uiRT = 0; uiRT < nRT ; uiRT++)
	{
		m_arrRenderTargets[uiRT]->DebugRender();
		GL_ERROR();
	}
}

VOID RenderSequenceBasic::Update( void* pData /*= NULL*/ )
{

}

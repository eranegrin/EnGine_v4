#include "BaseRT.h"

bool				CBaseRT::bSTEREO = false;
int					CBaseRT::iRTCounter = 0;

CBaseRT::CBaseRT()
{
	m_pGL = NULL;
	m_pRndMng = NULL;
	m_pResMng = NULL;
	m_pRenderTarget = NULL;	
}

CBaseRT::~CBaseRT()
{
	if(m_pRenderTarget)
		delete m_pRenderTarget;
}

VOID CBaseRT::Init( RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects )
{
	if (!a_pChunk )
		return;
	m_pRndMng = pRndMng;
	m_pResMng = ResourceMng::GetInstance();// 
	m_pGL = pGL;
	assert(m_pRndMng);
	assert(m_pResMng);
	assert(m_pGL);
	
	m_pRenderTarget = AllocateRenderTargetInput(m_eType, m_pGL);

	a_pChunk->ReadChildValue("FBO", m_pRenderTarget->m_strFBO);
	a_pChunk->ReadChildValue("ResolverFBO", m_pRenderTarget->m_strResolverFBO);
	a_pChunk->ReadChildValue("Shader", m_pRenderTarget->m_strMainShaderName);
	a_pChunk->ReadChildValue("Name", m_pRenderTarget->m_strName);
		
	CChunk* pCleanOpChunk = a_pChunk->GetChunkUnique("ClearOp");
	if (pCleanOpChunk)
		InitClearOp(pCleanOpChunk);

	CChunk* pDefaultSubsChunk = a_pChunk->GetChunkUnique("Subroutines");
	vector<string> arrSubRoutinesDefault;
	if (pDefaultSubsChunk)
	{
		CONST UINT nSubroutines = pDefaultSubsChunk->GetChunckCount();
		for (UINT uiC = 0; uiC < nSubroutines; uiC++)
		{
			CSubroutineInfo subroutineInfo;
			CChunk& cChunkObject = pDefaultSubsChunk->GetChunkXMLByID(uiC);
			string strSubName = cChunkObject.GetNode().name();
			arrSubRoutinesDefault.push_back(strSubName);
		}
		
	}

	CChunk* pObjectsChunk = a_pChunk->GetChunkUnique("Objects");
	if(pObjectsChunk)
	{
		UINT nOnj = pObjectsChunk ->GetChunckCount();
		for (UINT uiO = 0; uiO < nOnj; uiO++)
		{
			BOOL bValid = FALSE;
			CChunk& pObjChunk = pObjectsChunk->GetChunkXMLByID(uiO);
			string strCName = pObjChunk.GetNode().name();		
			if (strCName.find_first_of('_') == 0)
				continue;

			PRenderedObj pRndObject = new CRenderedObj;
			// Find Object
			UINT nO = arrObjects.size();
			for(UINT ui = 0 ; ui < nO; ui++)
			{
				if (arrObjects[ui]->GetName() == strCName)
				{
					pRndObject->m_pObj = arrObjects[ui];
					pRndObject->m_strObjName = strCName;
					break;
				}
			}
			if(!pRndObject->m_pObj)
				continue;

			// Read Shader and Material
			//////////////////////////////////////////////////////////////////////////
			pObjChunk.ReadChildValue("Shader", pRndObject->m_strShader);
			if (pRndObject->m_strShader == "")
				pRndObject->m_strShader = m_pRenderTarget->m_strMainShaderName;
			pObjChunk.ReadChildValue("Material", pRndObject->m_strMaterial);			
			
			// Init Samplers
			//////////////////////////////////////////////////////////////////////////
			CChunk* pSaplersChunk = pObjChunk.GetChunkUnique("ShaderInputs");
			if (pSaplersChunk)
				InitShaderInputs(pRndObject, pSaplersChunk);

			// Init Subroutines
			//////////////////////////////////////////////////////////////////////////
			CChunk* pChSubroutines = pObjChunk.GetChunkUnique("Subroutines");
			if (pChSubroutines)
				InitShaderSubroutines(pRndObject, pChSubroutines);
			else
				SetDefaultShaderSubroutines(pRndObject, arrSubRoutinesDefault);
						
			// Init Shadow data 
			CChunk* pShadowChunk = pObjChunk.GetChunkUnique("Shadow");
			if (pShadowChunk)
				InitShaderShadowData(pRndObject, pShadowChunk);

			string strlcObjName = pRndObject->m_strObjName; 
			std::transform(strlcObjName.begin(), strlcObjName.end(), strlcObjName.begin(), ::tolower);

			m_pRenderTarget->m_mapRenderedObj.insert(std::make_pair(strlcObjName, pRndObject));
			m_pRenderTarget->m_arrObjects.push_back(pRndObject->m_pObj);
		}	
		m_pRenderTarget->m_iRTIndex = iRTCounter;
		iRTCounter++;	
	}		
}

VOID CBaseRT::PostInit()
{
	for (map<string, PRenderedObj>::iterator iter = m_pRenderTarget->m_mapRenderedObj.begin(); iter != m_pRenderTarget->m_mapRenderedObj.end(); iter++)
	{
		PRenderedObj pRndObj = iter->second;
		m_pResMng->CopyMaterial(pRndObj->m_strMaterial, pRndObj->m_strObjName, pRndObj->m_Material);
	}

	// Init FBO pointers
	if (!m_pRenderTarget->m_strFBO.empty())
		m_pRenderTarget->m_pFBO = m_pResMng->GetFBO(m_pRenderTarget->m_strFBO);
	if (!m_pRenderTarget->m_strResolverFBO.empty())
		m_pRenderTarget->m_pResolverFBO = m_pResMng->GetFBO(m_pRenderTarget->m_strResolverFBO);

	if (m_pRenderTarget->m_pFBO)
	{
		m_pRenderTarget->m_ClearOp.m_iWidth  = m_pRenderTarget->m_pFBO->m_iWidth;
		m_pRenderTarget->m_ClearOp.m_iHeight = m_pRenderTarget->m_pFBO->m_iHeigth;
	}
	else
	{
		m_pRenderTarget->m_ClearOp.m_iWidth  = ENG::SCR_WIDTH;
		m_pRenderTarget->m_ClearOp.m_iHeight = ENG::SCR_HEIGHT ;
	}
}

BOOL CBaseRT::GetSamplerInfo( CChunk& pSampChunk, PBaseShaderInput pSampInfo)
{
	string strSampType = "";
	pSampChunk.ReadChildValue("Type", strSampType);
	
	if(strSampType == "Texture")				
	{
		pSampInfo->m_eType = eSIT_SAMPLER;
		pSampChunk.ReadChildValue("Location", pSampInfo->m_iBindLocation);
		bool bCubeMap = false;
		pSampChunk.ReadChildValue("CubeMap", bCubeMap);
		if (bCubeMap)
			AddFlag(pSampInfo->m_eFlags, F_SI_CUBE_MAP);
		pSampChunk.ReadChildValue("Name", pSampInfo->m_strName);
		pSampInfo->m_iTextureID = m_pResMng->LoadTexture(&pSampChunk);
		return TRUE;
	}
	else if(strSampType == "FBO")
	{
		pSampInfo->m_eType = eSIT_FBO;
		pSampChunk.ReadChildValue("Name", pSampInfo->m_strName);	
		pSampChunk.ReadChildValue("FBO", pSampInfo->m_strNameFBO);			
		pSampChunk.ReadChildValue("Location", pSampInfo->m_iBindLocation);		
		pSampChunk.ReadChildValueDef("ColorAttachment", pSampInfo->m_iColAttachFBO, 0);
		PFBO pFBO = m_pResMng->GetFBO(pSampInfo->m_strNameFBO);
		if (pFBO)
			pSampInfo->m_iTextureID = pFBO->m_arrTextures[pSampInfo->m_iColAttachFBO];	
		return TRUE;
	}
	else if(strSampType == "PREV_FBO")
	{
		pSampInfo->m_eType = eSIT_PREV_FBO;
		pSampChunk.ReadChildValue("Name", pSampInfo->m_strName);	
		pSampChunk.ReadChildValue("FBO", pSampInfo->m_strNameFBO);			
		pSampChunk.ReadChildValue("Location", pSampInfo->m_iBindLocation);		
		pSampChunk.ReadChildValueDef("ColorAttachment", pSampInfo->m_iColAttachFBO, 0);			
		PFBO pFBO = m_pResMng->GetFBO(pSampInfo->m_strNameFBO);
		if (pFBO)
			pSampInfo->m_iTextureID = pFBO->m_arrTextures[pSampInfo->m_iColAttachFBO];			
		return TRUE;
	}
	else if(strSampType == "OffsetTex")				
	{
		pSampInfo->m_eType = eSIT_OFFSET_TEX;
		pSampChunk.ReadChildValue("Location", pSampInfo->m_iBindLocation);	
		pSampChunk.ReadChildValue("Name", pSampInfo->m_strName);
		pSampInfo->m_iTextureID = m_pResMng->GetOffsetTexture();
		return TRUE;
	}
	else if(strSampType == "FLOAT4")
	{
		pSampInfo->m_eType = eSIT_FLOAT4;
		pSampChunk.ReadChildValue("Name", pSampInfo->m_strName);
		Vec4 vVals;
		pSampChunk.ReadChildValue("Values", vVals);
		pSampInfo->m_arrVals[0] = vVals.x;
		pSampInfo->m_arrVals[1] = vVals.y;
		pSampInfo->m_arrVals[2] = vVals.z;
		pSampInfo->m_arrVals[3] = vVals.w;
		return TRUE;
	}
	else if(strSampType == "SCREEN_DATA")
	{
		pSampInfo->m_eType = eSIT_FLOAT4;
		pSampChunk.ReadChildValue("Name", pSampInfo->m_strName);
		pSampInfo->m_arrVals[0] = ENG::SCR_WID_INV;
		pSampInfo->m_arrVals[1] = ENG::SCR_HEI_INV;
		pSampInfo->m_arrVals[2] = Camera::GetInstance()->GetCurrentView()->GetZFar();
		pSampInfo->m_arrVals[3] = Camera::GetInstance()->GetCurrentView()->GetZNear();
		return TRUE;
	}	
	return FALSE;
}

VOID CBaseRT::InitShaderInputs( PRenderedObj pRndObject, CChunk* pSaplersChunk )
{	
	UINT nSamp = pSaplersChunk->GetChunckCount();
	for (UINT uiS = 0; uiS < nSamp; uiS++)
	{
		CChunk& pSampChunk = pSaplersChunk->GetChunkXMLByID(uiS);
		string strSampName = pSampChunk.GetNode().name();		
		if (strSampName.find_first_of('_') == 0)
			continue;
		PBaseShaderInput pSampInfo = new CBaseShaderInput;
		if(GetSamplerInfo(pSampChunk, pSampInfo))
			pRndObject->m_arrSamplers.push_back(pSampInfo);
	}
}

VOID CBaseRT::InitShaderSubroutines( PRenderedObj pRndObject, CChunk* pSubroutinesChunk )
{
	CONST UINT nSubroutines = pSubroutinesChunk->GetChunckCount();
	if (nSubroutines == 0)
		return;
	for (UINT uiC = 0; uiC < nSubroutines; uiC++)
	{
		CSubroutineInfo subroutineInfo;
		CChunk& cChunkObject = pSubroutinesChunk->GetChunkXMLByID(uiC);
		subroutineInfo.m_strName = cChunkObject.GetNode().name();
		subroutineInfo.m_bGotID = FALSE;
		pRndObject->m_arrSubroutines.push_back(subroutineInfo);
	}
	pRndObject->m_iCountSubroutinse = nSubroutines;
	pRndObject->m_arrSubroutinesIDs = new GLuint[nSubroutines];
}

VOID CBaseRT::SetDefaultShaderSubroutines( PRenderedObj pRndObject, const vector<string>& arrSubRoutines )
{
	CONST UINT nSubroutines = arrSubRoutines.size();
	if (nSubroutines == 0)
		return;
	for (UINT uiC = 0; uiC < nSubroutines; uiC++)
	{
		CSubroutineInfo subroutineInfo;
		subroutineInfo.m_strName = arrSubRoutines[uiC];
		subroutineInfo.m_bGotID = FALSE;
		pRndObject->m_arrSubroutines.push_back(subroutineInfo);
	}
	pRndObject->m_iCountSubroutinse = nSubroutines;
	pRndObject->m_arrSubroutinesIDs = new GLuint[nSubroutines];
}

VOID CBaseRT::InitShaderShadowData( PRenderedObj pRndObject, CChunk* pShadowChunk )
{
	// TODO: Transfer to default values!
	pShadowChunk->ReadChildValueDef("Intensity", pRndObject->m_Material.m_vShadow.x, pRndObject->m_Material.m_vShadow.x);
	pShadowChunk->ReadChildValueDef("BiasDirect", pRndObject->m_Material.m_vShadow.y, pRndObject->m_Material.m_vShadow.y);
	pShadowChunk->ReadChildValueDef("PCFlevel", pRndObject->m_Material.m_vShadow.z, pRndObject->m_Material.m_vShadow.z);
	pShadowChunk->ReadChildValueDef("BiasPoint", pRndObject->m_Material.m_vShadow.w, pRndObject->m_Material.m_vShadow.w);
}

VOID CBaseRT::InitClearOp( CChunk* pClearChunk )
{
	pClearChunk->ReadChildValueDef("ColorBuff", m_pRenderTarget->m_ClearOp.m_bClearColor, m_pRenderTarget->m_ClearOp.m_bClearColor);
	pClearChunk->ReadChildValueDef("DepthBuff", m_pRenderTarget->m_ClearOp.m_bClearDepth, m_pRenderTarget->m_ClearOp.m_bClearDepth);
	pClearChunk->ReadChildValueDef("StencilBuff", m_pRenderTarget->m_ClearOp.m_bClearStencil, m_pRenderTarget->m_ClearOp.m_bClearStencil);
	pClearChunk->ReadChildValueDef("Color", m_pRenderTarget->m_ClearOp.m_vBackColor, m_pRenderTarget->m_ClearOp.m_vBackColor);
	pClearChunk->ReadChildValueDef("Depth", m_pRenderTarget->m_ClearOp.m_fDepthValue, m_pRenderTarget->m_ClearOp.m_fDepthValue);

	if (m_pRenderTarget->m_ClearOp.m_bClearColor)
		m_pRenderTarget->m_ClearOp.m_uiClear |= GL_COLOR_BUFFER_BIT;// | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	if (m_pRenderTarget->m_ClearOp.m_bClearDepth)
		m_pRenderTarget->m_ClearOp.m_uiClear |= GL_DEPTH_BUFFER_BIT;// | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	if (m_pRenderTarget->m_ClearOp.m_bClearStencil)
		m_pRenderTarget->m_ClearOp.m_uiClear |= GL_STENCIL_BUFFER_BIT;// | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;

}

VOID CBaseRT::DebugRender()
{
	m_pRndMng->RenderRTQuad(m_pRenderTarget->m_strName, m_pRenderTarget->m_pFBO, m_pRenderTarget->m_iRTIndex);
}

VOID DepthRT::Init(RenderManager* pRndMng,  OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects ) 
{
	CBaseRT::Init(pRndMng, pGL, a_pChunk, arrObjects);
}

VOID DepthRT::Render(  )
{
	m_pRndMng->RenderSceneToDepthFBO(m_pRenderTarget);
}

VOID DepthRT::DebugRender()
{
	PFBO fboDbg = m_pResMng->GetFBO("fboDepthDbg");
	m_pRndMng->RenderRTQuad(m_pRenderTarget->m_strName, fboDbg, m_pRenderTarget->m_iRTIndex);
}

VOID SceneRT::Init( RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects )
{
	CBaseRT::Init(pRndMng, pGL, a_pChunk, arrObjects);	
}

VOID SceneRT::Render( )
{
	m_pRndMng->SetPassSide(E_RND_PASS_LEFT);
	m_pRndMng->RenderSceneToFBO(m_pRenderTarget);
#ifndef C_R
	m_pRndMng->RenderDebug(m_pRenderTarget);
#endif
	if (bSTEREO)
	{
		m_pRndMng->SetPassSide(E_RND_PASS_RIGHT);
		m_pRndMng->RenderSceneToFBO(m_pRenderTarget);	
	}
}

VOID ReflectRT::Init( RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects )
{
	CBaseRT::Init(pRndMng, pGL, a_pChunk, arrObjects);	
}

VOID ReflectRT::Render()
{
	m_pRndMng->SetPassSide(E_RND_PASS_LEFT);
	m_pRndMng->RenderSceneReflectToFBO(m_pRenderTarget);
	if (bSTEREO)
	{
		m_pRndMng->SetPassSide(E_RND_PASS_RIGHT);
		m_pRndMng->RenderSceneReflectToFBO(m_pRenderTarget);
	}
}


VOID SceneMSRT::Init( RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects )
{
	CBaseRT::Init(pRndMng, pGL, a_pChunk, arrObjects);	
}

VOID SceneMSRT::Render(  )
{
	glEnable(GL_MULTISAMPLE);
	
	m_pRndMng->SetPassSide(E_RND_PASS_LEFT);
	m_pRndMng->RenderSceneToFBO(m_pRenderTarget);
	
#ifndef C_R
	m_pRndMng->RenderDebug(m_pRenderTarget);
#endif
	if (bSTEREO)
	{
		m_pRndMng->SetPassSide(E_RND_PASS_RIGHT);
		m_pRndMng->RenderSceneToFBO(m_pRenderTarget);	
	}

	glDisable(GL_MULTISAMPLE);

	m_pRndMng->SetPassSide(E_RND_PASS_LEFT);
	m_pRndMng->ResolveMultiSampleFBO(m_pRenderTarget);

	if (bSTEREO)
	{
		m_pRndMng->SetPassSide(E_RND_PASS_RIGHT);
		m_pRndMng->ResolveMultiSampleFBO(m_pRenderTarget);	
	}
}

VOID SceneMSRT::DebugRender()
{
	m_pRndMng->RenderRTQuad(m_pRenderTarget->m_strName, m_pRenderTarget->m_pResolverFBO, m_pRenderTarget->m_iRTIndex);
}

VOID ResolverRT::Init(RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects)
{
	CBaseRT::Init(pRndMng, pGL, a_pChunk, arrObjects);
}

VOID ResolverRT::Render()
{
	m_pRndMng->SetPassSide(E_RND_PASS_LEFT);
	m_pRndMng->ResolveMultiSampleFBO(m_pRenderTarget);
	
	if (bSTEREO)
	{
		m_pRndMng->SetPassSide(E_RND_PASS_RIGHT);
		m_pRndMng->ResolveMultiSampleFBO(m_pRenderTarget);	
	}
}

VOID CameraDepthRT::Init( RenderManager* pRndMng, OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects )
{
	CBaseRT::Init(pRndMng, pGL, a_pChunk, arrObjects);	
}

VOID CameraDepthRT::Render()
{
	m_pRndMng->SetPassSide(E_RND_PASS_LEFT);
	m_pRndMng->RenderSceneToWaterFBO(m_pRenderTarget);
	if (bSTEREO)
	{
		m_pRndMng->SetPassSide(E_RND_PASS_RIGHT);
		m_pRndMng->RenderSceneToWaterFBO(m_pRenderTarget);
	}
}

VOID PostRT::Init(RenderManager* pRndMng,  OGL4* pGL, CChunk* a_pChunk, vector<PBasicObj>& arrObjects )
{
	CBaseRT::Init(pRndMng, pGL, a_pChunk, arrObjects);

	CChunk* pCleanOpChunk = a_pChunk->GetChunkUnique("ClearOp");
	if (pCleanOpChunk)
		InitClearOp(pCleanOpChunk);

	CChunk* pDefaultSubsChunk = a_pChunk->GetChunkUnique("Subroutines");
	vector<string> arrSubRoutinesDefault;
	if (pDefaultSubsChunk)
	{
		CONST UINT nSubroutines = pDefaultSubsChunk->GetChunckCount();
		for (UINT uiC = 0; uiC < nSubroutines; uiC++)
		{
			CSubroutineInfo subroutineInfo;
			CChunk& cChunkObject = pDefaultSubsChunk->GetChunkXMLByID(uiC);
			string strSubName = cChunkObject.GetNode().name();
			arrSubRoutinesDefault.push_back(strSubName);
		}

	}

	// Init Effects
	//////////////////////////////////////////////////////////////////////////
	CChunk* pChEffects = a_pChunk->GetChunkUnique("Effects");
	if(pChEffects)
	{
		UINT nEffects = pChEffects->GetChunckCount();
		for (UINT uiE = 0; uiE < nEffects; uiE++)
		{
			PRenderedObj pEffect = new CRenderedObj;
			CChunk& cChunkEffect = pChEffects->GetChunkXMLByID(uiE);
			string strChunkName = cChunkEffect.GetNode().name();			
			if (strChunkName.find_first_of('_') == 0)
				continue;

			pEffect->m_strObjName = strChunkName;
			cChunkEffect.ReadChildValue("Shader", pEffect->m_strShader);			
			cChunkEffect.ReadChildValue("FBO", pEffect->m_strFBO);			
			cChunkEffect.ReadChildValueDef("Active", pEffect->m_bActive, true);			

			
			CChunk* pSaplersChunk = cChunkEffect.GetChunkUnique("ShaderInputs");
			if (pSaplersChunk)
				InitShaderInputs(pEffect, pSaplersChunk);

			// Init Subroutines
			//////////////////////////////////////////////////////////////////////////
			CChunk* pChSubroutines = cChunkEffect.GetChunkUnique("Subroutines");
			if (pChSubroutines)
				InitShaderSubroutines(pEffect, pChSubroutines);
			else
				SetDefaultShaderSubroutines(pEffect, arrSubRoutinesDefault);
			
			m_pRenderTarget->m_arrEffects.push_back(pEffect);

		}
		m_pRenderTarget->m_iRTIndex = iRTCounter;
		iRTCounter++;
	}
}

VOID PostRT::Render( )
{
	m_pRndMng->SetPassSide(E_RND_PASS_LEFT);
	string strLastFBO = "";
	if (bSTEREO)
	{
		// render left eye FBO
		m_pRndMng->PostRenderScene(m_pRenderTarget, strLastFBO);

		// render right eye FBO
 		m_pRndMng->SetPassSide(E_RND_PASS_RIGHT);
 		m_pRndMng->PostRenderScene(m_pRenderTarget, strLastFBO);

		m_pRndMng->RenderLastFBO(strLastFBO, true);
	}
	else
	{
		m_pRndMng->PostRenderScene(m_pRenderTarget, strLastFBO);
		m_pRndMng->RenderLastFBO(strLastFBO);

	}

	m_pRndMng->RenderUI();

}

VOID PostRT::DebugRender()
{
	UINT nEffects = m_pRenderTarget->m_arrEffects.size();
	UINT iActives = 0;
	for (UINT uiE = 0; uiE < nEffects; uiE++)
	{
		PRenderedObj pEffect = m_pRenderTarget->m_arrEffects[uiE];
		if (!pEffect->m_bActive)
			continue;
		PFBO fbo = m_pResMng->GetFBO(pEffect->m_strFBO);
		m_pRndMng->RenderRTQuad(pEffect->m_strObjName, fbo, m_pRenderTarget->m_iRTIndex + iActives);
		iActives ++;
	}
}

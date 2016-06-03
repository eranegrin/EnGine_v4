#include "RenderManager.h"
#include "../EnGBasic/EnGBasic.h"

//////////////////////////////////////////////////////////////////////////
//	http://www.learnopengl.com/
//  FXAA
//  http://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/
//  http://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/3/
//////////////////////////////////////////////////////////////////////////

BOOL		RenderManager::RENDER_UI = FALSE;
BOOL		RenderManager::RENDER_VA = FALSE;
BOOL		RenderManager::RENDER_CAMERA = FALSE;

CVAR(rmRegularDepthMap, 1, "Write to regular depth FBO (no textProj)");
// vDebug
//////////////////////////////////////////////////////////////////////////
CVAR(rmBumpCoff, 1.0f, "Bump normal offset");
CVAR(ltShinness, 0.0f, "Shine factor (light and material)");
CVAR(ltUseHalfector, 0.0f, "Half vector for reflection calculation");
CVAR(ltAttenFactor, 0.0f, "Bump normal offset");

CVAR(texoffsetRadius, 1.0f, "Bump normal offset");
CVAR(texoffsetDepth, 4.0f, "Bump normal offset");
CVAR(texoffsetSize, 512.0f, "Bump normal offset");

// vDebugCT
//////////////////////////////////////////////////////////////////////////
CVAR(ctRough, 0.03f, "SpecTest");
CVAR(ctF0, 0.8f, "SpecTest");
CVAR(ctK, 0.2f, "SpecTest");
CVAR(spotCut, 1.0f, "Water Bais");

// vDebugReflect
//////////////////////////////////////////////////////////////////////////
CVAR(perWid, 0.00078125f, "Reflection power 0-1");
CVAR(perPower, 0.9f, "Reflection power 0-1");
CVAR(perStep, 10.0f, "reflect against the mesh normal");
CVAR(perFactor, 0.1f, "reflect against the mesh normal");

// Water Ripple
CVAR(rmRippleAmp, 0.03f, "Water Bais");
CVAR(rmRippleSpeed, 1.0f, "Water Bais");
CVAR(rmRippleShift, 5.0f, "Water Bais");

// Water Waves
CVAR(rmWaterAmp, 0.03f, "Water Bais");
CVAR(rmWaterSpeed, 0.06f, "Water Bais");
CVAR(rmEmiterCount, 1.0f, "Water Bais");
CVAR(rmWaveLen, 0.7f, "Water Bais");
CVAR(rmWaterRefX, 0.1f, "Water Bais");
CVAR(rmWaterRefY, 0.7f, "Water Bais");

// Color Quad
CVAR(cqType, 1.0f, "Type of color quad");
CVAR(cqOpacity, 0.5f, "Type of color quad");
CVAR(cqBorderSize, 0.02f, "color quad border size");

/////////////////////////
// DEBUG RENDER OBJECT //
/////////////////////////
CVAR(rtScreenRes, 4 , "");
CVAR(debugRenderObject, 0 , "");
CVAR(renderObjectName, "" , "");
CVAR(fontID, 0 , "");

CVAR(partAtt, 0.1f , "");
CVAR(partPointSize, 20.0f , "");

RenderManager::RenderManager()
{
	m_pOpenGL		= NULL;
	m_pResMng		= NULL;
	m_pLightsMng	= NULL;
	m_pVA			= NULL;	
	m_bStereo		= false;
	
	m_pFreeTyprFont = NULL;
	m_dwFlags		= 0;
	m_fCounter01	= 0.0f;
	m_ePassSide		= E_RND_PASS_LEFT;
	// Debug UI
	m_parrSceneObjects = NULL;
}

RenderManager::~RenderManager()
{
	UINT nUIObj = m_arrObjectsUI.size();
	for (UINT uiO = 0; uiO < nUIObj; uiO++)
	{
		CFrameUI* pUI = m_arrObjectsUI[uiO];
		delete pUI;
	}
	m_arrObjectsUI.clear();

	if (m_pVA)
	{
		 VisAids::GetInstance()->Kill();
		 m_pVA = NULL;
	}
	if (m_pResMng)
	{
		delete m_pResMng;
	}
	if (m_pFreeTyprFont)
		delete m_pFreeTyprFont;
}

//////////////////////////////////////////////////////////////////////////
//	Loading
//////////////////////////////////////////////////////////////////////////
BOOL RenderManager::InitRenderer( OGL4* ogl, CChunk* a_pRootChunk, PCamera pCamera )
{
	m_pOpenGL = ogl;
	m_pCamera = pCamera;
	m_pResMng = ResourceMng::GetInstance();// new ResourceMng(m_pOpenGL);
	m_pResMng->SetOpenGL(ogl);
	m_pLightsMng = LightsManager::GetInstance();

	glEnable(GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST); //enable the depth testing
	glCullFace(GL_BACK);

	// Load shaders from XML
	CChunk* chShaders = a_pRootChunk->GetChunkUnique("Shaders");
	LoadShadersFromXML(chShaders);
	GL_ERROR();
	//  Load FBOs from XML
	CChunk* chFBOs = a_pRootChunk->GetChunkUnique("FBOs");	
	LoadFBOsFromXML(chFBOs);
	GL_ERROR();

	CChunk* chMaterials = a_pRootChunk->GetChunkUnique("Materials");	
	LoadMaterialsFromXML(chMaterials);
	GL_ERROR();

	
	// Set Screen perspective matrix
	m_matPerspectiveScreen.identity();
	m_matPerspectiveScreen[10] = -1.0f;
	
	CChunk* chFont = a_pRootChunk->GetChunkUnique("FontGL");
	LoadFontFromXML(chFont);
	GL_ERROR();

	// Init UI elements
	CChunk* chIcons = a_pRootChunk->GetChunkUnique("UIGL");
	LoadIconsFromXML(chIcons);
	GL_ERROR(); 

	MessageMng::MsgMng()->SubscribeToMessage(this, E_MSG_ADD_VA_MESH);
	MessageMng::MsgMng()->SubscribeToMessage(this, E_MSG_RENDER_UI);
	
	
	m_pResMng->BuildOffsetTexture(512, 4, 8);

	return TRUE;

}

BOOL RenderManager::PostInitRenderer( CChunk* a_pRootChunk)
{
	SetStereo(FALSE);	
	// Fill lights and blur weights UBOs
	// Fill Additional data to shaders (lights and material UBO's)
	int nShaders = m_pResMng->GetShaderCount();
	for (int i = 0; i < nShaders; i++)
	{
		GLSLProgram* pProg = m_pResMng->GetShader(i);
		if (pProg)
		{
			CUBO* pUBOLight = UBOManager::GetInstance()->GetUBO("Lights");
			// lights ubo exists and hasn't been filled yet...
			if (pUBOLight && !pUBOLight->m_bFilled)
			{
				pProg->Use();
				UINT nLights = m_pLightsMng->GetLightCount();
				for (UINT uiL = 0; uiL < nLights; uiL++)
				{
					tLight* tL = m_pLightsMng->GetLight(uiL);
					float arrData[24];
					tL->GetLightsFloatArray(arrData);
					pProg->FillUBOData(pUBOLight, arrData, 24, uiL);
				}
				pProg->Disable();
				pUBOLight->m_bFilled = true;
			}
			
			CUBO* pUBOBlurWeights = UBOManager::GetInstance()->GetUBO("BlurWeights");
			if (pUBOBlurWeights && !pUBOBlurWeights->m_bFilled)
			{
				pProg->Use();
				float arrData[60];
				GetBlurWeightsArray(arrData);
				pProg->FillUBOData(pUBOBlurWeights, arrData, 60, 0);
				pProg->Disable();
				pUBOBlurWeights->m_bFilled = true;
			}
		}
	}	



	//m_pLightsMng = LightsManager::GetInstance();

	//// Fill Additional data to shaders (lights and material UBO's)
	//int nShaders = m_pResMng->GetShaderCount();
	//for (int i = 0; i < nShaders; i++)
	//{
	//	GLSLProgram* pProg = m_pResMng->GetShader(i);
	//	if (pProg && IsFlagUp(pProg->m_dwFlags, F_GLSL_USE_LIGHT_UBO))
	//	{
	//		//	 Fill lights uniform buffer data
	//		pProg->Use();
	//		UINT nLights = m_pLightsMng ->GetLightCount();
	//		for (UINT uiL = 0; uiL < nLights; uiL++)
	//		{
	//			tLight* tL = m_pLightsMng->GetLight(uiL);
	//			pProg->FillLightUBO(tL, uiL);
	//		}
	//		pProg->Disable();

	//	}
	//	if (pProg && IsFlagUp(pProg->m_dwFlags, F_GLSL_USE_BLUR_UBO))
	//	{
	//		//	 Fill blur weights uniform buffer data
	//		pProg->Use();
	//		pProg->FillBlurUBO(0);
	//		pProg->Disable();
	//	}
	//}	
	GL_ERROR();

	return TRUE;
}

BOOL RenderManager::InitVisualAids(vector<PTriMesh> arrMeshes)
{
	// Load and Init VA objects
	m_pVA = VisAids::GetInstance();
	return m_pVA->LoadVisAids(arrMeshes);
}

BOOL RenderManager::LoadShadersFromXML( CChunk* a_pChunck )
{
	return m_pResMng->LoadShaders(a_pChunck);
}

BOOL RenderManager::LoadFBOsFromXML( CChunk* a_pChunck )
{
	return m_pResMng->LoadFBOs(a_pChunck);
}


BOOL RenderManager::LoadFontFromXML( CChunk* a_pChunck )
{
	m_pFreeTyprFont = new FreeTypeEng();
	BOOL bres = m_pResMng->CreateVertexBufferArrayForFont(m_pFreeTyprFont->m_vboData.m_uiVertexArrayID, m_pFreeTyprFont->m_vboData.m_uiVertexBufferID, m_pFreeTyprFont->m_vboData.m_uiIndexBufferID);
	//BOOL bres = m_pResMng->CreateVertexBufferArrayForFontDouble(m_pFreeTyprFont->m_vboData.m_uiVertexArrayID, m_pFreeTyprFont->m_vboData.m_uiVertexBufferID, m_pFreeTyprFont->m_vboData.m_uiIndexBufferID);
	string strShaderName = "";
	a_pChunck->ReadChildValue("Shader", strShaderName );
	GLSLProgram* pProg =  m_pResMng->GetShader(strShaderName );
	if (!pProg)
	{
		PRINT_ERROR("FontGL shader %s is missing, FontGL loading fail!", strShaderName);
		delete m_pFreeTyprFont;
		return FALSE;
	}
	m_pFreeTyprFont->m_pProgram = pProg;
	m_pFreeTyprFont->InitFont(m_pOpenGL, a_pChunck);
	return TRUE;
}

BOOL RenderManager::LoadIconsFromXML(CChunk* a_pChunck)
{
	if (!a_pChunck)
		return FALSE;
	TIC();
	PRINT_INFO("========| Loading Icons");

	UINT uiFrames = a_pChunck->GetChunckCount();
	for (UINT uiObj = 0; uiObj < uiFrames; uiObj++)
	{
		CChunk cIcon = a_pChunck->GetChunkXMLByID(uiObj);
		// Skip icons
		string strChName = cIcon.GetNode().name();
		if (strChName.find_first_of('_') == 0)
			continue;
		
		string strType = "";
		cIcon.ReadChildValue("Type", strType);
		PFrameUI pFrame = AllocateFrameType(strType);
		pFrame->m_strName = strChName;
		pFrame->InitObj(&cIcon, m_pOpenGL);
		m_arrObjectsUI.push_back(pFrame);
		PRINT_INFO("Material Name: %s", pFrame->m_strName.c_str());
	}
	TOC("Loading Icons Completed... ");

	return TRUE;
}

BOOL RenderManager::LoadObjectVAO(PBasicObj pObj)
{
	if (pObj->dwType == F_OBJTYP_PART_SYS)
	{
		ParticleSysManager* pSysMng = static_cast<ParticleSysManager*>(pObj);
		const int nPartSysSize = pSysMng->GetDescriptor().nSize;
		float* arrPos = new float[nPartSysSize * 4];
		float* arrCol = new float[nPartSysSize * 4];
		pSysMng->GetRendereArrays(arrPos, arrCol);
		m_pResMng->CreateParticleSystem(nPartSysSize, arrPos, arrCol);
	}
	else
	{
		UINT nMehses = pObj->GetMeshCount();
		for(UINT uiM = 0; uiM < nMehses; uiM++)
		{
			PTriMesh pMesh = pObj->GetMeshArray()[uiM];
			if (!m_pResMng->ParseMeshVAO(pMesh))
				return FALSE;
		}
	}

	return TRUE;
}

INT RenderManager::LoaMeshVA( PTriMesh pMesh )
{
	if (!m_pResMng->ParseMeshVAO(pMesh))
		return -1;
	return m_pVA->AddVisAids(pMesh);
}

int RenderManager::ReceiveMessage( BasicMsg *msg, const void *sender )
{

	if (msg->m_eMsg == E_MSG_ADD_VA_MESH)
	{
		CPsxActorParams* pActParam = static_cast<CPsxActorParams*>(msg->m_pData);
		INT iVAIndex = LoaMeshVA(pActParam->m_pMesh );
		pActParam->m_iVAInd = iVAIndex;		
	}	
	if (msg->m_eMsg == E_MSG_RENDER_UI)
	{
		string str = *(static_cast<string*>(msg->m_pData));
		if(str == "UI")
			RENDER_UI = !RENDER_UI;
		else if(str == "CAM")
			RENDER_CAMERA = !RENDER_CAMERA;
		else if(str == "VA")
			RENDER_VA = !RENDER_VA;

	}	
	return 1;
}

BOOL RenderManager::ReleaseObjectVAO( PBasicObj pObj )
{
	if (pObj->dwType == F_OBJTYP_PART_SYS)
	{
		m_pResMng->ReleaseParticleSystem();
	}
	else
	{
		UINT nMehses = pObj->GetMeshCount();
		for(UINT uiM = 0; uiM < nMehses; uiM++)
		{
			PTriMesh pMesh = pObj->GetMeshArray()[uiM];
			if (!m_pResMng->ReleaseMeshVAO(pMesh))
				return FALSE;
		}
		UINT nActors = pObj->GetActors().size();
		for(UINT uiA = 0; uiA < nActors; uiA++)
		{
			PTriMesh pMesh = pObj->GetActorMesh(uiA);
			if (pMesh )
			{
				if (!m_pResMng->ReleaseMeshVAO(pMesh))
					return FALSE;
			}
		}


	}
	return TRUE;
}

VOID RenderManager::PreRenderUpdate(vector<PBasicObj>& arrObjects)
{
	// update lights UBO from light map
	//int nShaders = m_pResMng->GetShaderCount();
	//for (int i = 0; i < nShaders; i++)
	//{
	//	GLSLProgram* pProg = m_pResMng->GetShader(i);
	//	if (pProg)
	//	{
	//		CUBO* pUBOLight = UBOManager::GetInstance()->GetUBO("Lights");
	//		// lights ubo exists and hasn't been filled yet...
	//		if (pUBOLight)
	//		{
	//			UINT nLights = m_pLightsMng->GetLightCount();
	//			for (UINT uiL = 0; uiL < nLights; uiL++)
	//			{
	//				tLight* tL = m_pLightsMng->GetLight(uiL);
	//				if (tL->m_bUpdated)
	//				{
	//					float arrData[24];
	//					tL->GetLightsFloatArray(arrData);
	//					pProg->Use();
	//					pProg->UpdateUBOData(pUBOLight, arrData, 24, uiL);							
	//					pProg->Disable();
	//				}
	//			}
	//			
	//		}
	//	}
	//}

	//m_pFreeTyprFont->PrepareFontforRender();

	UINT nLights = m_pLightsMng->GetLightCount();
	for (UINT uiL = 0; uiL < nLights; uiL++)
	{
		tLight* tL = m_pLightsMng->GetLight(uiL);
		if (tL->m_bUpdated)
		{
			GLSLProgram* pLightsProgram = m_pResMng->GetShader("SceneLight");
			if (pLightsProgram)
			{
				UINT nUBS = pLightsProgram->GetArrayUBO().size();
				for (UINT u = 0; u < nUBS; u++)
				{
					string strUBO = pLightsProgram->GetArrayUBO()[u];
					if (strUBO == "Lights")
					{
						CUBO* pUBO = UBOManager::GetInstance()->GetUBO(strUBO);
						if (pUBO)
						{
							float arrData[24];
							tL->GetLightsFloatArray(arrData);
							pLightsProgram->Use();
							pLightsProgram->UpdateUBOData(pUBO, arrData, 24, uiL);
							pLightsProgram->Disable();
						}
					}
				}
			}

			
		}
	}

	

	CameraView* pCamView = m_pCamera->GetCurrentView();
	UINT nObjects = arrObjects.size();	
	for (UINT uiO = 0; uiO < nObjects; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		if (pCamView->m_bUpdateViewMatrix)
		{
			if (IsFlagUp(pObj->dwFlags, F_OBJ_FRUSTUM_UPDATE))
			{
				//////////////////////////////////////////////////////////////////////////
				// Update the index buffer object, if necessary, mostly for frustum tests 
				if (pObj->GetUpdateVaoIndices())
				{
					PRenderPack pPack = pObj->GetMeshArray()[0]->GetRenderPack();
					m_pOpenGL->glBindBuffer(GL_ARRAY_BUFFER, pPack->m_uiIndexBufferID);
					m_pOpenGL->glBufferSubData(GL_ARRAY_BUFFER, 0, pPack->m_uiLength* sizeof(unsigned int), pPack->m_arrIndices);
					m_pOpenGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}
		}

		if (pObj->dwType == F_OBJTYP_WATER)
		{
			if (pObj->GetUpdateVAO())
			{
				/////////////////////////////////////////////////////////////////////////
				// Update the vertex buffer object, if necessary 
				PRenderPack pPack = pObj->GetMeshArray()[0]->GetRenderPack();
				m_pOpenGL->glBindBuffer(GL_ARRAY_BUFFER, pPack->m_uiVertexBufferID);
				m_pOpenGL->glBufferSubData(GL_ARRAY_BUFFER, 0, pPack->m_uiLength * sizeof(VertexUVN), pPack->m_arrVerticesUVN);
				m_pOpenGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

		if (pObj->dwType == F_OBJTYP_PART_SYS)
		{
			ParticleSysManager* pSysMng = static_cast<ParticleSysManager*>(pObj);
			const int nPartSysSize = pSysMng->GetDescriptor().nSize;
			if (nPartSysSize > 0)
			{	
				float* arrPos = new float[nPartSysSize * 4];
				float* arrCol = new float[nPartSysSize * 4];
				pSysMng->GetRendereArrays(arrPos, arrCol);
			
				m_pResMng->UpdateParticleSystem(nPartSysSize, arrPos, arrCol);
				
				delete[] arrPos;
				delete[] arrCol;
			}
			
		}
	}

	
 
}

//////////////////////////////////////////////////////////////////////////
//	Rendering
//////////////////////////////////////////////////////////////////////////
VOID RenderManager::RenderSceneToDepthFBO(  const IRenderTargetBase* rtInfo)
{
	PFBO fbo = rtInfo->m_pFBO;// m_pResMng->GetFBO(rtInfo->m_strFBO);
	fbo->Use();

	//Render the scene objects, no specific order
	//==============================================
	// get current view		
	rtInfo->ClearBuffers();	
	
	glEnable (GL_DEPTH_TEST); //enable the depth testing

	vector<PBasicObj>  arrObjects = rtInfo->m_arrObjects;
	UINT nObjects = arrObjects.size();
	for (UINT uiO = 0; uiO < nObjects; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		BOOL bNoCulling = IsTypeOf(pObj->dwType, F_OBJTYP_TERRAIN) || IsFlagUp(pObj->dwFlags, F_OBJ_NO_DEPTH_CULLING);
		if (IsFlagUp(pObj->dwFlags, F_OBJ_NO_DEPTH_CULLING))
		{
			glDisable(GL_CULL_FACE);
		}
		if (IsFlagUp(pObj->dwFlags, F_OBJ_CAST_SHADOW))
		{
			if (!bNoCulling)
			{
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glCullFace(GL_FRONT);
			}
			
			tLight* pLight = m_pLightsMng->GetActiveLight();
			const CameraView& lightView = m_pCamera->GetViewFromLight(pLight, pObj);
			RenderObjectDepth(&lightView, pObj, rtInfo);

			if (!bNoCulling)
			{
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	
				glCullFace(GL_BACK);
			}
		}
		if (IsFlagUp(pObj->dwFlags, F_OBJ_NO_DEPTH_CULLING))
		{
			glEnable(GL_CULL_FACE);
		}

	}

 	
	fbo->Disable();	

	if (rmRegularDepthMap.GetInt() == 1)
	{
		PFBO fboDbg = m_pResMng->GetFBO("fboDepthDbg");
		fboDbg->Use();

		rtInfo->ClearBuffers();

		glEnable (GL_DEPTH_TEST); //enable the depth testing		
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCullFace(GL_FRONT);

		for (UINT uiO = 0; uiO < nObjects; uiO++)
		{
			PBasicObj pObj = arrObjects[uiO];
			if (IsFlagUp(pObj->dwFlags, F_OBJ_CAST_SHADOW))
			{
				tLight* pLight = m_pLightsMng->GetActiveLight();
				const CameraView& lightView = m_pCamera->GetViewFromLight(pLight, pObj);
				RenderObjectDepth(&lightView, pObj, rtInfo);
			}		
		}

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	
		glCullFace(GL_BACK);

		fboDbg->Disable();
	}

	

}

VOID RenderManager::RenderSceneToWaterFBO(  const IRenderTargetBase* rtInfo)
{
	tLight* pLight = m_pLightsMng->GetActiveLight();

	//glEnable (GL_DEPTH_TEST); //enable the depth testing
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	vector<PBasicObj>  arrObjects = rtInfo->m_arrObjects;
	UINT nObjects = arrObjects.size();
	CameraView* pCamView = m_pCamera->GetCurrentView();

	PFBO fboWater = rtInfo->m_pFBO;
	//PFBO fboWater = m_pResMng->GetFBO(rtInfo->m_strFBO);
	fboWater->Use();

	rtInfo->ClearBuffers();	

	for (UINT uiO = 0; uiO < nObjects; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		RenderObjectDepthMVP(pCamView, pObj, rtInfo);
	}

	fboWater->Disable();		
	
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	
}

VOID RenderManager::RenderSceneToFBO(  const IRenderTargetBase* rtInfo)
{
	GLuint attachments[2];
	PFBO fbo = rtInfo->m_pFBO;
	//PFBO fbo = m_pResMng->GetFBO(rtInfo->m_strFBO);
	attachments[0] = m_ePassSide == E_RND_PASS_RIGHT ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0;
	attachments[1] = m_ePassSide == E_RND_PASS_RIGHT ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT1;
	if(!fbo)
		return;

	fbo->Use();
	m_pOpenGL->glDrawBuffers(1,  attachments);

	//Render the scene objects, no specific order
	//==============================================	
	rtInfo->ClearBuffers();

	CameraView* camView = m_pCamera->GetCurrentView();
	vector<PBasicObj>  arrObjects = rtInfo->m_arrObjects;
	UINT nObjects = arrObjects.size();
	for (UINT uiO = 0; uiO < nObjects; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		RenderObject(camView, pObj, rtInfo);
	}

	fbo->Disable();


}

VOID RenderManager::RenderSceneReflectToFBO(  const IRenderTargetBase* rtInfo )
{
	GLuint attachments[2];
	PFBO fbo = rtInfo->m_pFBO;
	//PFBO fbo = m_pResMng->GetFBO(rtInfo->m_strFBO);

	attachments[0] = m_ePassSide == E_RND_PASS_RIGHT ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0;
	attachments[1] = m_ePassSide == E_RND_PASS_RIGHT ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT1;
	if(!fbo)
		return;

	fbo->Use();
	m_pOpenGL->glDrawBuffers(1,  attachments);

	//Render the scene objects, no specific order
	//==============================================	
	rtInfo->ClearBuffers();	

	CameraView* camView = m_pCamera->GetCurrentView();
	vector<PBasicObj>  arrObjects = rtInfo->m_arrObjects;
	UINT nObjects = arrObjects.size();
	for (UINT uiO = 0; uiO < nObjects; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		RenderObjectRefelct(camView, pObj, rtInfo);
	}	
	m_pOpenGL->glDrawBuffers(1,  attachments);
	fbo->Disable();
}

VOID RenderManager::PostRenderScene( const IRenderTargetBase* rtInfo, string& strFBO)
{
	UINT nPostEffect = rtInfo->m_arrEffects.size();
	INT iLastEffect = nPostEffect-1;
	BOOL bHasEffect = FALSE;
	for (UINT uiE = 0; uiE < nPostEffect; uiE++)
	{
		PRenderedObj pEffect = rtInfo->m_arrEffects[uiE];

		if (!pEffect->m_bActive)
			continue;
 				
		GLuint attachments[2];
		PFBO fbo = m_pResMng->GetFBO(pEffect->m_strFBO);
		if (!fbo)
			continue;
		attachments[0] = m_ePassSide == E_RND_PASS_RIGHT ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0;
		attachments[1] = m_ePassSide == E_RND_PASS_RIGHT ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT1;
		
		
		fbo->Use();
		m_pOpenGL->glDrawBuffers(1,  attachments);

		//Render the scene objects, no specific order
		//==============================================	
		rtInfo->ClearBuffers();	

		GLSLProgram* pProg = m_pResMng->GetShader(pEffect->m_strShader);

		// Get shader program
		if (!pProg )
			return;
		
		pProg->BindUniformBuffer();

		pProg->Use();

		rtInfo->SendMatricesToProgram(pProg, &m_matPerspectiveScreen, NULL);

		// DEBUG!!!
		rtInfo->AttachMaterialAndTextures(pProg, NULL);


		//////////////////////////////////////////////////////////////////////////
		// Subroutines
		//////////////////////////////////////////////////////////////////////////	
		if (IsFlagUp(pProg->m_dwFlags, F_GLSL_SUBROUTINES))
		{
			if (pEffect->m_arrSubroutinesIDs)
			{
				CONST UINT nSubroutines = pEffect->m_arrSubroutines.size();
				for (UINT uiS = 0; uiS < nSubroutines; uiS++)
				{
					if(!pEffect->m_arrSubroutines[uiS].m_bGotID)
					{ 
						pEffect->m_arrSubroutinesIDs[uiS] = m_pOpenGL->glGetSubroutineIndex(pProg->GetID(), GL_FRAGMENT_SHADER, pEffect->m_arrSubroutines[uiS].m_strName.c_str());
						pEffect->m_arrSubroutines[uiS].m_bGotID = TRUE;
					}
				}
				m_pOpenGL->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, nSubroutines, pEffect->m_arrSubroutinesIDs);
			}
			GL_ERROR();
		}




		INT nSamplers = pEffect->m_arrSamplers.size();
		for (INT uiS = 0; uiS < nSamplers; uiS++)
		{
			PBaseShaderInput pShaderInput = pEffect->m_arrSamplers[uiS];
			if (pShaderInput->m_eType == eSIT_PREV_FBO )
			{
				// Effect chain FBO
				PRenderedObj pLastEffect = rtInfo->m_arrEffects[iLastEffect];
				pShaderInput->m_strNameFBO = pLastEffect->m_strFBO;
			}
			rtInfo->SendInputToShader(pProg, pShaderInput, m_ePassSide == E_RND_PASS_RIGHT);			
		}

		const PRenderPack pPack = ResourceMng::GetInstance()->GetDefaultQuadRP();

		// Bind the vertex array object that stored all the information about the vertex and index buffers.
		m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

		// Render the vertex buffer using the index buffer.
		glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);	

		// Unbind the buffer
		m_pOpenGL->glBindVertexArray(0);			

		// Unbind all samplers
		for (INT uiS = nSamplers - 1; uiS >= 0; uiS--)
		{
			m_pOpenGL->glActiveTexture(GL_TEXTURE0 + uiS);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		pProg->Disable();
		
		fbo->Disable();
		iLastEffect = uiE;
		bHasEffect = TRUE;
	}
	if ( !bHasEffect )
	{
		strFBO = "fboScene";
	}
	else
	{
		PRenderedObj pLastEffect = rtInfo->m_arrEffects[iLastEffect];
		strFBO = pLastEffect->m_strFBO;
	}


}

VOID RenderManager::RenderParticleSystem( PBasicObj pObj, GLSLProgram* pProg, const Mat4* arrMatrices, const IRenderTargetBase* rtInfo )
{
	if (!pProg)
		return;
	pProg->Use();
	
	const Mat4&	viewMatrix = arrMatrices[MATRIX_VIEW_ID];
	const Mat4&	perspectiveMatrix = arrMatrices[MATRIX_PROJ_ID];
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();

	ParticleSysManager* pPartSys = static_cast<ParticleSysManager*>(pObj);
	// Send material to program
	//////////////////////////////////////////////////////////////////////////	
	rtInfo->AttachMaterialAndTextures(pProg, pObj->GetName());

	// Calculate the point size

	Vec3 vCamPos = m_pCamera->GetCurrentView()->GetEye();	
	float fSize = pPartSys->GetPointSizeByCamera(vCamPos);

	UINT unfID = pProg->GetUniformLocation("SizeOfPoint");
	if (unfID != -1)
	{
		m_pOpenGL->glUniform1f(unfID, fSize);
	}

	glEnable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);	
	glEnable(GL_BLEND);
	pProg->SendMatrices(NULL, fViewMat, fProjMat);

	m_pOpenGL->glBindVertexArray(m_pResMng->GetPartSysVOAID());

	const size_t count = pPartSys->GetDescriptor().nSize;
	if (count > 0)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//m_pOpenGL->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, count);
	}
	GL_ERROR();	

	m_pOpenGL->glBindVertexArray(0);


	glEnable(GL_DEPTH_TEST);	
	glDisable(GL_BLEND);
	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();

	GL_ERROR();	

}

VOID RenderManager::RenderFBO( const PFBO fbo, GLSLProgram* pProg )
{	
	const float* fProjMat = m_matPerspectiveScreen.get();
	// Get shader program
	if (!pProg )
		return;
	pProg->Use();
	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );
	
	// Bind texture from the fbo
	INT uiMainTexture = fbo->m_arrTextures[0]; // rmTextureIdFBO = 0
 	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
 	
	glBindTexture(GL_TEXTURE_2D, uiMainTexture);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pOpenGL->glUniform1i(unfID, 0);

	const PRenderPack pPack = fbo->m_pRenderPack;
	
	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);	

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);			
	
	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();
	
// 	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, 0);
}

VOID RenderManager::RenderFBOMSAA( const PFBO fbo, GLSLProgram* pProg )
{
	const float* fProjMat = m_matPerspectiveScreen.get();
	// Get shader program
	if (!pProg )
		return;
	pProg->Use();
	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );

	float vSampling[] = {static_cast<float>(ENG::SCR_WIDTH), static_cast<float>(ENG::SCR_HEIGHT), 16.0f };
	unfID = pProg->GetUniformLocation("SamplingVector");
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, vSampling );
	
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->m_arrTextures[0]);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pOpenGL->glUniform1i(unfID, 0);

	const PRenderPack pPack = fbo->m_pRenderPack;

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);	

	pProg->Disable();
}

VOID RenderManager::RenderFont(const IRenderTargetBase* rtInfo)
{

	GLuint attachments[1];
	attachments[0] = GL_COLOR_ATTACHMENT0;
	int iMinY = 75;
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);	
	CvarMng* pCvarMng = CvarMng::GetInstance();
	if (pCvarMng->IsActive())
	{
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		RenderColorQuad("BasicColor", 0.0, 0.75, 0.33, 0.25);
	}
	else 
	{
		float fStartY = 0.7f;
		if (m_pLightsMng->GetDebugMode())
		{
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			RenderColorQuad("BasicColor", 0.0, fStartY, 0.25, 0.25);
			iMinY += 200;
			fStartY -= 0.25f;
		}
		if (m_pCamera->GetDebugMode())
		{
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			RenderColorQuad("BasicColor", 0.0, fStartY, 0.25, 0.25);
			fStartY -= 0.25f;
			iMinY += 200;
		}
		if (debugRenderObject.GetInt())
		{
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			RenderColorQuad("BasicColor", 0.0, fStartY, 0.25, 0.25);
			iMinY += 200;
		}
	}
 	
	glViewport(0, 0, ENG::SCR_WIDTH, ENG::SCR_HEIGHT);
	if (!m_pFreeTyprFont)
		return;
	
	if (m_pFreeTyprFont->m_pProgram)
	{
		m_pFreeTyprFont->m_pProgram->Use();
		UINT unfID = m_pFreeTyprFont->m_pProgram->GetUniformLocation("projectionMatrix");
		if (unfID != -1)
		{
			const float* fProjMat = m_matPerspectiveScreen.get();
			m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );
		}

		m_pFreeTyprFont->Render(fontID.GetInt());

		m_pFreeTyprFont->m_pProgram->Disable();
		GL_ERROR();
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);	
		
}

VOID RenderManager::RenderUI()
{
	// Convert mouse position to GL UI
	float mx = ENG::MOUSE_POS_X * ENG::SCR_WID_INV;
	float my = (ENG::SCR_HEIGHT - ENG::MOUSE_POS_Y) * ENG::SCR_HEI_INV;
	bool bLButtonJustGotUp = ENG::MOUSE_LBUTTONUP && !ENG::MOUSE_LBUTTON;
	glDisable(GL_DEPTH_TEST);
	UINT nUI = m_arrObjectsUI.size();
	for (UINT uiO = 0; uiO < nUI; uiO++)
	{
		PFrameUI pFrameUI = m_arrObjectsUI[uiO];

		pFrameUI->Update(mx, my, bLButtonJustGotUp);
		pFrameUI->Render(m_matPerspectiveScreen.get());
	}
	if (bLButtonJustGotUp)
		ENG::MOUSE_LBUTTONUP = false;
	glEnable(GL_DEPTH_TEST);
}

CVAR(rmTest, 1, "");
VOID RenderManager::RenderObject( const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo)
{
	if (!pObj->IsInFrustrum())
		return;

	// get view, projection and light view matrices
	Mat4 arrMatrices[3];
	GetArrayOfMatrices(pObj, camView, arrMatrices);
	
	// Get shader program
	string strShaderName = "";
	PRenderedObj rndObj = rtInfo->GetRenderedObject(pObj->GetName());
	if (rndObj)
	{
		strShaderName = rndObj->m_strShader;		
	}
	else
	{
		strShaderName = rtInfo->m_strMainShaderName;		
	}
	

	GLSLProgram* pProg = m_pResMng->GetShader(strShaderName);
	if (!pProg )
		return;
	
	if (IsFlagUp(pObj->dwFlags, F_OBJ_NO_DEPTH_CULLING))
	{
		glDisable(GL_CULL_FACE);
	}
	
	pProg->BindUniformBuffer();

	pProg->Use();	
	
	
	// vDebug
	// x => Bump coefficient // y => light type // z => attenuation // w => use half vector for point light
	//////////////////////////////////////////////////////////////////////////
	UINT unfID = pProg->GetUniformLocation("vDebug");	
	if (unfID != -1)
	{	
		float vDebug[4] = {ltUseHalfector.GetFloat(), rmBumpCoff.GetFloat(), ltAttenFactor.GetFloat(), ltShinness.GetFloat()};
		m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
	}

// 	// vOffsetTexSize
// 	// x => texture Width // y => texture Height // z => texture Depth// w => Radius
// 	unfID = pProg->GetUniformLocation("vOffsetTexSize");	
// 	if (unfID != -1)
// 	{	
// 		float vDebug[4] = {texoffsetSize.GetFloat(), texoffsetSize.GetFloat(), texoffsetDepth.GetFloat(), texoffsetRadius.GetFloat()};
// 		m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
// 	}
	// vDebugCT
	// x => roughness // y => F0 // z => k // w => specular shine
	//////////////////////////////////////////////////////////////////////////
	unfID = pProg->GetUniformLocation("vDebugCT");	
	if (unfID != -1)
	{	
		float vDebug[4] = {ctRough.GetFloat(), ctF0.GetFloat(), ctK.GetFloat(), rmBumpCoff.GetFloat()};
		m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
	}

	// vDebugReflect
	// x => power of reflection // y => use plane / mesh normal // z => FREE // w => FREE
	//////////////////////////////////////////////////////////////////////////
	unfID = pProg->GetUniformLocation("vDebugReflect");	
	if (unfID != -1)
	{	
		float vDebug[4] = { perWid.GetFloat(), perPower.GetFloat(), perStep.GetFloat(), perFactor.GetFloat() * ENG::GetElapsedTime()};
		m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
	}
	UINT nMeshes = pObj->GetMeshCount();
	const PTriMesh* arrMeshes = pObj->GetMeshArray();

	GL_ERROR();

	if (pObj->dwType == F_OBJTYP_SKY)
	{
		RenderMeshSkyBox(pProg, arrMatrices, arrMeshes[0], rtInfo);
	}
	else if (pObj->dwType == F_OBJTYP_PART_SYS)
	{
		RenderParticleSystem(pObj, pProg, arrMatrices, rtInfo);
	}
	else
	{
		for (UINT uiM = 0; uiM < nMeshes; uiM++)
		{
			RenderMesh(pProg, arrMatrices, arrMeshes[uiM], rtInfo);
		}
	}

	pProg->Disable();
	GL_ERROR();

	if (IsFlagUp(pObj->dwFlags, F_OBJ_NO_DEPTH_CULLING))
	{
		glEnable(GL_CULL_FACE);
	}
}

VOID RenderManager::RenderObjectRefelct( const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo )
{
	if (!pObj->IsInFrustrum())
		return;

	
	Mat4 arrMatrices[3];
	GetArrayOfMatrices(pObj, camView, arrMatrices);

	// Reflect on Y (0)	
	arrMatrices[MATRIX_VIEW_ID].scale(1,-1,1);	
	//if (pObj->dwType != F_OBJTYP_SKY)
	glCullFace(GL_FRONT);
	// Get shader program
	string strShaderName = "";
	PRenderedObj rndObj = rtInfo->GetRenderedObject(pObj->GetName());
	if (rndObj)
	{
		strShaderName = rndObj->m_strShader;		
	}
	else
	{
		strShaderName = rtInfo->m_strMainShaderName;		
	}


	GLSLProgram* pProg = m_pResMng->GetShader(strShaderName);
	if (!pProg )
		return;
	
	pProg->BindUniformBuffer(); 
	
	pProg->Use();


	// vDebug
	// x => Bump coefficient // y => light type // z => attenuation // w => use half vector for point light
	//////////////////////////////////////////////////////////////////////////
	UINT unfID = pProg->GetUniformLocation("vDebug");	
	if (unfID != -1)
	{	
		float vDebug[4] = {ltUseHalfector.GetFloat(), rmBumpCoff.GetFloat(), ltAttenFactor.GetFloat(), ltShinness.GetFloat()};
		m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
	}

// 	vOffsetTexSize
// 		// x => texture Width // y => texture Height // z => texture Depth// w => Radius
// 		unfID = pProg->GetUniformLocation("vOffsetTexSize");	
// 		if (unfID != -1)
// 		{	
// 			float vDebug[4] = {texoffsetSize.GetFloat(), texoffsetSize.GetFloat(), texoffsetDepth.GetFloat(), texoffsetRadius.GetFloat()};
// 			m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
// 		}
	// vDebugCT
	// x => roughness // y => F0 // z => k // w => specular shine
	//////////////////////////////////////////////////////////////////////////
	unfID = pProg->GetUniformLocation("vDebugCT");	
	if (unfID != -1)
	{	
		float vDebug[4] = {ctRough.GetFloat(), ctF0.GetFloat(), ctK.GetFloat(), rmBumpCoff.GetFloat()};
		m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
	}

	// vDebugReflect
	//////////////////////////////////////////////////////////////////////////
	unfID = pProg->GetUniformLocation("vDebugReflect");	
	if (unfID != -1)
	{	
		float vDebug[4] = { perWid.GetFloat(), perPower.GetFloat(), perStep.GetFloat(), perFactor.GetFloat() * ENG::GetElapsedTime() };
		m_pOpenGL->glUniform4fv(unfID, 1, vDebug);
	}
	UINT nMeshes = pObj->GetMeshCount();
	const PTriMesh* arrMeshes = pObj->GetMeshArray();

	GL_ERROR();
	if (pObj->dwType == F_OBJTYP_WATER)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);		
	}
	if (pObj->dwType == F_OBJTYP_SKY)
	{
		RenderMeshSkyBox(pProg, arrMatrices, arrMeshes[0], rtInfo);
	}
	else
	{
		for (UINT uiM = 0; uiM < nMeshes; uiM++)
		{
			RenderMesh(pProg, arrMatrices, arrMeshes[uiM], rtInfo);
		}
	}

	pProg->Disable();
	GL_ERROR();
	if (pObj->dwType == F_OBJTYP_WATER)
		glDisable(GL_BLEND);

	glCullFace(GL_BACK);
}

VOID RenderManager::RenderObjectDepth( const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo  )
{
	if (!pObj->IsInFrustrum())
		return;
	if(IsTypeOf(pObj->dwType, F_OBJTYP_PART_SYS))
	{
		string strOwner = pObj->GetName();
		PRenderedObj rndObj = rtInfo->GetRenderedObject(strOwner);		
		string strProgPS = rndObj->m_strShader != "" ? rndObj->m_strShader : rtInfo->m_strMainShaderName;
		GLSLProgram* pProgPS = m_pResMng->GetShader(strProgPS);
		if (!pProgPS)
			return;

		RenderParticleSystemDepth(camView, pObj, pProgPS, rtInfo);
		return;
	}

	// Get shader program
	// send samplers, if any, to shader
	
	GLSLProgram* pProg = m_pResMng->GetShader(rtInfo->m_strMainShaderName);
	if (!pProg )
		return;
	

	// Get matrices
	const Mat4&	matVP = camView->GetViewProjectionMatrix();

	pProg->Use();	
	UINT nMeshes = pObj->GetMeshCount();
	const PTriMesh* arrMeshes = pObj->GetMeshArray();	
	// Get Object Meshes	
	for (UINT uiM = 0; uiM < nMeshes; uiM++)
	{
		RenderMeshDepth(pProg, matVP, arrMeshes[uiM], rtInfo);
	}
	pProg->Disable();
}

VOID RenderManager::RenderObjectDepthMVP( const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo)
{
	if (!pObj->IsInFrustrum())
		return;

	if(IsTypeOf(pObj->dwType, F_OBJTYP_PART_SYS))
	{
		string strOwner = pObj->GetName();
		PRenderedObj rndObj = rtInfo->GetRenderedObject(strOwner);		
		string strProgPS = rndObj->m_strShader != "" ? rndObj->m_strShader : rtInfo->m_strMainShaderName;
		GLSLProgram* pProgPS = m_pResMng->GetShader(strProgPS);
		if (!pProgPS)
			return;

		RenderParticleSystemDepth(camView, pObj, pProgPS, rtInfo);
		return;
	}
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const Mat4&	matVP = viewMatrix * perspectiveMatrix;
	
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader(rtInfo->m_strMainShaderName);
	if (!pProg )
		return;
	pProg->Use();	
	
	UINT nMeshes = pObj->GetMeshCount();
	const PTriMesh* arrMeshes = pObj->GetMeshArray();	
	// Get Object Meshes	
	for (UINT uiM = 0; uiM < nMeshes; uiM++)
	{
		RenderMeshDepth(pProg, matVP, arrMeshes[uiM], rtInfo);
	}
	pProg->Disable();
}

VOID RenderManager::RenderMesh( GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh, const IRenderTargetBase* rtInfo )
{
	// Send matrices to program
	//////////////////////////////////////////////////////////////////////////
	
	rtInfo->SendMatricesToProgram(pProg, arrMatrices, pMesh);	
	

	// Send material to program
	//////////////////////////////////////////////////////////////////////////	
	rtInfo->AttachMaterialAndTextures(pProg, pMesh);
	
	// Send light to program
	//////////////////////////////////////////////////////////////////////////
	pProg->SendActiveLights(m_pLightsMng->m_arrActiveLight);
	GL_ERROR();	
	
	const PRenderPack pPack = pMesh->GetRenderPack();
	
	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);
	GL_ERROR();	
	
	// Recursive
	UINT nChildren = pMesh->GetChildrenCount();
	for (UINT uiM = 0; uiM < nChildren; uiM++)
	{
		PTriMesh pchildMesh = pMesh->m_arrMeshChilds[uiM];
		RenderMesh(pProg, arrMatrices, pchildMesh, rtInfo);
	}

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);

	// Unbind the textures
	INT nSamplers = 5;
	for (INT uiS = nSamplers - 1; uiS >= 0; uiS--)
	{
		m_pOpenGL->glActiveTexture(GL_TEXTURE0 + uiS);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	GL_ERROR();	
}

VOID RenderManager::RenderMeshSkyBox( GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh, const IRenderTargetBase* rtInfo )
{
	// Send matrices to program
	//////////////////////////////////////////////////////////////////////////
	// Get model matrix
// 	Mat4 matModel = Mat4(pMesh->m_matLocal.get());	
// 	// Get view matrix
// 	Mat4 matView = Mat4(arrMatrices[MATRIX_VIEW_ID].get());	
// 	// Get Proj matrix
// 	Mat4 matProj = Mat4(arrMatrices[MATRIX_PROJ_ID].get());	
// 	pProg->SendMatrices(matModel.get(), matView.get(), matProj.get());
	rtInfo->SendMatricesToProgram(pProg, arrMatrices, pMesh);	

	GL_ERROR();
	// Cube map
	glDepthMask (GL_FALSE);
	
	rtInfo->AttachMaterialAndTextures(pProg, pMesh);

	const PRenderPack pPack = pMesh->GetRenderPack();

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);

	// Unbind the textures
	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDepthMask (GL_TRUE);

	GL_ERROR();
}

VOID RenderManager::RenderMeshDepth( GLSLProgram* pProg, const Mat4& matDepthViewProj, PTriMesh pMesh, const IRenderTargetBase* rtInfo )
{
	// Send matrices to program
	Mat4 matModel = pMesh->m_matLocal;// * matDepthViewProj;
	/*if (pMesh->GetParentMesh())
	{
		Vec3 vModelPos = pMesh->m_matLocal.getPosition();
		PTriMesh pParent =  pMesh->GetParentMesh();
		Vec3 vParent = pParent->m_matLocal.getPosition();
		matModel.setPosition(vModelPos + vParent);
	}*/
	Mat4 matMVP = matModel * matDepthViewProj;

	const float* fMaMVP = matMVP.get();
	pProg->SendMatrixMVP(fMaMVP);
	
	// send samplers, if any, to shader
	string strOwner = pMesh->GetOwnerName();
	if (pMesh->GetParentMesh())
		strOwner = pMesh->GetParentMesh()->GetName();

	PRenderedObj rndObj = rtInfo->GetRenderedObject(strOwner);
	if (rndObj)
	{
		UINT nSamplers = rndObj->m_arrSamplers.size();
		for (UINT uiS = 0; uiS < nSamplers; uiS++)
		{
			const PBaseShaderInput pShaderInput = rndObj->m_arrSamplers[uiS];
			rtInfo->SendInputToShader(pProg, pShaderInput);
		}
		float fUseSampler = nSamplers > 0 ? 1.0f : 0.0f;
		UINT unfID = pProg->GetUniformLocation("UseSampler");
		if (unfID != -1)
			m_pOpenGL->glUniform1f(unfID, fUseSampler );	
	}
	else	
	{
		UINT unfID = pProg->GetUniformLocation("UseSampler");
		if (unfID != -1)
			m_pOpenGL->glUniform1f(unfID, 0.0f);	

	}

	const PRenderPack pPack = pMesh->GetRenderPack();

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	// Recursive
	UINT nChildren = pMesh->GetChildrenCount();
	for (UINT uiM = 0; uiM < nChildren; uiM++)
	{
		PTriMesh pchildMesh = pMesh->m_arrMeshChilds[uiM];
		RenderMeshDepth(pProg, matDepthViewProj, pchildMesh, rtInfo);
	}

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);
}

VOID RenderManager::RenderParticleSystemDepth( const CameraView* camView, PBasicObj pObj, GLSLProgram* pProg, const IRenderTargetBase* rtInfo )
{
	if (!pProg)
		return;
	pProg->Use();

	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();

	ParticleSysManager* pPartSys = static_cast<ParticleSysManager*>(pObj);
	// Send material to program
	//////////////////////////////////////////////////////////////////////////	
	rtInfo->AttachMaterialAndTextures(pProg, pObj->GetName());

	UINT unfID = pProg->GetUniformLocation("vData");
	if (unfID != -1)
	{
		float vData[] = {partAtt.GetFloat(), partPointSize.GetFloat(), 0.0f, 0.0f };
		m_pOpenGL->glUniform4fv(unfID, 1, vData);
	}

	glEnable(GL_PROGRAM_POINT_SIZE);
	pProg->SendMatrices(NULL, fViewMat, fProjMat);

	m_pOpenGL->glBindVertexArray(m_pResMng->GetPartSysVOAID());

	const size_t count = pPartSys->GetDescriptor().nSize;
	if (count > 0)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//m_pOpenGL->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, count);
	}
	GL_ERROR();	

	m_pOpenGL->glBindVertexArray(0);


	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();

	GL_ERROR();	
}

//////////////////////////////////////////////////////////////////////////
//	Visual Aids
//////////////////////////////////////////////////////////////////////////
VOID RenderManager::RenderVA( VAMng* pVAMng )
{
	vector<tVAInfo>& arrData = pVAMng->GetDataArray();
	UINT uiInx = 0;
	tVAInfo& tInfo = arrData[uiInx];
	while(tInfo.bactive)
	{
		if (tInfo.eType == VA_BALL)
		{
			//RenderBallVA(tInfo.vPos, tInfo.fSize, tInfo.eColor);
			RenderBallVA(tInfo, tInfo.eColor);

		}
		else if (tInfo.eType == VA_CAPSULE)
		{
			//RenderCapsuleVA(tInfo.vPos, tInfo.vDir, tInfo.fHeight, tInfo.fSize, tInfo.eColor);
			RenderCapsuleVA(tInfo, tInfo.eColor);
		}
		else if (tInfo.eType == VA_ARROW)
		{
			RenderArrowVA(tInfo.vPos, tInfo.vDir, tInfo.fHeight, tInfo.eColor);
		}
		else if (tInfo.eType == VA_MATRIX)
		{
			RenderMatrixVA(tInfo);
		}
		else if (tInfo.eType == VA_BOX)
		{
			//RenderBoxVA(tInfo.vPos, tInfo.vDir, tInfo.eColor);
			RenderBoxVA(tInfo, tInfo.eColor);
		}
		else if (tInfo.eType == VA_PLANE)
		{
			RenderPlaneVA(tInfo.vPos, tInfo.vDir, tInfo.fSize, tInfo.eColor);
			RenderArrowVA(tInfo.vPos, tInfo.vPos + tInfo.vDir, tInfo.fSize, tInfo.eColor);
		}
		else if (tInfo.eType == VA_MESH)
		{
			RenderMeshVA(tInfo, tInfo.eColor);
			
		}

		uiInx++;
		tInfo = arrData[uiInx];

	}
}

VOID RenderManager::RenderBallVA( const Vec3& vPos, float fsize/* = 1*/ , RND_COLOR eColor /*= RED*/)
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjBall = m_pVA->m_arrAVObjs[VAOBJ_BALL];
	PTriMesh pMesh = vaObjBall->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();

	Mat4 matLocal = pMesh->m_matLocal;
	matLocal.setPosition(vPos);

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");
	float fballSize = fsize;
	
	float vSize[] = {fballSize, fballSize, fballSize, 1.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();

}

VOID RenderManager::RenderBallVA( const tVAInfo& vaInfo, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjBall = m_pVA->m_arrAVObjs[VAOBJ_BALL];
	PTriMesh pMesh = vaObjBall->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();

	Mat4 matLocal = vaInfo.matLcl;
	matLocal.setPosition(vaInfo.matLcl.getPosition());

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");
	float fballSize = vaInfo.fSize;

	float vSize[] = {fballSize, fballSize, fballSize, 1.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();
}

VOID RenderManager::RenderCapsuleVA( const Vec3& vPos, const Vec3& vTarget, float fHeight, float fsize /*= 1*/, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjCapsule = m_pVA->m_arrAVObjs[VAOBJ_CAPSULE];
	PTriMesh pMesh = vaObjCapsule->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();
	
	Mat4 matLocal;
	matLocal.identity();
	matLocal.rotate(vTarget.x, 1.0f, 0.0f, 0.0f);			
	matLocal.rotate(vTarget.y, 0.0f, 1.0f, 0.0f);			
	matLocal.rotate(vTarget.z, 0.0f, 0.0f, 1.0f);			
	matLocal.setPosition(vPos);

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");
	float fballSize = fsize;
	
	float vSize[] = {fballSize, fHeight, fballSize, 2.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();

}

VOID RenderManager::RenderCapsuleVA( const tVAInfo& vaInfo, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjCapsule = m_pVA->m_arrAVObjs[VAOBJ_CAPSULE];
	PTriMesh pMesh = vaObjCapsule->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();

	const float* fMatModel = vaInfo.matLcl.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");
	float fballSize = vaInfo.fSize;

	float vSize[] = {fballSize, vaInfo.fHeight, fballSize, 2.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();

}

VOID RenderManager::RenderArrowVA( const Vec3& vPos, const Vec3& vTar, float fHeight, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjCapsule = m_pVA->m_arrAVObjs[VAOBJ_ARROW];
	PTriMesh pMesh = vaObjCapsule->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();

	Mat4 matLocal = pMesh->m_matLocal;
	Vec3 vDir = (vTar - vPos);
	vDir.normalize();
	Vec3 U = vDir.cross(Vec3(0,1,0)).normalize();
	if (U.lengthSQ() < EPSILON)
		U = vDir.cross(Vec3(0,0,1)).normalize();
	Vec3 L = vDir.cross(U);
	matLocal.setRow(0, U);
	matLocal.setRow(1, vDir);
	matLocal.setRow(2, L);
	matLocal.setPosition(vPos);

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");	
	float vSize[] = {1.0f, fHeight, 1.0f, 6.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();

}

VOID RenderManager::RenderBoxVA( const Vec3& vPos, const Vec3& vScale, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjCapsule = m_pVA->m_arrAVObjs[VAOBJ_BOX];
	PTriMesh pMesh = vaObjCapsule->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();

	Mat4 matLocal = pMesh->m_matLocal;	
	matLocal.setPosition(vPos);

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");
	
	float vSize[] = {vScale.x, vScale.y, vScale.z, 3.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();
}

VOID RenderManager::RenderBoxVA( const tVAInfo& vaInfo, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjCapsule = m_pVA->m_arrAVObjs[VAOBJ_BOX];
	PTriMesh pMesh = vaObjCapsule->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();

	Mat4 matLocal = vaInfo.matLcl;	
	matLocal.setPosition(vaInfo.matLcl.getPosition());

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");

	float vSize[] = {vaInfo.vDir.x, vaInfo.vDir.y, vaInfo.vDir.z, 3.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();
}

VOID RenderManager::RenderMeshVA( const tVAInfo& vaInfo, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjConvMes = m_pVA->m_arrAVObjs[vaInfo.iRenderIndex];
	PTriMesh pMesh = vaObjConvMes->m_pMesh;	
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();

	Mat4 matLocal = vaInfo.matLcl;	
	matLocal.setPosition(vaInfo.matLcl.getPosition());

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");

	float vSize[] = {vaInfo.vDir.x, vaInfo.vDir.y, vaInfo.vDir.z, 5.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();
}

VOID RenderManager::RenderPlaneVA( const Vec3& vPos, const Vec3& vN, float fSize /*= 1.0f*/, RND_COLOR eColor /*= RED*/ )
{
	const CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObj = m_pVA->m_arrAVObjs.size();
	if (nObj < 1)
		return;
	PBasicVisAidObj vaObjCapsule = m_pVA->m_arrAVObjs[VAOBJ_PLANE];
	PTriMesh pMesh = vaObjCapsule->m_pMesh;
	const PRenderPack pPack = pMesh->GetRenderPack();
	// Get matrices
	const Mat4&	viewMatrix = camView->GetViewMatrix();
	const Mat4&	perspectiveMatrix = camView->GetPerspectiveMatrix();
	const float* fViewMat = viewMatrix.get();
	const float* fProjMat = perspectiveMatrix.get();
	// Get shader program
	GLSLProgram* pProg = m_pResMng->GetShader("VABasic");
	if (!pProg )
		return;
	pProg->Use();


	Vec3 N = vN;
	N.normalize();
	Mat4 matLocal = pMesh->m_matLocal;
	Vec3 U = Vec3(0,1,0);//vN;
	Vec3 vDir = N.cross(U).normalize();
	if (vDir.lengthSQ() < EPSILON)
	{
		U = Vec3(0,0,1);//vN;
		vDir = N.cross(U).normalize();
	}
	Vec3 L = vDir.cross(U);
	matLocal.setRow(0, U);
	matLocal.setRow(1, vDir);
	matLocal.setRow(2, L);
	matLocal.setPosition(vPos);

	const float* fMatModel = matLocal.get();
	pProg->SendMatrices(fMatModel, fViewMat, fProjMat);

	UINT unfID = pProg->GetUniformLocation("vSize");

	float vSize[] = {fSize, fSize, fSize, 3.0f};
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, vSize );

	unfID = pProg->GetUniformLocation("vColor");	
	float arrCol[3] = {RND_COLORS_ARRAY[eColor][0], RND_COLORS_ARRAY[eColor][1], RND_COLORS_ARRAY[eColor][2] };
	if (unfID != -1)
		m_pOpenGL->glUniform3fv(unfID, 1, arrCol );	

	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);
	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	m_pOpenGL->glBindVertexArray(0);


	pProg->Disable();
}

VOID RenderManager::RenderMatrixVA( const tVAInfo& vaInfo, RND_COLOR eColor /*= RED*/ )
{
	RenderArrowVA(vaInfo.vPos, vaInfo.vDir, vaInfo.fHeight, RED);
	RenderArrowVA(vaInfo.vPos, vaInfo.vUp, vaInfo.fHeight, BLUE);
	RenderArrowVA(vaInfo.vPos, vaInfo.vRight, vaInfo.fHeight, GREEN);

// 	RenderCapsuleVA(vaInfo.vPos, vaInfo.vDir, vaInfo.fHeight, vaInfo.fSize, RED);
// 	RenderCapsuleVA(vaInfo.vPos, vaInfo.vUp, vaInfo.fHeight, vaInfo.fSize, BLUE);
// 	RenderCapsuleVA(vaInfo.vPos, vaInfo.vRight, vaInfo.fHeight, vaInfo.fSize, GREEN);
}

EnG::PRenderPack RenderManager::GetPlaneRenderPack( Vec3 vPos, Vec3 vSize )
{
	float posX = 2.0f * (vPos.x / (FLOAT)ENG::SCR_WIDTH - 0.5f);
	float posY = -2.0f * (vPos.y / (FLOAT)ENG::SCR_HEIGHT - 0.5f);
	float sizeX = 2.0f * (vSize.x / (FLOAT)ENG::SCR_WIDTH);
	float sizeY = -2.0f * (vSize.y / (FLOAT)ENG::SCR_HEIGHT);
	float posX2 = posX + sizeX;
	float posY2 = posY + sizeY;


	PRenderPack pPack = new RenderPack();
	
	pPack->m_dwFlags = F_VBO_ALL;
	pPack->SetSize(2);


 	VertexUVN* arrVerts = pPack->m_arrVerticesUVN;
// 	// Face 1
// 	Face* pFace1 = m_arrTriMesh[0]->m_arrFace[0];
// 	pFace1->P0->m_vPos = Vec3(posX, posY, 0.0f);
// 	pFace1->P1->m_vPos = Vec3(posX, posY2, 0.0f);
// 	pFace1->P2->m_vPos = Vec3(posX2, posY, 0.0f);
// 	// Face 2
// 	Face* pFace2 = m_arrTriMesh[0]->m_arrFace[1];
// 	pFace2->P0->m_vPos = Vec3(posX2, posY2, 0.0f);
// 	pFace2->P1->m_vPos = Vec3(posX2, posY, 0.0f);
// 	pFace2->P2->m_vPos = Vec3(posX, posY2, 0.0f);
// 
	//////////////////////////////////////////////////////////////////////////
	/// Face - 1
	//P0
	arrVerts[0].x = posX;
 	arrVerts[0].y = posY;
 	arrVerts[0].z = 0.0f;			
 	// P1
 	arrVerts[1].x = posX;
 	arrVerts[1].y = posY2;
 	arrVerts[1].z = 0.0f;
 	// P2
 	arrVerts[2].x = posX2;
 	arrVerts[2].y = posY;
 	arrVerts[2].z = 0.0f;

	//////////////////////////////////////////////////////////////////////////
	/// Face - 2
	//P0
	arrVerts[3].x = posX2;
 	arrVerts[3].y = posY2;
 	arrVerts[3].z = 0.0f;			
 	// P1
 	arrVerts[4].x = posX2;
 	arrVerts[4].y = posY;
 	arrVerts[4].z = 0.0f;
 	// P2
 	arrVerts[5].x = posX;
 	arrVerts[5].y = posY2;
 	arrVerts[5].z = 0.0f;


	return pPack;

}

VOID RenderManager::ResolveMultiSampleFBO(const IRenderTargetBase* rtInfo)
{	
	if (m_ePassSide == E_RND_PASS_LEFT)
	{		
		PFBO fboMult = rtInfo->m_pFBO;
		PFBO fboResolve = rtInfo->m_pResolverFBO;
		//PFBO fboMult = m_pResMng->GetFBO(rtInfo->m_strFBO);
		//PFBO fboResolve = m_pResMng->GetFBO(rtInfo->m_strResolverFBO);	

		m_pOpenGL->glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMult->m_uiFBOID);	
		fboMult->UseRead();	
		glReadBuffer(GL_COLOR_ATTACHMENT0);


		m_pOpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboResolve->m_uiFBOID);	
		fboResolve->UseWrite();
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		m_pOpenGL->glBlitFramebuffer(
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		m_pOpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);

		fboMult->Disable();
		fboResolve->Disable();
		
		// resolve reflect 
		m_pOpenGL->glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMult->m_uiFBOID);	
		fboMult->UseRead();	
		glReadBuffer(GL_COLOR_ATTACHMENT2);


		m_pOpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboResolve->m_uiFBOID);	
		fboResolve->UseWrite();
		glDrawBuffer(GL_COLOR_ATTACHMENT2);

		m_pOpenGL->glBlitFramebuffer(
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		m_pOpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);

		fboMult->Disable();
		fboResolve->Disable();
	}
	else if (m_ePassSide == E_RND_PASS_RIGHT)
	{		
		PFBO fboMult = rtInfo->m_pFBO;
		PFBO fboResolve = rtInfo->m_pResolverFBO;
		/*PFBO  fboMult = m_pResMng->GetFBO(rtInfo->m_strFBO);
		PFBO  fboResolve = m_pResMng->GetFBO(rtInfo->m_strResolverFBO);	*/

		m_pOpenGL->glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMult->m_uiFBOID);	
		fboMult->UseRead();	
		glReadBuffer(GL_COLOR_ATTACHMENT1);

		m_pOpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboResolve->m_uiFBOID);	
		fboResolve->UseWrite();
		glDrawBuffer(GL_COLOR_ATTACHMENT1);

		m_pOpenGL->glBlitFramebuffer(
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		fboMult->Disable();
		fboResolve->Disable();

		m_pOpenGL->glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMult->m_uiFBOID);	
		fboMult->UseRead();	
		glReadBuffer(GL_COLOR_ATTACHMENT3);

		m_pOpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboResolve->m_uiFBOID);	
		fboResolve->UseWrite();
		glDrawBuffer(GL_COLOR_ATTACHMENT3);

		m_pOpenGL->glBlitFramebuffer(
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			0, 0, fboMult->m_iWidth, fboMult->m_iHeigth,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		fboMult->Disable();
		fboResolve->Disable();
	}

	m_pOpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_pOpenGL->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	m_pOpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);	


}

VOID RenderManager::SetStereo( bool bStereo )
{	
	m_bStereo = bStereo;
	m_pCamera->SetStereo(m_bStereo);
	if (m_bStereo)
	{
		// Set Stereo		
		CameraView::s_eType = eCS_STEREO;		
	}
	else
	{
		// Set Mono
		CameraView::s_eType = eCS_MONO;
	}
	
}

VOID RenderManager::RenderFBOQuad( const PFBO fbo, const string& strNameShader, double fW, double fH, double sizeW /*= 0.25*/, double sizeH/*= 0.25*/, int iColorAtt /*= 0 */ )
{
	glDisable(GL_DEPTH_TEST);
	glViewport( static_cast<GLsizei>(ENG::SCR_WIDTH * fW), static_cast<GLsizei>(ENG::SCR_HEIGHT * fH), 
		static_cast<GLsizei>(ENG::SCR_WIDTH * sizeW), static_cast<GLsizei>(ENG::SCR_HEIGHT * sizeH));
	GLSLProgram* pProg = m_pResMng->GetShader(strNameShader);
	const float* fProjMat = m_matPerspectiveScreen.get();
	// Get shader program
	if (!pProg )
		return;
	pProg->Use();
	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );
	// Screen Data
	float screenData[4] = {ENG::SCR_WID_INV, ENG::SCR_HEI_INV, Camera::GetInstance()->GetCurrentView()->GetZFar(), Camera::GetInstance()->GetCurrentView()->GetZNear()};	
	unfID = pProg->GetUniformLocation("vScreenData");		
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, screenData );

	// Bind texture from the fbo
	INT uiMainTexture = fbo->m_arrTextures[iColorAtt]; // rmTextureIdFBO = 0
	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uiMainTexture);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pOpenGL->glUniform1i(unfID, 0);

	const PRenderPack pPack = fbo->m_pRenderPack;

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);	

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);			

	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();	
}

VOID RenderManager::RenderFBOQuad(const string& strName, const string& strNameShader, double fW, double fH, double sizeW /*= 0.25*/, double sizeH /*= 0.25*/, int iColorAtt /*= 0*/ )
{
	const PFBO fbo = m_pResMng->GetFBO(strName);	 
	if (fbo)
		RenderFBOQuad(fbo, strNameShader, fW, fH, sizeW, sizeH,iColorAtt);
}

VOID RenderManager::RenderTextureQuad( const GLuint texID, const string& strNameShader, double fW, double fH, double sizeW /*= 0.25*/, double sizeH/*= 0.25*/ )
{
	glDisable(GL_DEPTH_TEST);
	glViewport( static_cast<GLsizei>(ENG::SCR_WIDTH * fW), static_cast<GLsizei>(ENG::SCR_HEIGHT * fH), 
		static_cast<GLsizei>(ENG::SCR_WIDTH * sizeW), static_cast<GLsizei>(ENG::SCR_HEIGHT * sizeH));
	GLSLProgram* pProg = m_pResMng->GetShader(strNameShader);
	const float* fProjMat = m_matPerspectiveScreen.get();
	// Get shader program
	if (!pProg )
		return;
	pProg->Use();
	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );
	// Screen Data	
	float screenData[4] = {ENG::SCR_WID_INV, ENG::SCR_HEI_INV, Camera::GetInstance()->GetCurrentView()->GetZFar(), Camera::GetInstance()->GetCurrentView()->GetZFar()};	
	unfID = pProg->GetUniformLocation("vScreenData");		
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, screenData );

	// Bind texture from the fbo
	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pOpenGL->glUniform1i(unfID, 0);

	const PRenderPack pPack = m_pResMng->GetDefaultQuadRP();

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);	

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);			

	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();	
}

VOID RenderManager::RenderLoadingQuad( const GLuint texID, GLSLProgram* pProg, float fTime, double fW, double fH, double sizeW /*= 0.25*/, double sizeH/*= 0.25*/ )
{
	glDisable(GL_DEPTH_TEST);
// 	glViewport( static_cast<GLsizei>(ENG::SCR_WIDTH * fW), static_cast<GLsizei>(ENG::SCR_HEIGHT * fH), 
// 		static_cast<GLsizei>(ENG::SCR_WIDTH * sizeW), static_cast<GLsizei>(ENG::SCR_HEIGHT * sizeH));
	glViewport( (ENG::SCR_WIDTH - fW) / 2,  (ENG::SCR_HEIGHT - fH) / 2, fW, fH);

	const float* fProjMat = m_matPerspectiveScreen.get();
	// Get shader program
	if (!pProg )
		return;
	pProg->Use();
	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );
	// Screen Data
	CameraView* pView = Camera::GetInstance()->GetCurrentView();
	if (pView)
	{
		float screenData[4] = {ENG::SCR_WID_INV, ENG::SCR_HEI_INV, Camera::GetInstance()->GetCurrentView()->GetZFar(), Camera::GetInstance()->GetCurrentView()->GetZFar()};	
		unfID = pProg->GetUniformLocation("vScreenData");		
		if (unfID != -1)
			m_pOpenGL->glUniform4fv(unfID, 1, screenData );
	}

	// Bind texture from the fbo
	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pOpenGL->glUniform1i(unfID, 0);

	unfID = pProg->GetUniformLocation("vData");
	if (unfID != -1)
	{
		/*float fVal = ((float)rand() / RAND_MAX) ;*/
		float fRedH = (ENG::SCR_HEIGHT - fH) / 2 + 280.0f;
		float vData[] = {fTime, fRedH, 0.0f, 0.0f };
		m_pOpenGL->glUniform4fv(unfID, 1, vData);
	}
	const PRenderPack pPack = m_pResMng->GetDefaultQuadRP();

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);	

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);			

	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();	
}

VOID RenderManager::RenderColorQuad( const string& strNameShader, double fW, double fH, double sizeW /*= 0.25*/, double sizeH/*= 0.25*/ )
{
	glDisable(GL_DEPTH_TEST);
	glViewport( static_cast<GLsizei>(ENG::SCR_WIDTH * fW), static_cast<GLsizei>(ENG::SCR_HEIGHT * fH), 
		static_cast<GLsizei>(ENG::SCR_WIDTH * sizeW), static_cast<GLsizei>(ENG::SCR_HEIGHT * sizeH));
	GLSLProgram* pProg = m_pResMng->GetShader(strNameShader);
	const float* fProjMat = m_matPerspectiveScreen.get();
	// Get shader program
	if (!pProg )
		return;
	pProg->Use();
	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );
	// Screen Data
	float screenData[4] = {ENG::SCR_WID_INV, ENG::SCR_HEI_INV, Camera::GetInstance()->GetCurrentView()->GetZFar(), Camera::GetInstance()->GetCurrentView()->GetZFar()};	
	unfID = pProg->GetUniformLocation("vScreenData");		
	if (unfID != -1)
		m_pOpenGL->glUniform4fv(unfID, 1, screenData );

	// Bind texture from the fbo
	unfID = pProg->GetUniformLocation("vData");
	if (unfID != -1)
	{
		float vData[] = {0.0f, cqOpacity.GetFloat(), cqBorderSize.GetFloat(), cqType.GetFloat()};
		m_pOpenGL->glUniform4fv(unfID, 1, vData);
	}

	const PRenderPack pPack = m_pResMng->GetDefaultQuadRP();

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pOpenGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);	

	// Unbind the buffer
	m_pOpenGL->glBindVertexArray(0);			

	m_pOpenGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();	
}

VOID RenderManager::RenderDebug(const IRenderTargetBase* rtInfo)
{
	if (rtInfo->m_strName != "Scene")
		return;
	CvarMng* pCvarMng = CvarMng::GetInstance();
	if (pCvarMng->IsActive())
		return;
	
	GLuint attachments[1];
	PFBO fbo = rtInfo->m_pFBO;// m_pResMng->GetFBO(rtInfo->m_strFBO);
	attachments[0] = GL_COLOR_ATTACHMENT0;
	if(!fbo)
		return;

	fbo->Use();

	const int iBuffSize = 150;
	char buff[iBuffSize];
	// FPS
	sprintf_s( buff, iBuffSize, "FPS: %.2f ", ENG::GetInstance()->FPS);	
	GLTEXT->DrawTextOS(buff, fontID.GetInt(), 10, 28, GRAY);
	// Mouse Position
	sprintf_s( buff, iBuffSize, "Mouse : (%.0f, %.0f), %d", ENG::MOUSE_POS_X, ENG::MOUSE_POS_Y, ENG::MOUSE_LBUTTON ? 1 : 0);
	GLTEXT->DrawTextOS(buff, fontID.GetInt(), 1000, 28, GRAY);
	
	RND_COLOR eCol = YELLOW;
	
	if (m_pLightsMng->GetDebugMode())
	{
		tLight* pLight = m_pLightsMng->GetActiveLight();

		// Get the intersection of plan xz with the ray of pLight->m_vPosition + t * pLight->m_vDirection.
		Vec3 vPos = Vec3(pLight->m_vPosition.x,pLight->m_vPosition.y,pLight->m_vPosition.z);
		Vec3 vDir = Vec3(pLight->m_vDirection.x,pLight->m_vDirection.y,pLight->m_vDirection.z);
		float t = -vPos.y / vDir.y;
		Vec3 vCenterProj =  vPos + t * vDir;	
		VA->RenderBall(vPos, 1.0f, RED);			
		VA->RenderArrow(vPos, vCenterProj, YELLOW);

	
		int fY = 65;
		int fDY = 20;

		RND_COLOR colPos = m_pLightsMng->GetDebugPosition() ? YELLOW : WHITE;
		RND_COLOR colDir = m_pLightsMng->GetDebugPosition() ? WHITE : YELLOW;
		GLTEXT->DrawTextOS("===| Lights |===", 0, 5, fY, GREEN); fY += fDY;		
		string strType = "Phong: ";
		strType +=  pLight->m_eType == e_LT_Direct ? "Direct" : ( pLight->m_eType == e_LT_Point ? "Point" : "Spot" );

		sprintf_s( buff, iBuffSize, "Type: %s", strType.c_str());
		GLTEXT->DrawTextOS(buff, 0, 5, fY);	fY += fDY;
		sprintf_s( buff, iBuffSize, "Position: (%.1f, %.1f, %.1f)", vPos.x, vPos.y, vPos.z);
		GLTEXT->DrawTextOS(buff, 0, 5, fY, colPos);	fY += fDY;
		sprintf_s( buff, iBuffSize, "Direction: (%.1f, %.1f, %.1f)", vDir.x, vDir.y, vDir.z);
		GLTEXT->DrawTextOS(buff, 0, 5, fY, colDir);	fY += fDY;
		if (pLight->m_eType == e_LT_Direct )
		{
			
			sprintf_s( buff, iBuffSize, "Center: (%.1f, %.1f, %.1f)", ((tLightDirect*)pLight)->m_vCameraCenter.x, ((tLightDirect*)pLight)->m_vCameraCenter.y, ((tLightDirect*)pLight)->m_vCameraCenter.z);
			GLTEXT->DrawTextOS(buff, 0, 5, fY, colDir);	fY += fDY;

		}

		sprintf_s( buff, iBuffSize, "Ambient: (%.1f, %.1f, %.1f)", pLight->m_vAmbient.x,pLight->m_vAmbient.y,pLight->m_vAmbient.z);
		GLTEXT->DrawTextOS(buff, 0, 5, fY);	fY += fDY;
		sprintf_s( buff, iBuffSize, "Specular: (%.1f, %.1f, %.1f), Shine: (%.1f)", pLight->m_vSpecular.x,pLight->m_vSpecular.y,pLight->m_vSpecular.z,pLight->m_vSpecular.w);
		GLTEXT->DrawTextOS(buff, 0, 5, fY);	fY += fDY;
		sprintf_s( buff, iBuffSize, "Diffuse: (%.1f, %.1f, %.1f)", pLight->m_vDiffuse.x,pLight->m_vDiffuse.y,pLight->m_vDiffuse.z);
		GLTEXT->DrawTextOS(buff, 0, 5, fY);	fY += fDY;
	}

	if (debugRenderObject.GetInt())
	{	
		if (m_parrSceneObjects)
		{
			PBasicObj pDbgObj = NULL;
			string strObjName = renderObjectName.GetString();
			PRenderedObj rndObj = rtInfo->GetRenderedObject(strObjName);
			
			int fY = 65;
			if (m_pLightsMng->GetDebugMode())
				fY += 200;
			if (m_pCamera->GetDebugMode())
				fY += 200;
			int fDY = 20;
			RND_COLOR colY = YELLOW;
			RND_COLOR colW = WHITE;

			if (rndObj)
			{
				sprintf_s( buff, iBuffSize, "===| Material: %s", rndObj->m_Material.m_strName.c_str());
				GLTEXT->DrawTextOS(buff, 0, 5, fY, GREEN);	fY += fDY;
				Vec4 v = rndObj->m_Material.m_vAmbient;
				sprintf_s( buff, iBuffSize, "Ambient : (%.1f, %.1f, %.1f, %.1f)", v.x, v.y, v.z, v.w);
				GLTEXT->DrawTextOS(buff, 0, 5, fY, colW);	fY += fDY;
				v = rndObj->m_Material.m_vDiffuse;
				sprintf_s( buff, iBuffSize, "Diffuse : (%.1f, %.1f, %.1f, %.1f)", v.x, v.y, v.z, v.w);
				GLTEXT->DrawTextOS(buff, 0, 5, fY, colW);	fY += fDY;
				v = rndObj->m_Material.m_vSpecular;
				sprintf_s( buff, iBuffSize, "Specular: (%.1f, %.1f, %.1f) Power: %.1f", v.x, v.y, v.z, v.w);
				GLTEXT->DrawTextOS(buff, 0, 5, fY, colW);	fY += fDY;
				v = rndObj->m_Material.m_vShadow;
				// X - Intensity, Y - Bias, Z - PCF level
				sprintf_s( buff, iBuffSize, "Shadow Intensity: %.2f", v.x);
				GLTEXT->DrawTextOS(buff, 0, 5, fY, colW);	fY += fDY;
				sprintf_s( buff, iBuffSize, "Shadow Bias: %.6f", v.y);
				GLTEXT->DrawTextOS(buff, 0, 5, fY, colW);	fY += fDY;
				sprintf_s( buff, iBuffSize, "Shadow PCF Level: %.0f", v.z);
				GLTEXT->DrawTextOS(buff, 0, 5, fY, colW);	fY += fDY;

			}
			else	
			{
				GLTEXT->DrawTextOS("===| No Degub Render Object|===", 0, 5, fY, RED); fY += fDY;
			}
		}
		
	}


	if (m_pCamera->GetDebugMode() /*RENDER_CAMERA*/)
	{
		CameraView* pCamView = m_pCamera->GetCurrentView();
		if(pCamView)
		{
			Vec3 vEye = pCamView->GetEye();
			Vec3 vCenter = pCamView->GetCenter();
			FLOAT fFov = pCamView->GetFOV();
			
			int fY = m_pLightsMng->GetDebugMode() ? 265 : 65;
			int fDY = 20;

			GLTEXT->DrawTextOS("===| Camera |===", 0, 5, fY, GREEN);
			sprintf_s( buff, iBuffSize, "Eye: (%.1f, %.1f, %.1f)", vEye.x, vEye.y, vEye.z);
			GLTEXT->DrawTextOS(buff, 0, 5, fY + fDY);			
			sprintf_s( buff, iBuffSize, "Center: (%.1f, %.1f, %.1f)", vCenter.x, vCenter.y, vCenter.z);
			GLTEXT->DrawTextOS(buff, 0, 5, fY+ 2*fDY);
			sprintf_s( buff, iBuffSize, "FOV: %.1f", fFov);
			GLTEXT->DrawTextOS(buff, 0, 5, fY+ 3*fDY);			
		}
	}
	if (RENDER_VA)
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		RenderVA(VA);
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}

	fbo->Disable();
}

VOID RenderManager::GetArrayOfMatrices( PBasicObj pObj, const CameraView* camView, Mat4* arrMat) 
{
	if (m_ePassSide == E_RND_PASS_LEFT)
	{
		// Get View Matrix
		const Mat4&	matView = camView->GetViewMatrix();	
		// Get Perspective Matrix 
		Mat4 matProj = camView->GetPerspectiveMatrix();	
		// Get (View * Perspective) Matrix From Light Point of View
		const CameraView& camLightView = m_pCamera->GetViewFromLight(m_pLightsMng->GetActiveLight(), pObj);	
		const Mat4&	matLightVP = camLightView.GetViewProjectionMatrix();
		
		arrMat[MATRIX_VIEW_ID]			=  matView;
		arrMat[MATRIX_PROJ_ID]			=  matProj;
		arrMat[MATRIX_VIEW_LIGHT_ID]	=  matLightVP;
	}
	else //if (m_ePassSide == E_RND_PASS_RIGHT)
	{
		// Get View Matrix
		const Mat4&	matView = camView->GetViewMatrixRightEye();	
		// Get Perspective Matrix 
		Mat4 matProj = camView->GetPerspectiveMatrix();	
		// Get (View * Perspective) Matrix From Light Point of View
		const CameraView& camLightView	= m_pCamera->GetViewFromLight(m_pLightsMng->GetActiveLight(), pObj);	
		const Mat4&	matLightVP			= camLightView.GetViewProjectionMatrix();

		arrMat[MATRIX_VIEW_ID]			=  matView;
		arrMat[MATRIX_PROJ_ID]			=  matProj;
		arrMat[MATRIX_VIEW_LIGHT_ID]	=  matLightVP;
	}
}

VOID RenderManager::RenderLastFBO( const string& strFBO, bool bStereo /*= FALSE */)
{
	if (bStereo)
	{
		RenderFBOQuad(strFBO, "BasicSampler", 0.0, 0.0, 0.5, 1.0, 0);
		RenderFBOQuad(strFBO, "BasicSampler", 0.5, 0.0, 0.5, 1.0, 1);
	}
	else
	{
		RenderFBOQuad(strFBO, "BasicSampler", 0.0, 0.0, 1.0, 1.0);
	}
}

BOOL RenderManager::LoadMaterialsFromXML( CChunk* a_pChunck )
{
	if (!a_pChunck)
		return FALSE;
	TIC();
	PRINT_INFO("========| Loading Materials");		

	UINT uiMaterials = a_pChunck->GetChunckCount();
	for (UINT uiObj = 0; uiObj < uiMaterials; uiObj++)
	{
		tMaterial* tMat = new tMaterial;

		CChunk cMat = a_pChunck->GetChunkXMLByID(uiObj);
		// Skip shaders
		string strSampName = cMat.GetNode().name();		
		if (strSampName.find_first_of('_') == 0)
			continue;


		cMat.ReadChildValue("Name", tMat->m_strName);
		cMat.ReadChildValue("Ambient", tMat->m_vAmbient);
		cMat.ReadChildValue("Diffuse", tMat->m_vDiffuse);
		cMat.ReadChildValue("Specular", tMat->m_vSpecular);
		cMat.ReadChildValue("Emissive", tMat->m_vEmissive);
		cMat.ReadChildValue("Bloom", tMat->m_vBloom);

		tMat->m_iIndexInArray = uiObj;
		m_pResMng->AddMaterialToMap(tMat, tMat->m_strName);

		//m_pRndMng->AddMaterial(tMat);
		PRINT_INFO("Material Name: %s", tMat->m_strName.c_str());

	}
	TOC("Loading Materials Completed... ");

	return TRUE;
}

BOOL RenderManager::InitRendererLite( OGL4* ogl )
{
	m_pOpenGL = ogl;
	m_pResMng = ResourceMng::GetInstance();// new ResourceMng(m_pOpenGL);
	m_pResMng->SetOpenGL(ogl);


	glEnable(GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST); //enable the depth testing
	glCullFace(GL_BACK);

	return TRUE;
}

VOID RenderManager::RenderToStencil( const IRenderTargetBase* rtInfo )
{
	vector<PBasicObj>  arrObjects = rtInfo->m_arrObjects;
	//Render the scene objects, no specific order
	//==============================================	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 	
	// Clear the screen and depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(0, 0, ENG::SCR_WIDTH, ENG::SCR_HEIGHT);

	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should update the stencil buffer
	glStencilMask(0xFF); // Enable writing to the stencil buffer

	CameraView* camView = m_pCamera->GetCurrentView();
	UINT nObjects = arrObjects.size();
	for (UINT uiO = 0; uiO < nObjects; uiO++)
	{
		PBasicObj pObj = arrObjects[uiO];
		RenderObject(camView, pObj, rtInfo);
	}

	glStencilMask(0x00); // Disable writing to the stencil buffer
}

VOID RenderManager::RenderRTQuad( const string strName, const PFBO fbo,  int iIndex)
{
	int iRTIndex = iIndex;
	int iRes = rtScreenRes.GetInt();
	int iCol = iRTIndex % iRes;
	int iRow = iRTIndex / iRes;
	int iX = iCol * ENG::SCR_WIDTH / iRes;
	int iY = ENG::SCR_HEIGHT - (1 + iRow) * (ENG::SCR_HEIGHT / iRes);
	
	double fRes = 1.0 / static_cast<double>(iRes);
	double fSize = fRes - 0.01;
	double fX = iCol * fRes;
	double fY = iRow * fRes;
	// First Row
	if (fbo->m_eType == EFBOType::FBO_DEPTH)
	{
		GLTEXT->DrawTextOS(strName, 0, iX + 5, iY + 22, RED);	
 		glEnable(GL_BLEND);		
		glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
		RenderFBOQuad(fbo, "DepthMapToGray",fX, fY, fSize, fSize);		
		glDisable(GL_BLEND);
	}
	else if (fbo->m_eType == EFBOType::FBO_BASIC)
	{
		GLTEXT->DrawTextOS(strName, 0, iX + 5, iY + 22, RED);	
		RenderFBOQuad(fbo, "BasicSampler", fX, fY, fSize, fSize);				
	}
	

}
#include "RenderTargetInput.h"
#include "ResourceMng.h"


CVAR(refY, 0.0f, "Water Bais");
// vDataHDR
//////////////////////////////////////////////////////////////////////////
CVAR(hdr0, -1000.0f , "HDR On / Off");
CVAR(hdr1, -1000.0f , "HDR On / Off");
CVAR(hdr2, -1000.0f , "HDR On / Off");
CVAR(hdr3, -1000.0f , "HDR On / Off");

CVAR(effEdgeDetectOn, -1.0f, "");
CVAR(effBlurOn, -1.0f, "");
CVAR(effNoiseOn, -1.0f, "");
CVAR(effFXAA, -1.0f, "");
CVAR(blurUV, -1.0f, "Blur");
CVAR(blurFunc, -1.0f, "Blur");
CVAR(noiseFator, 12.0f, "Noise");
CVAR(noiseWid, 0.00078125f, "Noise");
CVAR(noiseSpeed, 0.01f, "Noise");
CVAR(noisePower, 0.97f, "Noise");
CVAR(shadBias, 0.0f, "Noise");

CVAR(bloomDist, -1.0f, "Blur");

CVAR(edThresh, -1.0f, "Water Bais");
CVAR(fxaaX, -1.0f, "FXAA Width");
CVAR(fxaaY, -1.0f, "FXAA Height");
CVAR(fxaaZ, -1.0f, "FXAA Height");
CVAR(fxaaW, -1.0f, "FXAA Height");
bool				IRenderTargetBase::s_STEREO = false;
int					IRenderTargetBase::s_LIGHTID = 0;


//////////////////////////////////////////////////////////////////////////
// IRenderTargetBase
IRenderTargetBase::IRenderTargetBase()
{
	m_strName = "";
	m_strMainShaderName = "";
	m_strFBO = "";		
	m_strResolverFBO = "";
	m_iRTIndex = -1;
	m_iWidth = 1024;
	m_iHeight = 1024;
	m_pOpenGL = NULL;	
	m_pFBO = NULL;
	m_pResolverFBO = NULL;	
}

IRenderTargetBase::IRenderTargetBase( OGL4* pOpenGL )
{
	m_strName = "";
	m_strMainShaderName = "";
	m_strFBO = "";		
	m_strResolverFBO = "";	
	m_iRTIndex = -1;
	m_iWidth = 1024;
	m_iHeight = 1024;
	m_pOpenGL = pOpenGL;
	m_pFBO = NULL;
	m_pResolverFBO = NULL;	
}

IRenderTargetBase::~IRenderTargetBase()
{
	for (map<string, PRenderedObj>::iterator iter = m_mapRenderedObj.begin(); iter != m_mapRenderedObj.end(); iter++)
	{
		delete iter->second;
	}
	for (UINT uiS = 0; uiS < m_arrShaderInput.size(); uiS++)
	{
		delete m_arrShaderInput[uiS];
	}
	for (UINT uiS = 0; uiS < m_arrEffects.size(); uiS++)
	{
		delete m_arrEffects[uiS];
	}

	// Destruct somewhere else... TODO: smart pointers
	m_pFBO = NULL;
	m_pResolverFBO = NULL;

}

VOID IRenderTargetBase::SendInputToShader( GLSLProgram* pProg, const PBaseShaderInput pShaderInput, BOOL bStereo /*= FALSE*/ ) const
{
	ResourceMng* pResMng = ResourceMng::GetInstance();
	
	if (pShaderInput->m_eType == eSIT_FBO || pShaderInput->m_eType == eSIT_PREV_FBO)
	{
		UINT unfID = pProg->GetUniformLocation(pShaderInput->m_strName);		// Default shadow mapping, using sampler2DShadow and textureProj(...) (PCF)
		if (unfID != -1)
		{
			string fboName = pShaderInput->m_strNameFBO;
			int iColorAtt = bStereo ? pShaderInput->m_iColAttachFBO + 1: pShaderInput->m_iColAttachFBO ;
			PFBO fbo = pResMng->GetFBO(fboName);	
			if (fbo)
			{
				if (iColorAtt >= fbo->m_iCountColorAttach)
					iColorAtt = fbo->m_iCountColorAttach - 1;
				UINT uiMainTexture = fbo->m_arrTextures[iColorAtt];
				m_pOpenGL->glActiveTexture(GL_TEXTURE0 + pShaderInput->m_iBindLocation);
				glBindTexture(GL_TEXTURE_2D, uiMainTexture);
				m_pOpenGL->glUniform1i(unfID, pShaderInput->m_iBindLocation);
			}
			GL_ERROR();
		}
	}
	else if (pShaderInput->m_eType == eSIT_DEPTH_FBO)
	{
		UINT unfID = pProg->GetUniformLocation(pShaderInput->m_strName);		// Default shadow mapping, using sampler2DShadow and textureProj(...) (PCF)
		if (unfID != -1)
		{
			string fboName = pShaderInput->m_strNameFBO;
			PFBO fbo = pResMng->GetFBO(fboName);	
			if (fbo)
			{
				UINT uiMainTexture = fbo->m_uiRenderBuffer;
				m_pOpenGL->glActiveTexture(GL_TEXTURE0 + pShaderInput->m_iBindLocation);
				glBindTexture(GL_TEXTURE_2D, uiMainTexture);
				m_pOpenGL->glUniform1i(unfID, pShaderInput->m_iBindLocation);
			}
			GL_ERROR();
		}
	}
	else if (pShaderInput->m_eType == eSIT_SAMPLER)
	{
		UINT unfID = pProg->GetUniformLocation(pShaderInput->m_strName);		// Default shadow mapping, using sampler2DShadow and textureProj(...) (PCF)
		if (unfID != -1)
		{
			GLuint uiLoc = (GLuint )pShaderInput->m_iBindLocation;
			m_pOpenGL->glActiveTexture(GL_TEXTURE0 + uiLoc);
			if (IsFlagUp(pShaderInput->m_eFlags, F_SI_CUBE_MAP))
				glBindTexture(GL_TEXTURE_CUBE_MAP, pShaderInput->m_iTextureID);
			else
				glBindTexture(GL_TEXTURE_2D, pShaderInput->m_iTextureID);
			m_pOpenGL->glUniform1i(unfID, uiLoc);
			GL_ERROR();
		}
	}
	else if (pShaderInput->m_eType == eSIT_OFFSET_TEX)
	{
		UINT unfID = pProg->GetUniformLocation(pShaderInput->m_strName);		// Default shadow mapping, using sampler2DShadow and textureProj(...) (PCF)
		if (unfID != -1)
		{
			GLuint uiLoc = (GLuint )pShaderInput->m_iBindLocation + 2;
			m_pOpenGL->glActiveTexture(GL_TEXTURE0 + uiLoc);
			glBindTexture(GL_TEXTURE_3D, pShaderInput->m_iTextureID);
			m_pOpenGL->glUniform1i(unfID, uiLoc);
			GL_ERROR();
		}
	}
	else if (pShaderInput->m_eType == eSIT_FLOAT4)
	{
		UINT unfID = pProg->GetUniformLocation(pShaderInput->m_strName);		// Default shadow mapping, using sampler2DShadow and textureProj(...) (PCF)
		if (unfID != -1)
		{
			m_pOpenGL->glUniform4fv(unfID, 1, pShaderInput->m_arrVals );	
			GL_ERROR();
		}
	}
}

VOID IRenderTargetBase::SendMaterialColors( GLSLProgram* pProg, const tMaterial* matInfo) const
{
	UINT unfID = pProg->GetUniformLocation("vMatAmbient");
	if (unfID != -1)
	{
		float vData[] = { matInfo->m_vAmbient.x, matInfo->m_vAmbient.y, matInfo->m_vAmbient.z, matInfo->m_vAmbient.w };
		m_pOpenGL->glUniform4fv(unfID, 1, vData );	 
	}
	unfID = pProg->GetUniformLocation("vMatDiffuse");
	if (unfID != -1)
	{
		float vData[] = { matInfo->m_vDiffuse.x, matInfo->m_vDiffuse.y, matInfo->m_vDiffuse.z, matInfo->m_vDiffuse.w };
		m_pOpenGL->glUniform4fv(unfID, 1, vData );	
	}
	unfID = pProg->GetUniformLocation("vMatSpecular");
	if (unfID != -1)
	{
		float vData[] = { matInfo->m_vSpecular.x, matInfo->m_vSpecular.y, matInfo->m_vSpecular.z, matInfo->m_vSpecular.w };
		m_pOpenGL->glUniform4fv(unfID, 1, vData );	
	}
	unfID = pProg->GetUniformLocation("vShadowData");
	if (unfID != -1)
	{
		float fShadBias = shadBias.GetFloat() > 0.0f ? shadBias.GetFloat() :  matInfo->m_vShadow.y;//(s_LIGHTID == 0 ? matInfo->m_vShadow.y : matInfo->m_vShadow.w);
		float vData[] = { matInfo->m_vShadow.x, fShadBias, matInfo->m_vShadow.z, matInfo->m_vShadow.w };
		m_pOpenGL->glUniform4fv(unfID, 1, vData);
	}
}

PRenderedObj IRenderTargetBase::GetEffect( const string&  strName ) const
{
	for (UINT uiE = 0; uiE < m_arrEffects.size(); uiE++)
	{
		if (m_arrEffects[uiE]->m_strObjName == strName)
			return m_arrEffects[uiE];
	}
	return NULL;
}

PRenderedObj IRenderTargetBase::GetRenderedObject( const string& strName ) const
{
	string strlcObjName = strName; 
	std::transform(strlcObjName.begin(), strlcObjName.end(), strlcObjName.begin(), ::tolower);
	map<string, PRenderedObj>::const_iterator iter = m_mapRenderedObj.find(strlcObjName);
	if (iter != m_mapRenderedObj.end())
	{
		return iter->second;		
	}
	return NULL;
}

CONST VOID IRenderTargetBase::ClearBuffers() CONST
{
	// Set the color to clear the screen to.
	glClearColor(m_ClearOp.m_vBackColor.x, m_ClearOp.m_vBackColor.y, m_ClearOp.m_vBackColor.z, m_ClearOp.m_vBackColor.w); 	
	// Clear the screen and depth buffer.
	glClearDepth(m_ClearOp.m_fDepthValue);
	glClear(m_ClearOp.m_uiClear);
	glViewport(0, 0, m_ClearOp.m_iWidth, m_ClearOp.m_iHeight);
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// RenderTargetScene

VOID RenderTargetScene::AttachMaterialAndTextures( GLSLProgram* pProg, PTriMesh pMesh) const
{
	GL_ERROR();
	UINT unfID = -1;
	string strOwner = "";
	if (pMesh)
	{
		strOwner = pMesh->GetOwnerName();
		if (pMesh->GetParentMesh())
			strOwner = pMesh->GetParentMesh()->GetName();
		AttachMaterialAndTextures(pProg, strOwner);
	}
	
	
}

VOID RenderTargetScene::AttachMaterialAndTextures( GLSLProgram* pProg, const string& strOwnerName ) const
{
	PRenderedObj rndObj = GetRenderedObject(strOwnerName);
	if (!rndObj )
		return;
	ResourceMng* pResMng = ResourceMng::GetInstance();
	//////////////////////////////////////////////////////////////////////////
	// Subroutines
	//////////////////////////////////////////////////////////////////////////	
	if (IsFlagUp(pProg->m_dwFlags, F_GLSL_SUBROUTINES))
	{
		if (rndObj->m_arrSubroutinesIDs)
		{
			CONST UINT nSubroutines = rndObj->m_arrSubroutines.size();
			for (UINT uiS = 0; uiS < nSubroutines; uiS++)
			{
				if(!rndObj->m_arrSubroutines[uiS].m_bGotID)
				{ 
					rndObj->m_arrSubroutinesIDs[uiS] = m_pOpenGL->glGetSubroutineIndex(pProg->GetID(), GL_FRAGMENT_SHADER, rndObj->m_arrSubroutines[uiS].m_strName.c_str());
					rndObj->m_arrSubroutines[uiS].m_bGotID = TRUE;
				}
			}
			m_pOpenGL->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, nSubroutines, rndObj->m_arrSubroutinesIDs);
		}
		GL_ERROR();
	}
	// E/O Subroutines
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Send Material information
	//////////////////////////////////////////////////////////////////////////	
	SendMaterialColors(pProg, &rndObj->m_Material);	

	// E/O Material
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Send Samplers to program
	//////////////////////////////////////////////////////////////////////////
	UINT nSamplers = rndObj->m_arrSamplers.size();
	for (UINT uiS = 0; uiS < nSamplers; uiS++)
	{
		const PBaseShaderInput pShaderInput = rndObj->m_arrSamplers[uiS];
		SendInputToShader(pProg, pShaderInput);
	}
	// E/O Samplers
	//////////////////////////////////////////////////////////////////////////
}

VOID RenderTargetScene::SendMatricesToProgram( GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh ) const
{
	// Get model matrix
	Mat4 matModel = Mat4(pMesh->m_matLocal.get());	
	/*if (pMesh->GetParentMesh())
	{
		Vec3 vModelPos = pMesh->m_pMatLocal.getPosition();
		PTriMesh pParent =  pMesh->GetParentMesh();
		Vec3 vParent = pParent->m_pMatLocal.getPosition();
		matModel.setPosition(vModelPos + vParent);
	}*/
	// Get view matrix
	Mat4 matView = Mat4(arrMatrices[MATRIX_VIEW_ID].get());		
	// Get Proj matrix
	Mat4 matProj = Mat4(arrMatrices[MATRIX_PROJ_ID].get());	

	pProg->SendMatrices(matModel.get(), matView.get(), matProj.get());

	UINT unfID = pProg->GetUniformLocation("depthMVP");
	if (unfID != -1)
	{
		Mat4 matDepthMVP = matModel * arrMatrices[MATRIX_VIEW_LIGHT_ID];
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, matDepthMVP.get()  );
	}
	
	
	// vScreenData
	//////////////////////////////////////////////////////////////////////////
	unfID = pProg->GetUniformLocation("vScreenData");	
	if (unfID != -1)
	{
		float fTime = ENG::GetElapsedTime() * 0.015f;
		float vScreenData[4] = {ENG::SCR_WID_INV, ENG::SCR_HEI_INV, fTime, s_STEREO ? 0.0f : 1.0f };
		m_pOpenGL->glUniform4fv(unfID, 1, vScreenData);
	}
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// RenderTargetPost
VOID RenderTargetPost::SendMatricesToProgram( GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh ) const
{
	const float* fProjMat = arrMatrices[0].get();
	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pOpenGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat );

}

VOID RenderTargetPost::AttachMaterialAndTextures( GLSLProgram* pProg, PTriMesh pMesh ) const
{
	
	// HDR Filter
	//////////////////////////////////////////////////////////////////////////	
	if (hdr0.GetFloat() > -1000.0f )
	{
		PRenderedObj pRndObj = GetEffect("HDR");//m_arrEffects[0];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[0] = hdr0.GetFloat();
		//hdr0.SetValueFloat(-1.0f);
	}
	if (hdr1.GetFloat() > -1000.0f )
	{
		PRenderedObj pRndObj = GetEffect("HDR");//m_arrEffects[0];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[1] = hdr1.GetFloat();
		//hdr1.SetValueFloat(-1.0f);
	}
	if (hdr2.GetFloat() > -1000.0f )
	{
		PRenderedObj pRndObj = GetEffect("HDR");//m_arrEffects[0];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[2] = hdr2.GetFloat();
		//hdr2.SetValueFloat(-1.0f);
	}
	if (hdr3.GetFloat() > -1000.0f )
	{
		PRenderedObj pRndObj = GetEffect("HDR");//m_arrEffects[0];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[3] = hdr3.GetFloat();
		//hdr3.SetValueFloat(-1.0f);
	}

	// Edge DetectOn Filter
	//////////////////////////////////////////////////////////////////////////
	if(effEdgeDetectOn.GetFloat() >= 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("EdgeDetection");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_bActive = effEdgeDetectOn.GetFloat() > 0.5f ? true : false;
		effEdgeDetectOn.SetValueFloat(-1.0f);
	}	
	if(edThresh.GetFloat() > 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("EdgeDetection");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[2] = edThresh.GetFloat();
		edThresh.SetValueFloat(-1.0f);
	}

	// Blur Filter
	//////////////////////////////////////////////////////////////////////////
	if(effBlurOn.GetFloat() >= 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("GaussianBlurV");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_bActive = effBlurOn.GetFloat() > 0.5f ? true : false;
		pRndObj = GetEffect("GaussianBlurH");
		if (pRndObj)			
			pRndObj->m_bActive = effBlurOn.GetFloat() > 0.5f ? true : false;

		effBlurOn.SetValueFloat(-1.0f);
	}	
	if (blurUV.GetFloat() >= 0.0f)
	{
		PRenderedObj pRndObj = GetEffect("GaussianBlurV");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[1] = blurUV.GetFloat();
		pRndObj = GetEffect("GaussianBlurH");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[1] = blurUV.GetFloat();
		pRndObj = GetEffect("BloomBlurV");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[1] = blurUV.GetFloat();
		pRndObj = GetEffect("BloomBlurH");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[1] = blurUV.GetFloat();

		blurUV.SetValueFloat(-1.0f);
	}
	if (blurFunc.GetFloat() >= 0.0f)
	{
		PRenderedObj pRndObj = GetEffect("GaussianBlurV");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[2] = blurFunc.GetFloat();
		pRndObj = GetEffect("GaussianBlurH");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[2] = blurFunc.GetFloat();

		pRndObj = GetEffect("BloomBlurV");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[2] = blurFunc.GetFloat();
		pRndObj = GetEffect("BloomBlurH");
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[2] = blurFunc.GetFloat();
		blurFunc.SetValueFloat(-1.0f);
	}

	// Perlin Noise Filter
	//////////////////////////////////////////////////////////////////////////
	if(effNoiseOn.GetFloat() >= 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("PerlinNoise");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_bActive = effNoiseOn.GetFloat() > 0.5f ? true : false;
		effNoiseOn.SetValueFloat(-1.0f);
	}	

	PRenderedObj pRndObjNoise = GetEffect("PerlinNoise");
	if (pRndObjNoise && pRndObjNoise->m_bActive)			
	{		
// 		pRndObjNoise->m_arrSamplers[2]->m_arrVals[0] = noiseWid.GetFloat();
// 		pRndObjNoise->m_arrSamplers[2]->m_arrVals[1] = noisePower.GetFloat();
// 		pRndObjNoise->m_arrSamplers[2]->m_arrVals[2] = noiseFator.GetFloat();
		pRndObjNoise->m_arrSamplers[2]->m_arrVals[3] += (noiseSpeed.GetFloat() * ENG::GetDT());
	}

	// FXAA
	//////////////////////////////////////////////////////////////////////////
	if(effFXAA.GetFloat() >= 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("FXAA");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_bActive = effFXAA.GetFloat() > 0.5f ? true : false;
		effFXAA.SetValueFloat(-1.0f);
	}	
	if(fxaaX.GetFloat() > 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("FXAA");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[0] = fxaaX.GetFloat();
		fxaaX.SetValueFloat(-1.0f);
	}
	if(fxaaY.GetFloat() > 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("FXAA");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[1] = fxaaY.GetFloat();
		fxaaY.SetValueFloat(-1.0f);
	}
	if(fxaaZ.GetFloat() > 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("FXAA");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[2] = fxaaZ.GetFloat();
		fxaaZ.SetValueFloat(-1.0f);
	}
	if(fxaaW.GetFloat() > 0.0f )
	{
		PRenderedObj pRndObj = GetEffect("FXAA");//m_arrEffects[1];
		if (pRndObj)			
			pRndObj->m_arrSamplers[1]->m_arrVals[3] = fxaaW.GetFloat();
		fxaaW.SetValueFloat(-1.0f);
	}
	
	// Bloom
	//////////////////////////////////////////////////////////////////////////

	if (bloomDist.GetFloat() >= 0.0f)
	{
		PRenderedObj pRndObj = GetEffect("Bloom");
		if (pRndObj)			
			pRndObj->m_arrSamplers[2]->m_arrVals[3] = bloomDist.GetFloat();		
		bloomDist.SetValueFloat(-1.0f);
	}
}
//////////////////////////////////////////////////////////////////////////

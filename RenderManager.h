#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include "RenderTargetInput.h"
#include "FrameUI.h"
#include "..\EnGBasic\ParticleSysManager.h"
#include "FreeType.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

class RenderManager : public IMessager
{
	
public:
	static BOOL				RENDER_UI;
	static BOOL				RENDER_VA;	
	static BOOL				RENDER_CAMERA;

public:
	RenderManager();
	~RenderManager();

	virtual int				ReceiveMessage(BasicMsg *msg, const void *sender);	// Message router
	virtual BOOL			InitRendererLite(OGL4* ogl);					// Init the scene objects and resources	
	virtual BOOL			InitRenderer(OGL4* ogl, CChunk* a_pRootChunk, PCamera pCamera );					// Init the scene objects and resources	
	virtual BOOL			PostInitRenderer(CChunk* a_pRootChunk);					// Init the scene objects and resources	
	virtual BOOL			InitVisualAids(vector<PTriMesh> arrMeshes);						// Init the visual aids objects
	
	BOOL					LoadObjectVAO(PBasicObj pObj);									// Load VAO for all object meshes
	INT						LoaMeshVA(PTriMesh pMesh);										// Load VAO for all object meshes
	BOOL					ReleaseObjectVAO(PBasicObj pObj);								// Release VAO for all object meshes
	
	VOID					PreRenderUpdate(vector<PBasicObj>& arrObjects);

	VOID					RenderSceneToDepthFBO(const IRenderTargetBase* rtInfo);
	VOID					RenderToStencil(const IRenderTargetBase* rtInfo);
	VOID					RenderSceneToWaterFBO(const IRenderTargetBase* rtInfo);
	VOID					RenderSceneToFBO(const IRenderTargetBase* rtInfo);
	VOID					RenderSceneReflectToFBO(const IRenderTargetBase* rtInfo);
	
	
	VOID					PostRenderScene(const IRenderTargetBase* rtInfo, string& strFBO);
	VOID					RenderLastFBO(const string& strFBO, bool bStereo = FALSE);
	VOID					ResolveMultiSampleFBO(const IRenderTargetBase* rtInfo);
	
	VOID					RenderFont(const IRenderTargetBase* rtInfo);
	VOID					RenderUI();
	VOID					RenderVA(VAMng* pVAMng);
	
	
	VOID					RenderFBO(const PFBO fbo, GLSLProgram* pProg);
	VOID					RenderParticleSystem(PBasicObj pObj, GLSLProgram* pProg, const Mat4* arrMatrices, const IRenderTargetBase* rtInfo);

	VOID					RenderFBOMSAA(const PFBO fbo, GLSLProgram* pProg);
	// Debug
	VOID					RenderFBOQuad(const string& strName, const string& strNameShader, double fW, double fH, double sizeW = 0.25, double sizeH= 0.25, int iColorAtt = 0 );
	VOID					RenderFBOQuad(const PFBO fbo, const string& strNameShader, double fW, double fH, double sizeW = 0.25, double sizeH= 0.25, int iColorAtt = 0 );
	VOID					RenderTextureQuad(const GLuint texID, const string& strNameShader, double fW, double fH, double sizeW = 0.25, double sizeH= 0.25);
	VOID					RenderLoadingQuad(const GLuint texID, GLSLProgram* pProg, float fTime, double fW, double fH, double sizeW = 0.25, double sizeH= 0.25);
	VOID					RenderColorQuad(const string& strNameShader, double fW, double fH, double sizeW = 0.25, double sizeH= 0.25);
	VOID					RenderDebug(const IRenderTargetBase* rtInfo);
	VOID					RenderRTQuad(const string strName, const PFBO fbo, int iIndex);

	PRenderPack				GetPlaneRenderPack(Vec3 vPos, Vec3 vSize);	
	

	const BOOL				GetStereo() const		{ return m_bStereo; }
	VOID					SetStereo(bool bStereo);
	

	ERndPass				GetPassSide() const { return m_ePassSide; }
	void					SetPassSide(ERndPass val) { m_ePassSide = val; }
	
	void					SetSceneObjects(vector<PBasicObj>* pval) { if (!m_parrSceneObjects) m_parrSceneObjects = pval; }

	DWORD					m_dwFlags;

	
private:
	BOOL					LoadShadersFromXML(CChunk* a_pChunck);							// Load all shaders from XML chunk
	BOOL					LoadMaterialsFromXML(CChunk* a_pChunck);						// Load all materials from XML chunk
	BOOL					LoadFBOsFromXML(CChunk* a_pChunck);								// Load all Render Targets (FBO) from XML chunk
	BOOL					LoadFontFromXML(CChunk* a_pChunck);								// Load Font images from XML chunk
	BOOL					LoadIconsFromXML(CChunk* a_pChunck);								// Load Font images from XML chunk
	
	VOID					RenderObject(const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo);			// Render Object
	VOID					RenderObjectRefelct(const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo);			// Render Object
	VOID					RenderObjectDepth(const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo);			// Render Object
	VOID					RenderObjectDepthMVP(const CameraView* camView, PBasicObj pObj, const IRenderTargetBase* rtInfo);			// Render Object
	VOID					RenderMesh(GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh, const IRenderTargetBase* rtInfo);
	VOID					RenderMeshSkyBox(GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh, const IRenderTargetBase* rtInfo);
	VOID					RenderMeshDepth(GLSLProgram* pProg, const Mat4& matDepthViewProj, PTriMesh pMesh, const IRenderTargetBase* rtInfo);
	VOID					RenderParticleSystemDepth(const CameraView* camView, PBasicObj pObj, GLSLProgram* pProg, const IRenderTargetBase* rtInfo);
	
	VOID					GetArrayOfMatrices(PBasicObj pObj, const CameraView* camView, Mat4* arrMat);
	
	// VA Rendering
	//////////////////////////////////////////////////////////////////////////
	VOID					RenderBallVA(const Vec3& vPos, float fsize = 1.0f, RND_COLOR eColor = RED);
	VOID					RenderBallVA(const tVAInfo& vaInfo, RND_COLOR eColor = RED);
	VOID					RenderCapsuleVA(const Vec3& vPos, const Vec3& vTarget, float fHeight, float fsize = 1.0f, RND_COLOR eColor = RED);
	VOID					RenderCapsuleVA(const tVAInfo& vaInfo, RND_COLOR eColor = RED);
	VOID					RenderArrowVA(const Vec3& vPos, const Vec3& vTar, float fHeight, RND_COLOR eColor = RED);
	VOID					RenderMatrixVA(const tVAInfo& vaInfo, RND_COLOR eColor = RED);
	VOID					RenderBoxVA(const Vec3& vPos, const Vec3& vScale, RND_COLOR eColor = RED);
	VOID					RenderBoxVA(const tVAInfo& vaInfo, RND_COLOR eColor = RED);
	VOID					RenderMeshVA(const tVAInfo& vaInfo, RND_COLOR eColor = RED);
	VOID					RenderPlaneVA(const Vec3& vPos, const Vec3& vN, float fSize = 1.0f, RND_COLOR eColor = RED);
	
	OGL4*					m_pOpenGL;
	VisAids*				m_pVA;
	PCamera					m_pCamera;
	PLightsManager			m_pLightsMng;
	Mat4					m_matPerspectiveScreen;	
	ResourceMng*			m_pResMng;
	bool					m_bStereo;
	
	
	ERndPass				m_ePassSide;
	FreeTypeEng*			m_pFreeTyprFont;	
	// Counters
	vector<PBasicObj>*		m_parrSceneObjects;
	vector<PFrameUI>		m_arrObjectsUI;
	float					m_fCounter01;	
	
};

typedef RenderManager* PRenderManager;


#endif
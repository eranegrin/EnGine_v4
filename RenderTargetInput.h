#ifndef RENDER_TARGET_INPUT_H
#define RENDER_TARGET_INPUT_H

#include "ResourceMng.h"
#include "VisAids.h"
//#include "..\EnGBasic\ParserXML.h"
#include "..\EnGBasic\BasicObj.h"
#include "..\EnGBasic\Camera.h"

// Array of matrices ID
//////////////////////////////////////////////////////////////////////////
#define MATRIX_VIEW_ID		0
#define MATRIX_PROJ_ID		1
#define MATRIX_VIEW_LIGHT_ID	2
#define MATRIX_PROJ_LIGHT_ID	3
//////////////////////////////////////////////////////////////////////////

// Stereo / Mono state
//////////////////////////////////////////////////////////////////////////
enum ERndPass
{
	E_RND_PASS_LEFT		= 0,
	E_RND_PASS_RIGHT	= 1,	
};
//////////////////////////////////////////////////////////////////////////

// Shader Input Flags
//////////////////////////////////////////////////////////////////////////
#define F_SI_NONE							0
#define F_SI_CUBE_MAP						BIT(0)
//////////////////////////////////////////////////////////////////////////

// Shader Input Type
//////////////////////////////////////////////////////////////////////////
enum EShaderInputType
{
	eSIT_NONE,
	eSIT_SAMPLER,
	eSIT_FBO,
	eSIT_DEPTH_FBO,
	eSIT_PREV_FBO,
	eSIT_FLOAT4,
	eSIT_OFFSET_TEX,
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Basic Shader Input class (GL uniforms data)
//////////////////////////////////////////////////////////////////////////
class CBaseShaderInput
{
public:
	CBaseShaderInput() : m_strName(""), m_iTextureID(-1)
	{
		m_pFBO = NULL;
		m_eFlags = F_SI_NONE;
		memset(m_arrVals, 0.0f, 4 * sizeof(float));
	}

	virtual ~CBaseShaderInput(){}
	
	EShaderInputType	m_eType;			// 
	DWORD				m_eFlags;

	string				m_strName;			// Uniform / Subroutine name
	string				m_strNameFBO;		// FOB Only
	INT					m_iColAttachFBO;	// FOB Only
	INT					m_iTextureID;		// Texture ID for loaded images
	INT					m_iBindLocation;	// Binding location for samplers in shader
	float				m_arrVals[4];
 	PFBO				m_pFBO;
 	
};
typedef CBaseShaderInput* PBaseShaderInput;

class CSubroutineInfo
{
public:
	CSubroutineInfo() : m_strName(""), m_bGotID(FALSE)
	{
		
	}
	string				m_strName;			// Uniform / Subroutine name
	BOOL				m_bGotID;
};

//////////////////////////////////////////////////////////////////////////
// E/O Shader Input classes 
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Render Object Interface - contain samplers, subroutines, effects, 
// shader name (optional) etc.
//////////////////////////////////////////////////////////////////////////

class IRenderedObj
{
public:
	IRenderedObj() : m_pObj(NULL), m_strObjName(""), m_strShader(""), m_strMaterial(""), m_arrSubroutinesIDs(NULL), m_bActive(TRUE){ }
	virtual ~IRenderedObj() 
	{
		for (UINT uiS = 0; uiS < m_arrSamplers.size(); uiS++)
		{
			delete m_arrSamplers[uiS];
		}
	}

	virtual bool			Update(float fDT) = 0;
	PBasicObj				m_pObj;

	string					m_strObjName;
	string					m_strShader;
	string					m_strFBO;
	string					m_strMaterial;
	bool					m_bActive;
	int						m_iCountSubroutinse;	
	tMaterial				m_Material;

	// Shader inputs
	GLuint*							m_arrSubroutinesIDs;
	vector<CSubroutineInfo>			m_arrSubroutines;
	vector<PBaseShaderInput>		m_arrSamplers;
};


class CRenderedObj : public IRenderedObj
{
public:
	CRenderedObj() : IRenderedObj() { }
	virtual ~CRenderedObj() { }
	
	virtual bool			Update(float fDT) { return true; }
};
typedef CRenderedObj* PRenderedObj;

//////////////////////////////////////////////////////////////////////////
// E/O Render Object Interface 
//////////////////////////////////////////////////////////////////////////

struct	tClearOps
{
	tClearOps()
	{
		m_bClearColor = m_bClearDepth = m_bClearStencil = TRUE;
		m_vBackColor = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_fDepthValue = 1.0f;
		m_iWidth = 800;
		m_iHeight = 600;
		m_uiClear = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	}
	BOOL	m_bClearDepth;
	BOOL	m_bClearColor;
	BOOL	m_bClearStencil;

	Vec4	m_vBackColor;
	float	m_fDepthValue;

	GLsizei	m_iWidth;
	GLsizei	m_iHeight;
	GLuint	m_uiClear;
};


//////////////////////////////////////////////////////////////////////////
// Render Target Classes (Interface and derived class)
//////////////////////////////////////////////////////////////////////////
class IRenderTargetBase
{
public:
	IRenderTargetBase();
	IRenderTargetBase(OGL4* pOpenGL);

	virtual ~IRenderTargetBase();
	
	
	virtual VOID	AttachMaterialAndTextures(GLSLProgram* pProg, PTriMesh pMesh) const { }
	virtual VOID	AttachMaterialAndTextures(GLSLProgram* pProg, const string& strOwnerName) const { }

	virtual VOID	SendMaterialColors(GLSLProgram* pProg, const tMaterial* matInfo) const;
	virtual VOID	SendMatricesToProgram(GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh) const { }
	
	VOID			SendInputToShader(GLSLProgram* pProg, const PBaseShaderInput pShaderInput, BOOL bStereo = FALSE) const;

	PRenderedObj	GetEffect(const string& strName) const;
	PRenderedObj	GetRenderedObject(const string& strName) const;

	CONST VOID		ClearBuffers() CONST ;
	
	
	
	OGL4*						m_pOpenGL;	
	map<string, PRenderedObj>	m_mapRenderedObj;
	vector<PBaseShaderInput>	m_arrShaderInput;
	vector<CSubroutineInfo>		m_arrSubroutines;
	vector<PRenderedObj>		m_arrEffects;
	vector<PBasicObj>			m_arrObjects;
	tClearOps					m_ClearOp;

	string						m_strName;					// The Render Target Name
	string						m_strMainShaderName;
	string						m_strFBO;					// The main scene FBO
	string						m_strResolverFBO;			// The resolver FBO (for multi sampling FBO only!) 
	PFBO						m_pFBO;
	PFBO						m_pResolverFBO;
	int							m_iRTIndex;
	int							m_iWidth;
	int							m_iHeight;
	bool						m_bViewFromLight;

	static bool					s_STEREO;
	static int					s_LIGHTID;

};

class RenderTargetScene : public IRenderTargetBase
{
public:
	RenderTargetScene() : IRenderTargetBase() { }
	RenderTargetScene(OGL4* pOpenGL) : IRenderTargetBase(pOpenGL){ }	
	virtual ~RenderTargetScene() {}

	VOID					AttachMaterialAndTextures(GLSLProgram* pProg, PTriMesh pMesh) const;
	VOID					AttachMaterialAndTextures(GLSLProgram* pProg, const string& strOwnerName) const;
	VOID					SendMatricesToProgram(GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh) const;

};

class RenderTargetPost : public IRenderTargetBase
{
public:
	RenderTargetPost() : IRenderTargetBase() { }
	RenderTargetPost(OGL4* pOpenGL) : IRenderTargetBase(pOpenGL) { }		
	virtual ~RenderTargetPost() { }

	VOID					AttachMaterialAndTextures(GLSLProgram* pProg, PTriMesh pMesh) const;
	VOID					SendMatricesToProgram(GLSLProgram* pProg, const Mat4* arrMatrices, PTriMesh pMesh) const;
	struct tPostEffect
	{
		string m_strName;
		string m_strShaderName;
	};
};

#endif
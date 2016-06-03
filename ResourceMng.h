#ifndef RESOURCE_MNG_H
#define RESOURCE_MNG_H

#include <stdio.h>
#include "OGL4.h"
#include "GLSLProgram.h"
#include "..\EnGBasic\EnGBasic.h"
#include "..\EnGBasic\ParserTarga.h"
#include "..\FreeImage\Dist\x32\FreeImage.h"

typedef enum {
	E_TF_NO_FILTER	= 0,
	E_TF_BILINEAR	= 1,
	E_TF_TRILINEAR	= 2,	
} ETextureFilterType;

typedef enum {
	E_TW_REPEAT	= 0,
	E_TW_CLAMP	= 1,
	E_TW_MIRROR	= 2
} ETextureWrapMode;


struct TextureInfo
{
	TextureInfo() : m_strPath(""), m_eFilterType(E_TF_NO_FILTER) , m_bMipmap(false), m_bAnisotrpoy(false), m_bCubeMap(false), m_fAnisotrpoy(0.0f), m_eWrapMode(E_TW_REPEAT), 
		m_iInternFormat(GL_RGBA), m_iDepthFormat(GL_DEPTH_COMPONENT24), m_eType(GL_UNSIGNED_BYTE), m_iHeight(0), m_iWidth(0), m_iCompareRtoTexture(false), m_iFilterMin(GL_NEAREST), m_iFilterMag(GL_NEAREST)
	{}

	string					m_strPath;
	ETextureFilterType		m_eFilterType;
	bool					m_bMipmap;
	bool					m_bAnisotrpoy;
	bool					m_bCubeMap;
	bool					m_iCompareRtoTexture;
	GLfloat					m_fAnisotrpoy;
	ETextureWrapMode		m_eWrapMode;
	GLint					m_iInternFormat;
	GLint					m_iDepthFormat;
	GLenum					m_eType;
	GLint					m_iWidth;
	GLint					m_iHeight;
	GLint					m_iFilterMin;
	GLint					m_iFilterMag;
};

typedef enum	
{
	FBO_BASIC,
	FBO_DEPTH,
	FBO_DEPTH_WATER,
	FBO_MULTISAMPLE,
} EFBOType;

class FBOObj
{
public:
	FBOObj(OGL4* ogl) : m_iWidth(1024), m_iHeigth(768), m_iCountColorAttach(1), m_uiFBOID(0), m_pRenderPack(NULL), m_uiRenderBuffer(0)
	{
		GL = ogl;		
		m_arrTextures = new GLuint[8]; 
		memset (m_arrTextures,0, 8 * sizeof(GLuint));
	}

	void SetQuadRenderPack(const PRenderPack pPack)		{ m_pRenderPack = pPack; }
	void ReleaseAll()
	{
		for (int i=0;i<m_iCountColorAttach; i++)
			glDeleteTextures(1, &m_arrTextures[i]);
		GL->glDeleteRenderbuffers(1, &m_uiRenderBuffer);
		//Bind 0, which means render to back buffer, as a result, m_uiFBO is unbound
		GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GL->glDeleteFramebuffers(1, &m_uiFBOID);
		delete[] m_arrTextures;
	}

	void Use() const
	{
		GL->glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBOID);
// 		if (m_uiRenderBuffer > 0)		
// 			GL->glBindRenderbuffer(GL_RENDERBUFFER, m_uiRenderBuffer);
	}
	void UseRead() const
	{
		GL->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_uiFBOID);
// 		if (m_uiDepthBuffer > 0)		
// 			GL->glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBuffer);
	}
	void UseWrite() const
	{
		GL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_uiFBOID);
// 		if (m_uiDepthBuffer > 0)		
// 			GL->glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBuffer);
	}
	const int	GetNumOfColorAttachments()	const { return m_iCountColorAttach; }
	void Disable() const
	{
		GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	EFBOType		m_eType;				// the Type of the frame buffer
	int				m_iWidth;				// the width of the frame buffer
	int				m_iHeigth;				// the height of the frame buffer
	int				m_iCountColorAttach;	// amount of color attachments
	GLuint			m_uiFBOID;				// Frame buffer handle
	GLuint			m_uiRenderBuffer;		// Render buffer handle
	GLuint*			m_arrTextures;			// array of textures of the frame buffer (size of m_iCountColorAttach)	
	OGL4*			GL;						// OpenGL pointer
	PRenderPack		m_pRenderPack;			// default quad render pack
};

typedef FBOObj* PFBO;

struct tMaterial
{
	tMaterial()
	{
		m_iIndexInArray = -1;
		m_strName = "Default";
		m_vAmbient = Vec4(0.0f,0.0f,0.0f,0.0f);
		m_vEmissive = Vec4(0.0f,0.0f,0.0f,0.0f);
		m_vSpecular = Vec4(0.0f,0.0f,0.0f,0.0f);
		m_vDiffuse = Vec4(0.0f,0.0f,0.0f,0.0f);
		m_vBloom = Vec4(0.0f,0.0f,0.0f,0.0f);
		m_vShadow = Vec4(0.35f,0.00005f,2.0f,0.0f); // X - Intensity, Y - Bias, Z - PCF level
	}

	VOID Copy(tMaterial* pMaterial)
	{
		m_strName = pMaterial->m_strName;
		m_vAmbient = pMaterial->m_vAmbient;
		m_vEmissive = pMaterial->m_vEmissive;
		m_vSpecular = pMaterial->m_vSpecular;
		m_vDiffuse = pMaterial->m_vDiffuse;
		m_vBloom = pMaterial->m_vBloom;
		m_iIndexInArray = -1;
	}
	string	m_strName;
	Vec4 m_vAmbient;
	Vec4 m_vEmissive;
	Vec4 m_vSpecular;
	Vec4 m_vDiffuse;
	Vec4 m_vBloom;		
	Vec4 m_vShadow;		 // x: Intensity, y: Direct Bias, z: PCF level, w: Point Bias
	int	m_iIndexInArray;

};

using namespace EnG;

class ResourceMng
{
private:
	static bool				m_bInstanceFlag;
	static ResourceMng*		m_pInstance;	
	ResourceMng();
public:

	static ResourceMng*		GetInstance();
	~ResourceMng();
	
	VOID				SetOpenGL(OGL4* pGL) { GL = pGL; }
	//	TEXTURE OBJECTS
	////////////////////////////////////////
	
	// delete the texture objects
	void				FreeTexture(GLuint* texture, const int size = 1);
	// Get the texture object ID 
	
	// SHADERS
	////////////////////////////////////////
	GLuint				LoadTexture( CChunk* a_pChunck );
	GLuint				LoadTexture(string strPath);
	BOOL				LoadShaders(CChunk* a_pChunck);
	const int			GetShaderCount() const { return m_iShaderCount; }
	const GLSLProgram*	GetShader(const string& strName) const;
	GLSLProgram*		GetShader(const string& strName);
	GLSLProgram*		GetShader(int iIndex);
	
	// VAO - Vertex Array Object
	//////////////////////////////////////////////////////////////////////////
	BOOL				ParseMeshVAO(PTriMesh pMesh);
	BOOL				ReleaseMeshVAO(PTriMesh pMesh);
	BOOL				CreateVertexBufferArray(PRenderPack pRPack );
	BOOL				CreateVertexBufferArrayForFont(UINT& uiVertexArrayID, UINT& uiVertexBufferID, UINT& uiIndexBufferID);	
	BOOL				UpdateVertexBufferArray(PRenderPack pRPack );
	BOOL				DeleteVertexBufferArray(PRenderPack pRPack );
	BOOL				CreateVertexBufferObject(PRenderPack pRPack );

	// FBO
	//////////////////////////////////////////////////////////////////////////
	BOOL				LoadFBOs( CChunk* a_pChunck );
	BOOL				CreateDepthFBO(EFBOType eType, const TextureInfo& texInfo, const string& strName, int iColorAttachmentNum = 1);
	BOOL				CreateFBO(EFBOType eType, const TextureInfo& texInfo, const string& strName, int iColorAttachmentNum = 1);
	BOOL				CreateFBOMultiSample(EFBOType eType, const TextureInfo& texInfo, const string& strName, int iSamples, int iColorAttachmentNum = 1);

	void				ReleaseFBOs();
	const PFBO			GetFBO(const string& strName) const;
	PFBO				GetFBO(const string& strName) ;
	PRenderPack			GetDefaultQuadRP();

	// Particle system
	//////////////////////////////////////////////////////////////////////////
	BOOL				CreateParticleSystem(CONST GLuint uiSize, float* arrPos, float* arrCol);
	BOOL				UpdateParticleSystem(int iCount, float* arrPos, float* arrCol);
	BOOL				ReleaseParticleSystem();
	CONST GLuint		GetPartSysVOAID() CONST				{ return m_uipsVertexArrayID; }
	
	// Materials
	//////////////////////////////////////////////////////////////////////////
	void				BuildOffsetTexture(int texSize, int samplesU, int samplesV);
	GLuint				GetOffsetTexture() { return  m_uiOffsetTexture; }
	VOID				AddMaterialToMap(tMaterial* pMaterial, const string& strName);
	tMaterial*			GetMaterial(const string& strName);
	VOID				CopyMaterial(const string& strMaterialName, const string& strObjectName, tMaterial& material);
	// Deprecated!!!
	GLuint*				LoadTextureTGA(const int numOfPic, const TextureInfo* arrTexInfo);
	//////////////////////////////////////////////////////////////////////////

	GLuint				LoadTextureRaw(const TextureInfo& textureInfo);
	GLuint				LoadTextureCubeMap(const TextureInfo& textureInfo);

protected:
	// Deprecated!!!
	GLuint*				LoadCubeTextureTGA(const TextureInfo* arrTexInfo	);
	//////////////////////////////////////////////////////////////////////////

	// SHADERS
	//////////////////////////////////////////////////////////////////////////
	void				AddShader(GLSLProgram* pProg);		// Add shader program to map

	// FBO
	//////////////////////////////////////////////////////////////////////////
	void				LoadEmptyTexture(GLuint* uiID, const TextureInfo& texInfo);
	void				LoadEmptyTextureMultiSample(GLuint* uiID, const TextureInfo& texInfo, const int num_samples);
	void				LoadEmptyDepthTexture(GLuint* uiID, const TextureInfo& texInfo);
	void				LoadEmptyDepthTextureMultiSample(GLuint* uiID, const TextureInfo& texInfo, const int num_samples);
	bool				CheckFrameBufferStatus();

	float				jitter();
	
	OGL4*							GL;
	map<string, GLSLProgram*>		m_mapShaders;
	map<string, PFBO>				m_mapFBO;
	map<string, GLuint>				m_mapTextures;
	map<string, tMaterial*>			m_mapMaterials;
	int								m_iShaderCount;
	GLfloat							m_fMaxAnisotropy;
	PRenderPack						m_pFullQuadRP;

	GLuint							m_uiOffsetTexture;

	// Particle system 
	//////////////////////////////////////////////////////////////////////////
	GLuint							m_uiInitVelocityPS;
	GLuint							m_uiInitTimePS;
	GLuint							m_uipsVertexArrayID;
	GLuint							m_uipsVertexBufferID;
	GLuint							m_uipsColorBufferID;
	
};

inline const ETextureFilterType		AllocateImageFilter(const string& strFilter)
{
	if (strFilter == "LINEAR")
	{
		return E_TF_BILINEAR;
	}
	else if (strFilter == "TRILINEAR")
	{
		return E_TF_TRILINEAR;
	}
	return E_TF_NO_FILTER;
}
inline const GLint		AllocateImageFilter2(const string& strFilter)
{
	if (strFilter == "LINEAR")
	{
		return GL_LINEAR;
	}
	else if (strFilter == "NEAREST")
	{
		return GL_NEAREST;
	}
	return GL_NEAREST;
}
inline const ETextureWrapMode		AllocateImageWrapMode(const string& strWrapMode)
{
	if (strWrapMode == "MIRROR")
	{
		return E_TW_MIRROR;
	}
	else if (strWrapMode == "CLAMP")
	{
		return E_TW_CLAMP;
	}
	return E_TW_REPEAT;
}

inline const GLint					AllocateImageInternalFormat(const string& strFormat)
{
	if (strFormat == "RGBA16F")
	{
		return GL_RGBA16F;
	}	
	else if (strFormat == "RGBA32F")
	{
		return GL_RGBA32F;
	}
	else if (strFormat == "RGBA8")
	{
		return GL_RGBA8;
	}
	else if (strFormat == "RGBA16")
	{
		return GL_RGBA16;
	}
	return GL_RGBA;
}

inline const GLint					AllocateImageDepthFormat(const string& strFormat)
{
	if (strFormat == "DEPTH16")
	{
		return GL_DEPTH_COMPONENT16;
	}
	else if (strFormat == "DEPTH32")
	{
		return GL_DEPTH_COMPONENT32;
	}
	return GL_DEPTH_COMPONENT24;
}

inline const GLint					AllocateImageType(const GLint& iFormat)
{
	if (iFormat == GL_RGBA16F)
	{
		return GL_HALF_FLOAT;
	}
	else if (iFormat == GL_RGBA32F)
	{
		return GL_FLOAT;
	}
	return GL_UNSIGNED_BYTE;
}
#endif

/*
Internal Format	Base	Type	  Count	   Norm		  Components
--------------------------------------------------------------------
GL_R8​					ubyte		1		YES		R	0	0	1
GL_R16​					ushort		1		YES		R	0	0	1
GL_R16F​					half		1		NO		R	0	0	1
GL_R32F​					float		1		NO		R	0	0	1
GL_R8I​					byte		1		NO		R	0	0	1
GL_R16I​					short		1		NO		R	0	0	1
GL_R32I​					int			1		NO		R	0	0	1
GL_R8UI​					ubyte		1		NO		R	0	0	1
GL_R16UI​				ushort		1		NO		R	0	0	1
GL_R32UI​				uint		1		NO		R	0	0	1
GL_RG8​					ubyte		2		YES		R	G	0	1
GL_RG16​					ushort		2		YES		R	G	0	1
GL_RG16F​				half		2		NO		R	G	0	1
GL_RG32F​				float		2		NO		R	G	0	1
GL_RG8I​					byte		2		NO		R	G	0	1
GL_RG16I​				short		2		NO		R	G	0	1
GL_RG32I​				int			2		NO		R	G	0	1
GL_RG8UI​				ubyte		2		NO		R	G	0	1
GL_RG16UI​				ushort		2		NO		R	G	0	1
GL_RG32UI​				uint		2		NO		R	G	0	1
GL_RGB32F​				float		3		NO		R	G	B	1
GL_RGB32I​				int			3		NO		R	G	B	1
GL_RGB32UI​				uint		3		NO		R	G	B	1
GL_RGBA8​				ubyte		4		YES		R	G	B	A
GL_RGBA16​				ushort		4		YES		R	G	B	A
GL_RGBA16F​				half		4		NO		R	G	B	A
GL_RGBA32F​				float		4		NO		R	G	B	A
GL_RGBA8I​				byte		4		NO		R	G	B	A
GL_RGBA16I​				short		4		NO		R	G	B	A
GL_RGBA32I​				int			4		NO		R	G	B	A
GL_RGBA8UI​				ubyte		4		NO		R	G	B	A
GL_RGBA16UI​				ushort		4		NO		R	G	B	A
GL_RGBA32UI​				uint		4		NO		R	G	B	A


*/
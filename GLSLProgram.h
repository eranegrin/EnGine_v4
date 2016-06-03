#ifndef GLSL_PROGRAM_H
#define GLSL_PROGRAM_H

#include "UBOManager.h"

// DEBUG!!!
//#define UBO_TEST				1

// GLSLProgram Flags
#define F_GLSL_NONE							0
#define F_GLSL_USE_LIGHT_UBO				BIT(0)
#define F_GLSL_USE_MATERIAL_UBO				BIT(1)
#define F_GLSL_USE_BLUR_UBO					BIT(2)
#define F_GLSL_SUBROUTINES					BIT(3)

#define ATT_VEC_3				3
#define ATT_VEC_2				2
#define ATT_VERTEX_LOCATION		0
#define ATT_UV_LOCATION			1
#define ATT_NORM_LOCATION		2 
#define ATT_VERTEX_STRING		"a_Vertex"
#define ATT_UV_STRING			"a_TexCoord0"
#define ATT_NORM_STRING			"a_Normal"

//////////////////////////////////////////////////////////////////////////
//  Error List
//	1280 GL_INVALID_ENUM      
// 	1281 GL_INVALID_VALUE     
// 	1282 GL_INVALID_OPERATION 
// 	1283 GL_STACK_OVERFLOW    
// 	1284 GL_STACK_UNDERFLOW   
// 	1285 GL_OUT_OF_MEMORY
//////////////////////////////////////////////////////////////////////////

__forceinline void CHECK_GL_ERROR(string strInfo, char *file, int line)	
{
	GLenum err;
	if ((err = glGetError()) != GL_NO_ERROR)
	{
		string strFile = file;
		int iF = strFile.find_last_of('\\');
		int iL = strFile.length();
		strFile = strFile.substr(iF, iL);
		PRINT_ERROR("GL Error (%d): %s, Line: %d, %s", (int)err, strFile.c_str(), line, strInfo);
	}
}
#ifdef C_R
#define GL_ERROR()	
#else
#define GL_ERROR()	CHECK_GL_ERROR("Test", __FILE__, __LINE__)
#endif

class GLSLProgram
{
public:

	struct GLSLShader
	{
		unsigned int id;
		string strType;
		string filename;
		string source;				
	};


	GLSLProgram(OGL4* ogl);
	virtual ~GLSLProgram();

	bool					Init(const string& strShaderName, const string& vertexShader, const string& fragmentShader, void* pShaderData = NULL);	// init the shader 				
	bool					CreateShader(GLSLShader& refShader, GLuint uiType, unsigned int uiProgID);

	GLuint					GetUniformLocation(const string& strName);
	GLuint					GetAttribLocation(const string& strName);
	const string&			GetName() const { return m_strName;}
	const unsigned int		GetID()	const { return m_programID; }
	const vector<string>&	GetArrayUBO() const { return m_arrUBONames;}

	void					Unload();			// release the shader 
	void					Use()			const	{ GL->glUseProgram(m_programID); }		
	void					Disable()		const	{ GL->glUseProgram(0); }
	// Set the model, the view and the projective matrices
	void					SendMatrices(const float* fMatModel, const float* fMatView, const float* fMatProj);
	void					SendMatrices(const float* fMatModel, const float* fMatMVP );
	void					SendMatrixMVP(const float* fMatMVP);
	
	void					BindUniformBuffer();	
	const unsigned int		SetSubrutine(GLenum shaderType, const string& strSubrutineName);

	virtual void			SendActiveLights(float arrActiveLights[4]);
	virtual void			SendSubroutine(const int iCount, string* arrNames );
	virtual void			SendAdditionalData(void* pdata) { }
	
	// UBOs Specific
	void					AddUBO(const string& strUniformName, const string& strType);	
	void					FillUBOData(const CUBO* ubo, float* fData, int iSizeOfData , int iOffset );	
	void					UpdateUBOData(const CUBO* ubo, float* fData, int iSizeOfData , int iOffset );	
	
	
	DWORD						m_dwFlags;
	static unsigned int			s_BlurWeightsUBOID;		// the OGL object uniform light block ID
	static unsigned int			s_LightsUBOID;		// the OGL object uniform light block ID
	static unsigned int			s_MaterialsUBOID;		// the OGL object uniform light block ID
	

protected:

	string					ReadFile(const string& filename);
	bool					CompileShader(const GLSLShader& shader, string* strError = NULL);
	void					PrintShaderLog(unsigned int shaderID);
	
	void					CreateUniformBlockData(CUBO& ubo );
	void					GetUBOIndicesAndOffsets(const CUBO& ubo, GLuint* arrIndices, GLint* arrOffsets);
	
	OGL4*					GL;
	
	string					m_strName;			// shader name in hash
	GLSLShader				m_vertexShader;		// Data of vertex shader
	GLSLShader				m_fragmentShader;	// Data of pixel/fragment shader
	unsigned int			m_programID;		// the OGL object program ID
	unsigned int			m_LightsUBOID;		// the OGL object uniform light block ID
	unsigned int			m_MaterialsUBOID;		// the OGL object uniform light block ID
	
	
	map<int, int*>			m_mapActiveLights;	// number of active lights to send to shader	
	map<string, GLuint>		m_uniformMap;		// map of uniforms ids with name as key
	map<string, GLuint>		m_attribMap;		// map of attributes ids with name as key
	vector<string>			m_arrUBONames;			// Array of uniform buffer objects
};

#endif
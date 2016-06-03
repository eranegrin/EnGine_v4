#include "GLSLProgram.h"
#include <assert.h>

unsigned int	GLSLProgram::s_LightsUBOID			= (unsigned int)-1;	
unsigned int	GLSLProgram::s_BlurWeightsUBOID		= (unsigned int)-1;	
unsigned int	GLSLProgram::s_MaterialsUBOID		= (unsigned int)-1;	

GLSLProgram::GLSLProgram(OGL4* ogl)
{
	m_strName = "No Name";	
	m_dwFlags = 0;
	m_programID = (unsigned int)-1;
	m_LightsUBOID = (unsigned int)-1;
	m_MaterialsUBOID = (unsigned int)-1;
	m_vertexShader.strType = "Vertex Shader";
	m_fragmentShader.strType = "Fragment Shader";
	GL = ogl;
}


GLSLProgram::~GLSLProgram()
{
	Unload();
}

bool GLSLProgram::Init( const string& strShaderName, const string& vertexShader, const string& fragmentShader, void* pShaderData /*= NULL */)
{
	if (!GL)
		return false;

	// Setup
	m_strName = strShaderName;
	m_vertexShader.filename = vertexShader;
	m_fragmentShader.filename = fragmentShader;

	
	// Create Program and get the handle to it
	m_programID = GL->glCreateProgram();	
	
	// Vertex Shader (MUST)
	//////////////////////////////////////////////////////////////////////////
	m_vertexShader.source = ReadFile(m_vertexShader.filename);
	if (m_vertexShader.source.empty())
		return false;		
	
	if (!CreateShader(m_vertexShader, GL_VERTEX_SHADER, m_programID))
	{
		PRINT_ERROR("Vertex shader loading fail: %s", m_vertexShader.filename.c_str());
		return false;
	}
	
	// Fragment Shader 
	//////////////////////////////////////////////////////////////////////////
	m_fragmentShader.source = ReadFile(m_fragmentShader.filename);
	if (!m_fragmentShader.source.empty())
	{
		if (!CreateShader(m_fragmentShader, GL_FRAGMENT_SHADER, m_programID))
		{
			PRINT_ERROR("Fragment shader loading fail: %s", m_vertexShader.filename.c_str());
			return false;
		}
	}	
	else
	{
		PRINT_WARN("Fragment shader is missing: %s", m_vertexShader.filename.c_str());
	}

	// Last Step - Linking the program !!!
	// ==========================================================
	GL->glBindAttribLocation(m_programID, ATT_VERTEX_LOCATION, ATT_VERTEX_STRING);
	GL->glBindAttribLocation(m_programID, ATT_UV_LOCATION, ATT_UV_STRING);
	GL->glBindAttribLocation(m_programID, ATT_NORM_LOCATION, ATT_NORM_STRING);


	GL->glLinkProgram(m_programID);

	GL_ERROR();
	Disable();

	// Create or Bind UBO to shader
	if (m_arrUBONames.size())
	{
		for (UINT uiU = 0; uiU < m_arrUBONames.size(); uiU++)
		{
			CUBO* ubo = UBOManager::GetInstance()->GetUBO(m_arrUBONames[uiU]);
			if (ubo)
			{
				CreateUniformBlockData(*ubo);
			}
		}
	}
	return true;
}

void GLSLProgram::Unload()
{
	GL->glDetachShader(m_programID, m_vertexShader.id);
	GL->glDetachShader(m_programID, m_fragmentShader.id);
	GL->glDeleteShader(m_vertexShader.id);
	GL->glDeleteShader(m_fragmentShader.id);
	GL->glDeleteShader(m_programID);
	PRINT_INFO("Shader '%s' was released", m_strName.c_str());

}

string GLSLProgram::ReadFile( const string& filename )
{
	ifstream fileIn(filename.c_str());

	if (!fileIn.good())
	{
		PRINT_WARN("Shader %s Error", filename.c_str());
		return string();
	}

	string stringBuffer(std::istreambuf_iterator<char>(fileIn), (std::istreambuf_iterator<char>()));
	return stringBuffer;
}

bool GLSLProgram::CompileShader( const GLSLShader& shader, string* strError /*= NULL*/ )
{
	GL->glCompileShader(shader.id);
	GLint result = 0xDEADBEEF;
	GL->glGetShaderiv(shader.id, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		PrintShaderLog(shader.id);
		/*assert(FALSE && "Shader compilation fails!");*/
		return false;
	}

	return true;
}

void GLSLProgram::PrintShaderLog( unsigned int shaderID )
{
	vector<char> infoLog;
	GLint infoLen;
	GL->glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLen);
	infoLog.resize(infoLen);

	std::cerr << "GLSL Shader: Shader contains errors, please validate this shader!" << std::endl;
	GL->glGetShaderInfoLog(shaderID, sizeof(infoLog), &infoLen, &infoLog[0]);	
	string strError = string(infoLog.begin(), infoLog.end());		
	PRINT_ERROR("Shader %s Error", m_strName.c_str());
	PRINT_ERROR(strError.c_str());
}

GLuint GLSLProgram::GetUniformLocation( const string& strName )
{
	map<string, GLuint>::iterator i = m_uniformMap.find(strName);
	if (i == m_uniformMap.end())
	{
		GLuint location = GL->glGetUniformLocation(m_programID, strName.c_str());
		m_uniformMap.insert(std::make_pair(strName, location));
		return location;
	}

	return (*i).second;
}

GLuint GLSLProgram::GetAttribLocation( const string& strName )
{
	map<string, GLuint>::iterator i = m_attribMap.find(strName);
	if (i == m_attribMap.end())
	{
		GLuint location = GL->glGetAttribLocation(m_programID, strName.c_str());
		m_attribMap.insert(std::make_pair(strName, location));
		return location;
	}

	return (*i).second;
}

void GLSLProgram::SendMatrices( const float* fMatModel, const float* fMatView, const float* fMatProj )
{

	UINT unfID = GetUniformLocation("modelMatrix");
	if (unfID != -1)
		GL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fMatModel );

	unfID = GetUniformLocation("viewMatrix");
	if (unfID != -1)
		GL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fMatView);

	unfID = GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		GL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fMatProj );
}

void GLSLProgram::SendMatrices( const float* fMatModel, const float* fMatMVP )
{
	UINT unfID = GetUniformLocation("modelMatrix");
	if (unfID != -1)
		GL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fMatModel );
	
	unfID = GetUniformLocation("matMVP");
	if (unfID != -1)
		GL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fMatMVP );
}

void GLSLProgram::SendMatrixMVP( const float* fMatMVP )
{
	UINT unfID = GetUniformLocation("matMVP");
	if (unfID != -1)
		GL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fMatMVP );

}

const unsigned int GLSLProgram::SetSubrutine( GLenum shaderType, const string& strSubrutineName )
{
	GLuint uiSubroutineIndex3 = GL->glGetSubroutineIndex(m_programID, shaderType, strSubrutineName.c_str());
	GL->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &uiSubroutineIndex3);
	return 0;
}

void GLSLProgram::BindUniformBuffer(  )
{	
	UINT nUBOs = m_arrUBONames.size();
	if (!nUBOs)
		return;
	for (UINT u = 0; u < nUBOs; u++)
	{
		CUBO* pUBO = UBOManager::GetInstance()->GetUBO(m_arrUBONames[u]);
		if (!pUBO )
			continue;
		GL->glBindBuffer(GL_UNIFORM_BUFFER, pUBO->m_uiIndex);
		GL->glBindBufferBase(GL_UNIFORM_BUFFER, pUBO->m_uiBlockIndex, pUBO->m_uiIndex);
		/*GLint blockSize;
		GL->glGetActiveUniformBlockiv(m_programID, blockIndex, GL_UNIFORM_BLOCK_BINDING, &blockSize);*/
		GL->glUniformBlockBinding (m_programID, pUBO->m_uiBlockIndex, 0);
		// reset
		GL->glBindBuffer(GL_UNIFORM_BUFFER, 0);
		GL_ERROR();
	}
	
}

void GLSLProgram::SendSubroutine( const int iCount, string* arrNames )
{
	GLuint* arrSubroutineIndex = new GLuint[iCount];
	for (int uiSub = 0; uiSub < iCount; uiSub++)
	{
		arrSubroutineIndex[uiSub] = GL->glGetSubroutineIndex(m_programID, GL_FRAGMENT_SHADER, arrNames[uiSub].c_str());		
	}	
	GL->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, iCount, arrSubroutineIndex);


}

bool GLSLProgram::CreateShader( GLSLShader& refShader, GLuint uiType, unsigned int uiProgID )
{
	// Vertex Shader id
	refShader.id = GL->glCreateShader(uiType);
	// Vertex Shader source
	const GLchar* tmpV = static_cast<const GLchar*>(refShader.source.c_str());
	GL->glShaderSource(refShader.id, 1, (const GLchar**)&tmpV, NULL);

	// VS
	if (!CompileShader(refShader))
	{
		//PRINT_ERROR("%s Vertex Shader is FAIL!!", m_strName);
		return false;
	}
	GL->glAttachShader(uiProgID, refShader.id);
	return true;
}

void GLSLProgram::FillUBOData( const CUBO* ubo, float* fData, int iSizeOfData, int iOffset )
{	
	GLint iStart = 0 + iOffset * ( iSizeOfData * sizeof(float));
	GL->glBindBuffer(GL_UNIFORM_BUFFER, ubo->m_uiIndex );
	GL->glBufferSubData(GL_UNIFORM_BUFFER, iStart, iSizeOfData * sizeof(float), fData);
	GL->glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GLSLProgram::UpdateUBOData( const CUBO* ubo, float* fData, int iSizeOfData , int iOffset )
{
	GLint iStart = 0 + iOffset * ( iSizeOfData * sizeof(float));
	GL->glBindBuffer(GL_UNIFORM_BUFFER, ubo->m_uiIndex );
	GL->glBufferSubData(GL_UNIFORM_BUFFER, iStart, iSizeOfData * sizeof(float), fData);
	GL->glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GLSLProgram::SendActiveLights( float arrActiveLights[4]  )
{
	UINT unfID = GetUniformLocation("vActiveLight");
	if (unfID != -1)
		GL->glUniform4fv(unfID, 1, arrActiveLights);
}

void GLSLProgram::CreateUniformBlockData( CUBO& ubo )
{
	/*

	GLuint		m_uiIndex;
	GLuint		m_uiBlockIndex;
	GLint		m_iBlockSize;
	GLubyte*	m_byBlockBuffer;
	string		m_strUniformName;*/

	// NEGRIN TODO (22-11-14): Share UBO data between shaders, do not recreate!
	///////////////////////////////////////////////////////////////////////////
	if (ubo.m_bCreated)
	{
		//if the buffer was created, just bind it		
		GL->glBindBuffer(GL_UNIFORM_BUFFER, ubo.m_uiIndex);
		GL->glBindBufferBase(GL_UNIFORM_BUFFER, ubo.m_uiBlockIndex, ubo.m_uiIndex);
		GL_ERROR();
		// reset
		GL->glUniformBlockBinding (m_programID, ubo.m_uiBlockIndex, 0);
		GL->glBindBuffer(GL_UNIFORM_BUFFER, 0);
		return;
	}

	// Init the uniform buffer with zeros
	float vEmpty[4] = {1.0f,1.0f,1.0f,0.0f};
		
	// Get uniform block id in program
	ubo.m_uiBlockIndex = GL->glGetUniformBlockIndex(m_programID, ubo.m_strUniformName.c_str());
	// Get uniform size
	GL->glGetActiveUniformBlockiv(m_programID, ubo.m_uiBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &ubo.m_iBlockSize);
	ubo.m_byBlockBuffer = (GLubyte*)malloc(ubo.m_iBlockSize);
	
	//////////////////////////////////////////////////////////////////////////
	// Fill the blockBuffer 
	const GLuint iVariables = ubo.m_uiSize;// 4 * 6;// 4 Lights objects X 6 vector for each light 
	GLuint* arrIndices = new GLuint[iVariables];
	GLint* arrOffsets = new GLint[iVariables];	
	GetUBOIndicesAndOffsets(ubo, arrIndices, arrOffsets);
	
	// the default size is vec4 (4 floating points)
	size_t defaultSize = 4 * sizeof(GLfloat);
	for (UINT ui = 0; ui < iVariables; ui++)
	{
		memcpy(ubo.m_byBlockBuffer + arrOffsets[ui], vEmpty, defaultSize);
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create and bind the UBO
	GL->glGenBuffers(1, &ubo.m_uiIndex);
	GL->glBindBuffer(GL_UNIFORM_BUFFER, ubo.m_uiIndex);
	GL->glBufferData(GL_UNIFORM_BUFFER, ubo.m_iBlockSize, ubo.m_byBlockBuffer, GL_DYNAMIC_DRAW);
	GL->glBindBufferBase(GL_UNIFORM_BUFFER, ubo.m_uiBlockIndex, ubo.m_uiIndex);
	GL_ERROR();
	// reset
	GL->glUniformBlockBinding (m_programID, ubo.m_uiBlockIndex, 0);
	GL->glBindBuffer(GL_UNIFORM_BUFFER, 0);

	ubo.m_bCreated = true;

	delete[] arrIndices ;
	delete[] arrOffsets ;	
}

void GLSLProgram::AddUBO( const string& strUniformName, const string& strType )
{
	// Check if UBO already exists
	const CUBO* pUBO = UBOManager::GetInstance()->GetUBO(strUniformName);
	if (!pUBO )
	{
		// if not, create it 
		CUBO ubo;
		if (strType == "LIGHT")
		{
			ubo.m_eType = uboLights;
			ubo.m_uiSize = 24;
		}
		else if (strType == "BLUR")
		{
			ubo.m_eType = uboBlurWeights;
			ubo.m_uiSize = 15;
		}
		ubo.m_strUniformName = strUniformName;
		m_arrUBONames.push_back(strUniformName);		
		UBOManager::GetInstance()->AddUBOtoMap(strUniformName, ubo);
	}
	else
		m_arrUBONames.push_back(strUniformName);
	
}

void GLSLProgram::GetUBOIndicesAndOffsets( const CUBO& ubo, GLuint* arrIndices, GLint* arrOffsets )
{
 	if (ubo.m_eType == uboLights)
 	{
		const GLuint iVariables = ubo.m_uiSize;// 4 * 6;// 4 Lights objects X 6 vector for each light 
		const GLchar*	varNames[] = 
		{ 
			"Lights.arrLights[0].vPosition", "Lights.arrLights[0].vDirection", "Lights.arrLights[0].vAmbient", "Lights.arrLights[0].vDiffuse", "Lights.arrLights[0].vSpecular", "Lights.arrLights[0].vSpot", 
			"Lights.arrLights[1].vPosition", "Lights.arrLights[1].vDirection", "Lights.arrLights[1].vAmbient", "Lights.arrLights[1].vDiffuse", "Lights.arrLights[1].vSpecular", "Lights.arrLights[1].vSpot", 
			"Lights.arrLights[2].vPosition", "Lights.arrLights[2].vDirection", "Lights.arrLights[2].vAmbient", "Lights.arrLights[2].vDiffuse", "Lights.arrLights[2].vSpecular", "Lights.arrLights[2].vSpot", 
			"Lights.arrLights[3].vPosition", "Lights.arrLights[3].vDirection", "Lights.arrLights[3].vAmbient", "Lights.arrLights[3].vDiffuse", "Lights.arrLights[3].vSpecular", "Lights.arrLights[3].vSpot"
		};
		GL->glGetUniformIndices(m_programID, iVariables, varNames, arrIndices);
		GL->glGetActiveUniformsiv(m_programID, iVariables, arrIndices, GL_UNIFORM_OFFSET, arrOffsets);	

 	}
 	else // Blur
 	{
		const GLuint iVariables = ubo.m_uiSize;// 1 * 15
		// Get the offset for each variable
		const GLchar*	varNames[] = 
		{ 
			"BlurWeights.arrBlurWeights[0].vWeight",
			"BlurWeights.arrBlurWeights[1].vWeight",
			"BlurWeights.arrBlurWeights[2].vWeight",
			"BlurWeights.arrBlurWeights[3].vWeight",
			"BlurWeights.arrBlurWeights[4].vWeight",
			"BlurWeights.arrBlurWeights[5].vWeight",
			"BlurWeights.arrBlurWeights[6].vWeight",
			"BlurWeights.arrBlurWeights[7].vWeight",
			"BlurWeights.arrBlurWeights[8].vWeight",
			"BlurWeights.arrBlurWeights[9].vWeight",
			"BlurWeights.arrBlurWeights[10].vWeight",
			"BlurWeights.arrBlurWeights[11].vWeight",
			"BlurWeights.arrBlurWeights[12].vWeight",
			"BlurWeights.arrBlurWeights[13].vWeight",
			"BlurWeights.arrBlurWeights[14].vWeight"
		};
		GL->glGetUniformIndices(m_programID, iVariables, varNames, arrIndices);
		GL->glGetActiveUniformsiv(m_programID, iVariables, arrIndices, GL_UNIFORM_OFFSET, arrOffsets);	

 	}

}
//
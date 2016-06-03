#include "ResourceMng.h"
#include <windows.h>


bool					ResourceMng::m_bInstanceFlag = false;
ResourceMng*			ResourceMng::m_pInstance= NULL;

#define	MAX_CHARS_ON_SCREEN		500

ResourceMng* ResourceMng::GetInstance()
{
	if (m_bInstanceFlag)
		return m_pInstance;
	else
	{
		m_pInstance = new ResourceMng();
		m_bInstanceFlag = true;
		return m_pInstance;
	}
}

ResourceMng::ResourceMng()
{
	m_pFullQuadRP = NULL;
	GL = NULL;
	m_iShaderCount = 0;
	m_fMaxAnisotropy = 1.0f;
	m_uiInitVelocityPS = 0;
	m_uiInitTimePS = 0;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &m_fMaxAnisotropy);

}
ResourceMng::~ResourceMng()
{	
	for (map<string, GLuint>::iterator iter = m_mapTextures.begin(); iter != m_mapTextures.end(); iter++)
	{
		FreeTexture(&iter->second);
	}
	
	for (map<string, GLSLProgram*>::iterator iter = m_mapShaders.begin(); iter != m_mapShaders.end(); iter++)
	{
		iter->second->Unload();			
		delete iter->second;
	}

	for (map<string, tMaterial*>::iterator iter = m_mapMaterials.begin(); iter != m_mapMaterials.end(); iter++)
	{
		delete iter->second;
	}

	ReleaseFBOs();

}

GLuint* ResourceMng::LoadTextureTGA( const int numOfPic, const TextureInfo* arrTexInfo )
{
	GLuint*		uiTexture = new GLuint[numOfPic];	
	TargaImage* pTargaImage = new TargaImage[numOfPic];
	glGenTextures(numOfPic, uiTexture);	
	
	for (int i=0; i<numOfPic; i++)
	{
		const TextureInfo& textureInfo = arrTexInfo[i];
		const string str = textureInfo.m_strPath;
		if (!pTargaImage[i].load(textureInfo.m_strPath))
		{	
			PRINT_ERROR("Couldn't load texture %s", textureInfo.m_strPath);
			assert(false);
			return NULL;
		}	

		glBindTexture(GL_TEXTURE_2D, uiTexture[i]);

		switch(textureInfo.m_eWrapMode)
		{
		default:
		case E_TW_REPEAT:		
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case E_TW_CLAMP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case E_TW_MIRROR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			break;
		}
		if (textureInfo.m_bAnisotrpoy)
		{
			float fVal = textureInfo.m_fAnisotrpoy > m_fMaxAnisotropy ? m_fMaxAnisotropy : textureInfo.m_fAnisotrpoy;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, fVal);
		}

		switch(textureInfo.m_eFilterType)
		{
		default:
		case E_TF_NO_FILTER:	
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1 );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case E_TF_BILINEAR:	
			if (textureInfo.m_bMipmap == false)
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);									// set the build type flag
			}
			else
			{
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			}
			break;
		case E_TF_TRILINEAR:		
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);								// always mip mapping for trilinear
			break;
		}

		// crank out the texture
		if (textureInfo.m_bMipmap == false)
		{	
			if (pTargaImage[i].getBytesPerPixel() == 4) // Alpha channel !!!
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pTargaImage[i].getWidth(), pTargaImage[i].getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pTargaImage[i].getImageData());

			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pTargaImage[i].getWidth(), pTargaImage[i].getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, pTargaImage[i].getImageData());	

			}
			PRINT_INFO("Texture: %s", str.c_str());				
			PRINT_INFO("Size: %d", (pTargaImage[i].getWidth() * pTargaImage[i].getHeight() * pTargaImage[i].getBytesPerPixel()) / 1204);				
		}
		else
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, pTargaImage[i].getWidth(), pTargaImage[i].getHeight(), GL_RGB, GL_UNSIGNED_BYTE, pTargaImage[i].getImageData()); 
			PRINT_INFO("Texture: %s", str.c_str());				
			PRINT_INFO("Size: %d", (pTargaImage[i].getWidth() * pTargaImage[i].getHeight() * pTargaImage[i].getBytesPerPixel()) / 1204);				

		}		
		GL_ERROR();
	}	
	delete[] pTargaImage;

	return uiTexture;
}

GLuint ResourceMng::LoadTextureRaw( const TextureInfo& textureInfo)
{
	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0), samples(0);
	//OpenGL's image ID to map to
	GLuint gl_texID;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(textureInfo.m_strPath.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(textureInfo.m_strPath.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
		return 0;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, textureInfo.m_strPath.c_str());
	//if the image failed to load, return failure
	if(!dib)
		return 0;

	//retrieve the image data
	bits = FreeImage_GetBits(dib);
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	samples = FreeImage_GetBPP(dib) / 8;
	//if this somehow one of these failed (they shouldn't), return failure
	if((bits == 0) || (width == 0) || (height == 0))
		return 0;

	
	//generate an OpenGL texture ID for this texture
	glGenTextures(1, &gl_texID);
	
	//bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, gl_texID);
	
	
	switch(textureInfo.m_eWrapMode)
	{
	default:
	case E_TW_REPEAT:		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	case E_TW_CLAMP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case E_TW_MIRROR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;
	}
	if (textureInfo.m_bAnisotrpoy)
	{
		float fVal = textureInfo.m_fAnisotrpoy > m_fMaxAnisotropy ? m_fMaxAnisotropy : textureInfo.m_fAnisotrpoy;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, fVal);
	}

	switch(textureInfo.m_eFilterType)
	{
	default:
	case E_TF_NO_FILTER:	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1 );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case E_TF_BILINEAR:	
		if (textureInfo.m_bMipmap == false)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);									// set the build type flag
		}
		else
		{
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}
		break;
	case E_TF_TRILINEAR:		
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);								// always mip mapping for trilinear
		break;
	}

	// crank out the texture
	if (textureInfo.m_bMipmap)
	{	
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_BGR, GL_UNSIGNED_BYTE, bits); 		
	}
	else
	{
		if (samples == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, bits);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bits);		
	}		
	PRINT_INFO("Texture: %s", textureInfo.m_strPath.c_str());				
	PRINT_INFO("Size: %d", (width * height * samples) / 1204);					

	GL_ERROR();
 	


	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);

	//return success
	return gl_texID;
}

GLuint ResourceMng::LoadTextureCubeMap( const TextureInfo& textureInfo )
{
	GLuint		uiTexture;
	glGenTextures(1, &uiTexture);	

	glBindTexture(GL_TEXTURE_CUBE_MAP, uiTexture);
	const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	GLuint targets[] = { 
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};


	string str = textureInfo.m_strPath;
	string strFileName = str.substr(0, str.length() - 4);
	string strFileType = str.substr(str.length() - 4);

	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0), samples(0);
	//OpenGL's image ID to map to
	GLuint gl_texID;

	for (int i = 0; i < 6; i++)
	{	
		string strSideImageName = strFileName + "_" + suffixes[i] + strFileType;
		
		//check the file signature and deduce its format
		fif = FreeImage_GetFileType(strSideImageName.c_str(), 0);
		//if still unknown, try to guess the file format from the file extension
		if(fif == FIF_UNKNOWN) 
			fif = FreeImage_GetFIFFromFilename(strSideImageName.c_str());
		//if still unkown, return failure
		if(fif == FIF_UNKNOWN)
			return 0;

		//check that the plugin has reading capabilities and load the file
		if(FreeImage_FIFSupportsReading(fif))
			dib = FreeImage_Load(fif, strSideImageName.c_str());
		//if the image failed to load, return failure
		if(!dib)
			return 0;

		//retrieve the image data
		bits = FreeImage_GetBits(dib);
		//get the image width and height
		width = FreeImage_GetWidth(dib);
		height = FreeImage_GetHeight(dib);
		samples = FreeImage_GetBPP(dib) / 8;
		//if this somehow one of these failed (they shouldn't), return failure
		if((bits == 0) || (width == 0) || (height == 0))
			return 0;

		glTexImage2D(targets[i], 0, GL_RGBA, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, bits);

		GL_ERROR();
	}	
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	PRINT_INFO("Texture: %s", textureInfo.m_strPath.c_str());				
	PRINT_INFO("Size: 6 x %d", (width * height * samples) / 1204);					

	return uiTexture;
}

GLuint* ResourceMng::LoadCubeTextureTGA( const TextureInfo* arrTexInfo )
{
	GLuint*		uiTexture = new GLuint[1];
	TargaImage* pTargaImage = new TargaImage[6];
	glGenTextures(1, uiTexture);	
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, uiTexture[0]);
	const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	GLuint targets[] = { 
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};


	const TextureInfo& textureInfo = arrTexInfo[0];
	string str = textureInfo.m_strPath;
	string strFileName = str.substr(0, str.length() - 4);
	string strFileType = str.substr(str.length() - 4);

	for (int i = 0; i < 6; i++)
	{	
		string strSideImageName = strFileName + "_" + suffixes[i] + strFileType;
		if (!pTargaImage[i].load(strFileName + "_" + suffixes[i] + strFileType))
		{	
			PRINT_ERROR("Couldn't load texture %s", textureInfo.m_strPath);
			assert(false);
			return NULL;
		}
		glTexImage2D(targets[i],
			0, 
			GL_RGBA,
			pTargaImage[i].getWidth(),
			pTargaImage[i].getHeight(),
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			pTargaImage[i].getImageData());
		
		GL_ERROR();
	}	
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	delete[] pTargaImage;

	return uiTexture;
}

void ResourceMng::FreeTexture( GLuint* texture, const int size /*= 1*/ )
{
	glDeleteTextures(size, texture);
	PRINT_INFO("%d Texture objects was released", size);

}

void ResourceMng::AddShader( GLSLProgram* pProg )
{
	m_mapShaders.insert(pair<string, GLSLProgram*>(pProg->GetName(), pProg));
	m_iShaderCount++;
}

GLuint ResourceMng::LoadTexture( CChunk* a_pChunck )
{
	TextureInfo textureInfo;
	string strPath, strFullPath, strFilter, strWrapMode, strFormat;
	// Path
	a_pChunck->ReadChildValue("Path", strPath);
	// Find if already exists
	for (map<string, GLuint>::iterator iter = m_mapTextures.begin(); iter != m_mapTextures.end(); iter++)
	{
		if (iter->first == strPath)
			return iter->second;
	}

	textureInfo.m_strPath = ENG::FULL_PATH + strPath;

	// Filter
	a_pChunck->ReadChildValue("Filter", strFilter);
	textureInfo.m_eFilterType = AllocateImageFilter(strFilter);

	// Wrap Mode 
	a_pChunck->ReadChildValue("WrapMode", strWrapMode);
	textureInfo.m_eWrapMode = AllocateImageWrapMode(strWrapMode);

	// Anisotropic
	int iAnisotropic		= 0;
	a_pChunck->ReadChildValue("Anisotropic", iAnisotropic);
	if (iAnisotropic > 0)
	{
		textureInfo.m_bAnisotrpoy = true;
		textureInfo.m_fAnisotrpoy = static_cast<float>(iAnisotropic);
	}	

	// Texture Object internal Format
	a_pChunck->ReadChildValue("Format", strFormat);
	textureInfo.m_iInternFormat = AllocateImageInternalFormat(strFormat);

	// Cube Map
	bool bCubeMap = false;
	a_pChunck->ReadChildValue("CubeMap", bCubeMap);
	textureInfo.m_bCubeMap = bCubeMap;

	// MipMap
	bool bMipMap = false;
	a_pChunck->ReadChildValue("MipMap", bMipMap);
	textureInfo.m_bMipmap = bMipMap;		
	GLuint* pTexture = NULL;
	
	if (textureInfo.m_bCubeMap)
	{
		//pTexture = LoadCubeTextureTGA(&textureInfo);	
		GLuint uiTexture = LoadTextureCubeMap(textureInfo);	
		pTexture = new GLuint[1];
		pTexture[0] = uiTexture;
	}
	else
	{
		//pTexture = LoadTextureTGA(1, &textureInfo);	
		GLuint uiTexture = LoadTextureRaw(textureInfo);	
		pTexture = new GLuint[1];
		pTexture[0] = uiTexture;
	}

	if(pTexture)
		m_mapTextures.insert(pair<string, GLuint>(strPath, pTexture[0]));
	return pTexture[0];
}

GLuint ResourceMng::LoadTexture(string strPath)
{
	TextureInfo textureInfo;
	// Path
	// Find if already exists
	for (map<string, GLuint>::iterator iter = m_mapTextures.begin(); iter != m_mapTextures.end(); iter++)
	{
		if (iter->first == strPath)
			return iter->second;
	}

	textureInfo.m_strPath = ENG::FULL_PATH + strPath;

	// Filter
	textureInfo.m_eFilterType = AllocateImageFilter("");

	// Wrap Mode 
	textureInfo.m_eWrapMode = AllocateImageWrapMode("");

	// Texture Object internal Format
	textureInfo.m_iInternFormat = AllocateImageInternalFormat("");

	// Cube Map
	textureInfo.m_bCubeMap = false;

	// MipMap
	textureInfo.m_bMipmap = false;
	GLuint* pTexture = NULL;

	GLuint uiTexture = LoadTextureRaw(textureInfo);
	pTexture = new GLuint[1];
	pTexture[0] = uiTexture;

	if (pTexture)
		m_mapTextures.insert(pair<string, GLuint>(strPath, pTexture[0]));
	return pTexture[0];
}

BOOL ResourceMng::LoadShaders(CChunk* a_pChunck)
{

	if (!a_pChunck)
		return FALSE;
	int iProgSuccess = 0;
	TIC();
	PRINT_INFO("========| Loading Shaders");
	string	strFullPath = ENG::FULL_PATH;// + "Shaders\\";
	string	strType, strName, strVS, strFS;
	UINT nShaders = a_pChunck->GetChunckCount();
	UINT loadedShaders;
	for (UINT uiS = 0; uiS < nShaders ; uiS++)
	{
		GLSLProgram* pProg = NULL;
		CChunk chShader		= a_pChunck->GetChunkXMLByID(uiS);/*GetChunkXML("Shader", uiS);*/
		// Skip shaders
		string strSampName = chShader.GetNode().name();		
		if (strSampName.find_first_of('_') == 0)
			continue;

		chShader.ReadChildValue("Type", strType);
		chShader.ReadChildValue("Name", strName);
		chShader.ReadChildValue("VS", strVS);
		chShader.ReadChildValue("FS", strFS);
		if (strName.empty() || strVS.empty())
			continue;
		// Load the shader
		pProg = new GLSLProgram(GL); 

		// Add falgs to GLSL program boject;
		bool bFlagOn = false;
		chShader.ReadChildValueDef("LightUBO", bFlagOn, false);
		if (bFlagOn )
			AddFlag(pProg->m_dwFlags, F_GLSL_USE_LIGHT_UBO);	
		
		chShader.ReadChildValueDef("MaterialUBO", bFlagOn, false);
		if (bFlagOn )
			AddFlag(pProg->m_dwFlags, F_GLSL_USE_MATERIAL_UBO);	

		chShader.ReadChildValueDef("BlurUBO", bFlagOn, false);
		if (bFlagOn )
			AddFlag(pProg->m_dwFlags, F_GLSL_USE_BLUR_UBO);	
		
		chShader.ReadChildValueDef("Subroutines", bFlagOn, true);
		if (bFlagOn )
			AddFlag(pProg->m_dwFlags, F_GLSL_SUBROUTINES);	
		

		// Init UBOs
		CChunk* pChUBOs = chShader.GetChunkUnique("UBOs");
		if (pChUBOs)
		{
			UINT nUBS		= pChUBOs->GetChunckCount("UBO");/*GetChunkXML("Shader", uiS);*/
			for (UINT uiU = 0; uiU < nUBS; uiU++)
			{
				CChunk chUBO		= pChUBOs->GetChunkXMLByID(uiU);
				// Skip shaders
				string strUBOName = chUBO.GetNode().name();		
				if (strUBOName.find_first_of('_') == 0)
					continue;

				string strUnifName = "";
				string strUBOType = "";
				chUBO.ReadChildValue("Type", strUBOType);
				chUBO.ReadChildValue("UniformName", strUnifName);
				pProg->AddUBO(strUnifName, strUBOType);
			}
		}
		
		// Init Shader
		if (!pProg->Init(strName, strFullPath + strVS, strFullPath + strFS ))
		{
			delete pProg;
			pProg = NULL;			

			std::wstring wide = ConvertToWs("Shader: \"" + strName + "\" has errors\nThe program is going down!");

			int msgboxID = MessageBox(
				NULL,
				wide.c_str(),
				L"Shader Loading Fail",
				MB_ICONERROR | MB_OK
				);
			exit(0);
		}			

		iProgSuccess++;
		
		
		//CChunk chUBO
		AddShader(pProg);

		//m_mapShaders.insert(pair<string, PGLSLProgram>(strName, pProg));
		PRINT_INFO("Shader Name: %s", strName.c_str());
		pProg = NULL;

	}

	TOC("Loading Shaders Completed...");

	
	return TRUE;
}

BOOL ResourceMng::LoadFBOs( CChunk* a_pChunck )
{
	TIC();
	PRINT_INFO("========| Loading FBOs");
	UINT nFBO = a_pChunck->GetChunckCount();
	for (UINT uiS = 0; uiS < nFBO ; uiS++)
	{
		CChunk chFBO		= a_pChunck->GetChunkXMLByID(uiS);
		// Skip FBOs
		string strSampName = chFBO.GetNode().name();		
		if (strSampName.find_first_of('_') == 0)
			continue;

		string strName, strNameType;
		int iWid = ENG::SCR_WIDTH;
		int iHei = ENG::SCR_HEIGHT;
		int iColorAttachmentNum = 1;
		int iSamples = 1;
		chFBO.ReadChildValue("Name", strName);	
		chFBO.ReadChildValue("Type", strNameType);	
		chFBO.ReadChildValueDef("Width", iWid, 0);	
		if(iWid == 0)
			iWid = ENG::SCR_WIDTH;
		chFBO.ReadChildValueDef("Height", iHei, 0);	
		if(iHei == 0)
			iHei = ENG::SCR_HEIGHT;
		chFBO.ReadChildValue("Samples", iSamples);	
		chFBO.ReadChildValue("ColorAttachmentNum", iColorAttachmentNum);	
		
		//////////////////////////////////////////////////////////////////////////
		// Image loading
		CChunk& chFBOImage		= chFBO.GetChunkXML("Image");
		TextureInfo texInfo;

		string strFilter;
		chFBOImage.ReadChildValue("FilterMin", strFilter);	
		texInfo.m_iFilterMin = AllocateImageFilter2(strFilter);
		chFBOImage.ReadChildValue("FilterMag", strFilter);	
		texInfo.m_iFilterMag = AllocateImageFilter2(strFilter);

		string strWrapMode;
		chFBOImage.ReadChildValue("WrapMode", strWrapMode);	
		texInfo.m_eWrapMode = AllocateImageWrapMode(strWrapMode);

		string strInternalFormat;
		chFBOImage.ReadChildValue("InternalFormat", strInternalFormat);	
		texInfo.m_iInternFormat = AllocateImageInternalFormat(strInternalFormat);
		texInfo.m_eType = AllocateImageType(texInfo.m_iInternFormat);

		string strDepthFormat;
		chFBOImage.ReadChildValue("DepthFormat", strDepthFormat);	
		texInfo.m_iDepthFormat = AllocateImageDepthFormat(strDepthFormat);

		// TODO: Current- only REF_TO_TEXTURE
		string strCompareMode = "";
		chFBOImage.ReadChildValue("CompareMode", strCompareMode);	
		if (!strCompareMode.empty())
			texInfo.m_iCompareRtoTexture = TRUE;

		texInfo.m_iWidth = iWid;
		texInfo.m_iHeight = iHei;

		BOOL bRes = FALSE;
		if (strNameType == "Basic")
			bRes = CreateFBO(FBO_BASIC, texInfo, strName, iColorAttachmentNum);	
		else if (strNameType == "MultiSample")
			bRes = CreateFBOMultiSample(FBO_MULTISAMPLE, texInfo, strName, iSamples, iColorAttachmentNum);	
		else if (strNameType == "Depth")
			bRes = CreateDepthFBO(FBO_DEPTH, texInfo, strName, iColorAttachmentNum);	

		GL_ERROR();
		if (bRes)
		{
			PRINT_INFO("FBO [ %s ] loaded successfully", strName.c_str());
		}
		else
		{
			PRINT_ERROR("FBO [ %s ] Fail to load", strName.c_str());
		}
	}
	TOC("Loading FBOs Completed...");
	return TRUE;
}

BOOL ResourceMng::ParseMeshVAO( PTriMesh pMesh )
{
	if (!CreateVertexBufferArray(pMesh->GetRenderPack()))
		return FALSE;
	// Recursive for mesh children
	UINT nChildrens = pMesh->GetChildrenCount();
	for (UINT uiC = 0; uiC < nChildrens; uiC++)
	{
		PTriMesh pChild = pMesh->m_arrMeshChilds[uiC];
		if (!ParseMeshVAO(pChild))
			return FALSE;
	}
	return TRUE;
}


BOOL ResourceMng::CreateVertexBufferArray( PRenderPack pRPack )
{
	// Allocate an OpenGL vertex array object.
	GL->glGenVertexArrays(1, &pRPack->m_uiVertexArrayID);

	// Bind the vertex array object to store all the buffers and vertex attributes we create here.
	GL->glBindVertexArray(pRPack->m_uiVertexArrayID);

	// Generate an ID for the vertex buffer.
	GL->glGenBuffers(1, &pRPack->m_uiVertexBufferID);

	// Bind the vertex buffer and load the vertex (position, texture, and normal) data into the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, pRPack->m_uiVertexBufferID);
	GL->glBufferData(GL_ARRAY_BUFFER, pRPack->m_uiLength * sizeof(VertexUVN), pRPack->m_arrVerticesUVN, GL_DYNAMIC_DRAW);

	// Enable the three vertex array attributes.
	GL->glEnableVertexAttribArray(0);  // Vertex position.
	GL->glEnableVertexAttribArray(1);  // Texture coordinates.
	GL->glEnableVertexAttribArray(2);  // Normals.
	GL->glEnableVertexAttribArray(3);  // Tangent.

	// Specify the location and format of the position portion of the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, pRPack->m_uiVertexBufferID);
	GL->glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(VertexUVN), 0);

	// Specify the location and format of the texture coordinate portion of the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, pRPack->m_uiVertexBufferID);
	GL->glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(VertexUVN), (unsigned char*)NULL + (3 * sizeof(float)));

	// Specify the location and format of the normal vector portion of the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, pRPack->m_uiVertexBufferID);
	GL->glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(VertexUVN), (unsigned char*)NULL + (5 * sizeof(float)));

	// Specify the location and format of the tangent vector portion of the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, pRPack->m_uiVertexBufferID);
	GL->glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(VertexUVN), (unsigned char*)NULL + (8 * sizeof(float)));

	// Generate an ID for the index buffer.
	GL->glGenBuffers(1, &pRPack->m_uiIndexBufferID);

	// Bind the index buffer and load the index data into it.
	GL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pRPack->m_uiIndexBufferID);
	GL->glBufferData(GL_ELEMENT_ARRAY_BUFFER, pRPack->m_uiLength* sizeof(unsigned int), pRPack->m_arrIndices, GL_DYNAMIC_DRAW);
	
	return TRUE;
}


BOOL ResourceMng::CreateVertexBufferArrayForFont(UINT& uiVertexArrayID, UINT& uiVertexBufferID, UINT& uiIndexBufferID)
{
	const GLint cNumOfCahrs = MAX_CHARS_ON_SCREEN; // Test
	const GLint iSize = cNumOfCahrs * 2 * 3;
	const GLint iSizeInd = cNumOfCahrs * 2 * 3;
	
	Vertex4 arrVals[iSize];
	memset(arrVals, 0.0f, iSize * sizeof (Vertex4));
	
	GLint arrIndices[iSizeInd];
	for (GLint i = 0; i < iSizeInd; i++)
		arrIndices[i] = i;
	//memset(arrIndices, 0, iSizeInd * sizeof (GLint));
	// Allocate an OpenGL vertex array object.
	GL->glGenVertexArrays(1, &uiVertexArrayID);

	// Bind the vertex array object to store all the buffers and vertex attributes we create here.
	GL->glBindVertexArray(uiVertexArrayID);

	// Generate an ID for the vertex buffer.
	GL->glGenBuffers(1, &uiVertexBufferID);

	// Bind the vertex buffer and load the vertex (position and texture combin) data into the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, uiVertexBufferID);
	GL->glBufferData(GL_ARRAY_BUFFER, iSize * sizeof(Vertex4), arrVals, GL_STATIC_DRAW);

	// Enable the three vertex array attributes.
	GL->glEnableVertexAttribArray(0);  // Vertex position.
	GL->glEnableVertexAttribArray(1);  // Vertex color.
	
	// Specify the location and format of the position portion of the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, uiVertexBufferID);
	GL->glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(Vertex4), 0);

	// Specify the location and format of the position portion of the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, uiVertexBufferID);
	GL->glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex4), (unsigned char*)NULL + (4 * sizeof(float)));
	
	// Generate an ID for the index buffer.
	GL->glGenBuffers(1, &uiIndexBufferID);

	// Bind the index buffer and load the index data into it.
	GL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexBufferID);
	GL->glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize* sizeof(unsigned int), arrIndices, GL_STATIC_DRAW);
	GL_ERROR();

	return TRUE;
}

BOOL ResourceMng::UpdateVertexBufferArray( PRenderPack pRPack )
{	
	
	// Bind the vertex buffer and load the vertex (position, texture, and normal) data into the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, pRPack->m_uiVertexArrayID);
	GL->glBufferSubData(GL_ARRAY_BUFFER, 0, pRPack->m_uiLength * sizeof(VertexUVN), pRPack->m_arrVerticesUVN);
	GL->glBindBuffer(GL_ARRAY_BUFFER, 0);

	return TRUE;
}

BOOL ResourceMng::CreateVertexBufferObject( PRenderPack pRPack )
{	
	// Generating buffer names 
	GLuint gluiBuffID = 0;
	GL->glGenBuffers(1, &gluiBuffID);

	// Bind the vertex buffer and load the vertex (position, texture, and normal) data into the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, gluiBuffID);
	// Fill the buffer
	GLsizeiptr iptrSize = sizeof(float) * pRPack->GetVertLength();
	GL->glBufferData(GL_ARRAY_BUFFER, iptrSize, pRPack->m_arrVert, GL_DYNAMIC_DRAW);

	GLint iSize;
	GL->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &iSize);
	if (iSize != iptrSize)
	{
		PRINT_ERROR("Vertex VBO Size is different !!!");
		GL->glBindBuffer(GL_ARRAY_BUFFER, 0);
		return FALSE;
	}
	GL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	pRPack->m_uiVertVBOID = static_cast<UINT>(gluiBuffID);
	return TRUE;
}

BOOL ResourceMng::ReleaseMeshVAO( PTriMesh pMesh )
{
	if (!DeleteVertexBufferArray(pMesh->GetRenderPack()))
		return FALSE;
	// Recursive for mesh children
	UINT nChildrens = pMesh->GetChildrenCount();
	for (UINT uiC = 0; uiC < nChildrens; uiC++)
	{
		PTriMesh pChild = pMesh->m_arrMeshChilds[uiC];
		if (!ReleaseMeshVAO(pChild))
			return FALSE;
	}
	
	return TRUE;
}

BOOL ResourceMng::DeleteVertexBufferArray( PRenderPack pRPack )
{	
	// Disable the two vertex array attributes.
	GL->glDisableVertexAttribArray(0);
	GL->glDisableVertexAttribArray(1);
	GL->glDisableVertexAttribArray(2);
	GL->glDisableVertexAttribArray(3);

	// Release the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL->glDeleteBuffers(1, &pRPack->m_uiVertexBufferID);

	// Release the index buffer.
	GL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GL->glDeleteBuffers(1, &pRPack->m_uiIndexBufferID);

	// Release the vertex array object.
	GL->glBindVertexArray(0);
	GL->glDeleteVertexArrays(1, &pRPack->m_uiVertexArrayID);
	PRINT_INFO("Mesh VAO %s was released", pRPack->m_strName.c_str());
	
	return TRUE;
}

const GLSLProgram* ResourceMng::GetShader( const string& strName )	const
{
	if (strName == "")
	{
		map<string, GLSLProgram*>::const_iterator iter = m_mapShaders.begin();
		return iter->second;
	}
	map<string, GLSLProgram*>::const_iterator iter = m_mapShaders.find(strName);
	return iter->second;
}

GLSLProgram* ResourceMng::GetShader( const string& strName )
{
	if (strName == "")
	{
		map<string, GLSLProgram*>::iterator iter = m_mapShaders.begin();
		return iter->second;
	}
	map<string, GLSLProgram*>::iterator iter = m_mapShaders.find(strName);
	return iter->second;
}

GLSLProgram* ResourceMng::GetShader( int iIndex )
{
	if (iIndex >= m_iShaderCount)
		return NULL;
	int iCount = 0;
	for (map<string, GLSLProgram*>::iterator iter = m_mapShaders.begin(); iter != m_mapShaders.end(); iter++)
	{
		if (iCount  == iIndex)
			return iter->second;
		iCount++;
	}
	return NULL;
}
void ResourceMng::LoadEmptyTexture( GLuint* uiID, const TextureInfo& texInfo )
{
	glGenTextures(1, uiID);
	glBindTexture(GL_TEXTURE_2D, uiID[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, texInfo.m_iInternFormat, texInfo.m_iWidth, texInfo.m_iHeight, 0, GL_RGBA, texInfo.m_eType, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texInfo.m_iFilterMin);//GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texInfo.m_iFilterMag);//GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap generation included in OpenGL v1.4
	glBindTexture(GL_TEXTURE_2D, 0);

}

void ResourceMng::LoadEmptyTextureMultiSample( GLuint* uiID, const TextureInfo& texInfo, const int num_samples )
{
	GL_ERROR();
	glGenTextures(1, uiID);
	GL_ERROR();
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, uiID[0]);
	GL_ERROR();
	GL->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, texInfo.m_iInternFormat, texInfo.m_iWidth, texInfo.m_iHeight, false );
	GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texInfo.m_iFilterMin);//GL_LINEAR
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texInfo.m_iFilterMag);//GL_LINEAR
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	GL_ERROR();

}

void ResourceMng::LoadEmptyDepthTexture( GLuint* uiID, const TextureInfo& texInfo)
{	
	glGenTextures(1, uiID);
	glBindTexture(GL_TEXTURE_2D, uiID[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, texInfo.m_iDepthFormat, texInfo.m_iWidth, texInfo.m_iHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT,NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texInfo.m_iFilterMin);//GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texInfo.m_iFilterMag);//GL_LINEAR
	if (texInfo.m_iCompareRtoTexture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
 	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY); 

 	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
 	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
	// Good shadow map tricks
	//http://www.java-gaming.org/index.php?topic=28018.0
}


void ResourceMng::LoadEmptyDepthTextureMultiSample( GLuint* uiID, const TextureInfo& texInfo, const int num_samples )
{
	GL_ERROR();
	glGenTextures(1, uiID);
	GL_ERROR();
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, uiID[0]);
	GL_ERROR();
	GL->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, /*GL_DEPTH_COMPONENT24*/texInfo.m_iDepthFormat, texInfo.m_iWidth, texInfo.m_iHeight, false );
	
	GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texInfo.m_iFilterMin);//GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texInfo.m_iFilterMag);//GL_LINEAR
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
//// 	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY); 
//
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	if (texInfo.m_bAnisotrpoy)
	{
		float fVal = texInfo.m_fAnisotrpoy > m_fMaxAnisotropy ? m_fMaxAnisotropy : texInfo.m_fAnisotrpoy;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, fVal);
	}
	GL_ERROR();
	// Good shadow map tricks
	//http://www.java-gaming.org/index.php?topic=28018.0
}

 BOOL ResourceMng::CreateFBO( EFBOType eType, const TextureInfo& texInfo, const string& strName, int iColorAttachmentNum /*= 1*/ )
 {
	 PFBO fbo = new FBOObj(GL);
	 fbo->m_eType = eType;
	 fbo->m_iWidth	= texInfo.m_iWidth; 
	 fbo->m_iHeigth	= texInfo.m_iHeight;		
	 fbo->m_iCountColorAttach = iColorAttachmentNum;

	 for (int uiT = 0; uiT < fbo->m_iCountColorAttach; uiT++)
	 {
		 glGenTextures(1, &fbo->m_arrTextures[uiT]);

		 glBindTexture(GL_TEXTURE_2D, fbo->m_arrTextures[uiT]);
		 glTexImage2D(GL_TEXTURE_2D, 0, texInfo.m_iInternFormat, texInfo.m_iWidth, texInfo.m_iHeight, 0, GL_RGBA, texInfo.m_eType, NULL);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texInfo.m_iFilterMin);//GL_LINEAR
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texInfo.m_iFilterMag);//GL_LINEAR
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		 if (texInfo.m_bAnisotrpoy)
		 {
			 float fVal = texInfo.m_fAnisotrpoy > m_fMaxAnisotropy ? m_fMaxAnisotropy : texInfo.m_fAnisotrpoy;
			 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, fVal);
		 }
		 glBindTexture(GL_TEXTURE_2D, 0);
	 }

	 // Framebuffers
	 GL->glGenFramebuffers(1, &fbo->m_uiFBOID);
	 GL->glBindFramebuffer(GL_FRAMEBUFFER, fbo->m_uiFBOID);  

	 // Create a multisampled color attachment texture	 
	 //GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fbo->m_arrTextures[0], 0);
	 GLenum arrCollorAttach[8] = 
	 { 
		 GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		 GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7
	 };
	 for (int uiT = 0; uiT < fbo->m_iCountColorAttach; uiT++)
	 {
		 GL->glFramebufferTexture2D(GL_FRAMEBUFFER, arrCollorAttach[uiT], GL_TEXTURE_2D, fbo->m_arrTextures[uiT], 0);
	 }

	 // Create a renderbuffer object for depth and stencil attachments
	 GL->glGenRenderbuffers(1, &fbo->m_uiRenderBuffer);
	 GL->glBindRenderbuffer(GL_RENDERBUFFER, fbo->m_uiRenderBuffer); 
	 GL->glRenderbufferStorage(GL_RENDERBUFFER, texInfo.m_iDepthFormat, fbo->m_iWidth, fbo->m_iHeigth);
	 //GL->glRenderbufferStorageMultisample(GL_RENDERBUFFER, iSamples, GL_DEPTH_COMPONENT24, fbo->m_iWidth, fbo->m_iHeigth); 
	 GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	 //GL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fbo->m_uiRenderBuffer); 
	 GL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->m_uiRenderBuffer); 

	 if (!CheckFrameBufferStatus())
	 {
		 GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		 GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		 return FALSE;
	 }
	 GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	 PRenderPack pPack = GetDefaultQuadRP();//RenderPack::GetQuad("Quad");
	 if (!CreateVertexBufferArray(pPack))
	 {
		 PRINT_ERROR("FBO Render Pack Fails!");
		 return FALSE;
	 }
	 GL_ERROR();

	 fbo->SetQuadRenderPack(pPack);
	 m_mapFBO.insert(pair<string, PFBO>(strName, fbo));	 
	 return TRUE;
 }

 BOOL ResourceMng::CreateFBOMultiSample( EFBOType eType, const TextureInfo& texInfo, const string& strName, int iSamples, int iColorAttachmentNum /*= 1*/ )
 { 
	 PFBO fbo = new FBOObj(GL);
	 fbo->m_eType = eType;
	 fbo->m_iWidth	= texInfo.m_iWidth; 
	 fbo->m_iHeigth	= texInfo.m_iHeight;		
	 fbo->m_iCountColorAttach = iColorAttachmentNum;

	 for (int uiT = 0; uiT < fbo->m_iCountColorAttach; uiT++)
		{
			glGenTextures(1, &fbo->m_arrTextures[uiT]);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->m_arrTextures[uiT]);
			GL->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, iSamples, texInfo.m_iInternFormat/*GL_RGBA16F*/, fbo->m_iWidth, fbo->m_iHeigth, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texInfo.m_iFilterMin);//GL_LINEAR
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texInfo.m_iFilterMag);//GL_LINEAR
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);			
			if (texInfo.m_bAnisotrpoy)
			{
				float fVal = texInfo.m_fAnisotrpoy > m_fMaxAnisotropy ? m_fMaxAnisotropy : texInfo.m_fAnisotrpoy;
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, fVal);
			}
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		}

	 // Framebuffers
	 GL->glGenFramebuffers(1, &fbo->m_uiFBOID);
	 GL->glBindFramebuffer(GL_FRAMEBUFFER, fbo->m_uiFBOID);  
	 
	 // Create a multisampled color attachment texture	 
	 //GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fbo->m_arrTextures[0], 0);
	 GLenum arrCollorAttach[8] = 
	 { 
		 GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		 GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7
	 };
	 for (int uiT = 0; uiT < fbo->m_iCountColorAttach; uiT++)
	 {
		 GL->glFramebufferTexture2D(GL_FRAMEBUFFER, arrCollorAttach[uiT], GL_TEXTURE_2D_MULTISAMPLE, fbo->m_arrTextures[uiT], 0);
	 }

	 // Create a renderbuffer object for depth and stencil attachments
	 GL->glGenRenderbuffers(1, &fbo->m_uiRenderBuffer);
	 GL->glBindRenderbuffer(GL_RENDERBUFFER, fbo->m_uiRenderBuffer); 
	 GL->glRenderbufferStorageMultisample(GL_RENDERBUFFER, iSamples, texInfo.m_iDepthFormat, fbo->m_iWidth, fbo->m_iHeigth); 
	 GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	 //GL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fbo->m_uiRenderBuffer); 
	 GL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->m_uiRenderBuffer); 

	 if (!CheckFrameBufferStatus())
	 {
		 GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		 return FALSE;
	 }
	 GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);

	 PRenderPack pPack = GetDefaultQuadRP();//RenderPack::GetQuad("Quad");
	 if (!CreateVertexBufferArray(pPack))
	 {
		 PRINT_ERROR("FBO Render Pack Fails!");
		 return FALSE;
	 }
	 GL_ERROR();

	 fbo->SetQuadRenderPack(pPack);
	 m_mapFBO.insert(pair<string, PFBO>(strName, fbo));	 
	 return TRUE;
 }

BOOL ResourceMng::CreateDepthFBO( EFBOType eType, const TextureInfo& texInfo, const string& strName, int iColorAttachmentNum /*= 1*/ )
 {
	 PFBO fbo = new FBOObj(GL);
	 fbo->m_eType = eType; 
	 fbo->m_iWidth	= texInfo.m_iWidth;
	 fbo->m_iHeigth	= texInfo.m_iHeight;		

	 LoadEmptyDepthTexture(&(fbo->m_arrTextures[0]), texInfo);

	 // Create and Bind frame buffer
	 //-------------------------
	 GL->glGenFramebuffers(1, &fbo->m_uiFBOID);
	 GL->glBindFramebuffer(GL_FRAMEBUFFER, fbo->m_uiFBOID);

	 // attach a render buffer to depth attachment point
	 GL->glGenRenderbuffers(1, &fbo->m_uiRenderBuffer);
	 GL->glBindRenderbuffer(GL_RENDERBUFFER, fbo->m_uiRenderBuffer);
	 GL->glRenderbufferStorage(GL_RENDERBUFFER, texInfo.m_iDepthFormat, fbo->m_iWidth, fbo->m_iHeigth); 		

	 GL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->m_uiRenderBuffer);	

	 if (!CheckFrameBufferStatus())
	 {
		 GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		 GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		 return FALSE;
	 }

	 GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo->m_arrTextures[0], 0);	

	 glDrawBuffer(GL_NONE);
	 glReadBuffer(GL_NONE);

	 if (!CheckFrameBufferStatus())
	 {
		 GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		 GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		 return FALSE;
	 }
	 GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	 GL->glBindRenderbuffer(GL_RENDERBUFFER, 0);

	 PRenderPack pPack = GetDefaultQuadRP();//RenderPack::GetQuad("Quad");
	 if (!CreateVertexBufferArray(pPack))
	 {
		 PRINT_ERROR("FBO Render Pack Fails!");
		 return FALSE;
	 }
	 fbo->SetQuadRenderPack(pPack);
	 m_mapFBO.insert(pair<string, PFBO>(strName, fbo));
	 return TRUE;
 }
 
bool ResourceMng::CheckFrameBufferStatus()
{
	GLenum status = GL->glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(status)
	{
	case GL_FRAMEBUFFER_COMPLETE:		
		return true;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		PRINT_ERROR("Framebuffer incomplete: Attachment is NOT complete.");
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		PRINT_ERROR("Framebuffer incomplete: No image is attached to FBO.");
		return false;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		PRINT_ERROR("Framebuffer incomplete: Draw buffer.");
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		PRINT_ERROR("Framebuffer incomplete: Read buffer.");
		return false;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		PRINT_ERROR("Framebuffer incomplete: Unsupported by FBO implementation.");
		return false;

	default:
		PRINT_ERROR("Framebuffer incomplete: Unknown error.");
		return false;
	}
	return false;
}

void ResourceMng::ReleaseFBOs()
{
	for (map<string, PFBO>::iterator iter = m_mapFBO.begin(); iter != m_mapFBO.end(); iter++)
	{
		PFBO fbo = iter->second;	
		fbo->ReleaseAll();	
		delete fbo;
	}
}

const PFBO ResourceMng::GetFBO( const string& strName ) const
{
	if (strName == "")
	{
		return NULL;
	}
	map<string, PFBO>::const_iterator iter = m_mapFBO.find(strName);
	if (iter->second)
		return iter->second;
	return NULL;
}

PFBO ResourceMng::GetFBO( const string& strName )
{
	if (strName == "")
	{
		return NULL;
	}
	map<string, PFBO>::iterator iter = m_mapFBO.find(strName);
	if (iter->second)
		return iter->second;
	return NULL;
}

EnG::PRenderPack ResourceMng::GetDefaultQuadRP()
{
	if (m_pFullQuadRP)
		return m_pFullQuadRP;
	m_pFullQuadRP = RenderPack::GetQuad("Quad");
	if (!CreateVertexBufferArray(m_pFullQuadRP))
	{
	 	PRINT_ERROR("FBO Render Pack Fails!");
	 	return NULL;
	}
	return m_pFullQuadRP;
}

void ResourceMng::BuildOffsetTexture(int texSize, int samplesU, int samplesV )
{
	int size = texSize;
	int samples = samplesU * samplesV;
	int buffSize = size * size * samples * 2;
	float *fdata = new float[buffSize];
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			for (int k = 0; k < samples; k += 2)
			{
				int x1,y1,x2,y2;
				
				x1 = k % (samplesU);
				y1 = (samples - 1 - k) / samplesU;

				x2 = (k + 1) % samplesU;
				y2 = (samples - 1 - k - 1) / samplesU;
				
				Vec4 v;
				// center the grid and jitter
				v.x = (x1 + 0.5f) + jitter();
				v.y = (y1 + 0.5f) + jitter();
				v.z = (x2 + 0.5f) + jitter();
				v.w = (y2 + 0.5f) + jitter();
				// Scale between 0 to 1
				v.x /= samplesU;
				v.y /= samplesV;
				v.z /= samplesU;
				v.w /= samplesV;
				//wrap to disc
				int cell = ((k / 2) * size * size + j * size + i) * 4;
				fdata[cell + 0] = sqrtf(v.y) * cosf(2 * PI * v.x);
				fdata[cell + 1] = sqrtf(v.y) * sinf(2 * PI * v.x);
				fdata[cell + 2] = sqrtf(v.w) * cosf(2 * PI * v.z);
				fdata[cell + 3] = sqrtf(v.w) * sinf(2 * PI * v.z);

			}
		}
	}

	GL->glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_uiOffsetTexture);
	glBindTexture(GL_TEXTURE_3D, m_uiOffsetTexture);
	GL->glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, size ,size, samples / 2, 0, GL_RGBA, GL_FLOAT, fdata );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	delete[] fdata;
}

VOID ResourceMng::AddMaterialToMap( tMaterial* pMaterial, const string& strName )
{
	m_mapMaterials.insert(pair<string, tMaterial*>(strName, pMaterial));		
}

float ResourceMng::jitter()
{
	return ((float)rand() / RAND_MAX) - 0.5f;
}

tMaterial* ResourceMng::GetMaterial( const string& strName )
{
	map<string, tMaterial*>::iterator iterMaterial = m_mapMaterials.find(strName);
	if (iterMaterial == m_mapMaterials.end())
		return NULL;
	return iterMaterial->second;
}

VOID ResourceMng::CopyMaterial( const string& strMaterialName, const string& strObjectName, tMaterial& material )
{
	tMaterial* pMaterial = GetMaterial(strMaterialName);
	if ( pMaterial )
	{
		material.Copy(pMaterial);
		material.m_strName += "_" + strObjectName;
	}
}

BOOL ResourceMng::CreateParticleSystem( CONST GLuint uiSize, float* arrPos, float* arrCol)
{
	const size_t count = uiSize;	
	
	// Allocate an OpenGL vertex array object.
	GL->glGenVertexArrays(1, &m_uipsVertexArrayID);
	// Bind the vertex array object to store all the buffers and vertex attributes we create here.
	GL->glBindVertexArray(m_uipsVertexArrayID);

	// Generate an ID for the vertex buffer.
	GL->glGenBuffers(1, &m_uipsVertexBufferID);
	// Bind the vertex buffer and load the vertex (position, texture, and normal) data into the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, m_uipsVertexBufferID);
	GL->glBufferData(GL_ARRAY_BUFFER, 4 * count * sizeof(float), arrPos, GL_STREAM_DRAW);

	// Enable the three vertex array attributes.
	GL->glEnableVertexAttribArray(0);  // Vertex position.
	GL->glVertexAttribPointer(0, 4, GL_FLOAT, false, (4)*sizeof(float), 0);

	// Generate an ID for the color buffer.
	GL->glGenBuffers(1, &m_uipsColorBufferID);
	// Bind the vertex buffer and load the vertex (position, texture, and normal) data into the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, m_uipsColorBufferID);
	GL->glBufferData(GL_ARRAY_BUFFER, 4 * count * sizeof(float), arrCol, GL_STREAM_DRAW);

	// Enable the three vertex array attributes.
	GL->glEnableVertexAttribArray(1);  // Vertex position.
	GL->glVertexAttribPointer(1, 4, GL_FLOAT, false, (4)*sizeof(float), 0);

	GL->glBindVertexArray(0);
	GL->glBindBuffer(GL_ARRAY_BUFFER, 0);

	GL_ERROR();

	return TRUE;
}

BOOL ResourceMng::ReleaseParticleSystem()
{
	// Release the vertex buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL->glDeleteBuffers(1, &m_uipsVertexBufferID);

	// Release the index buffer.
	GL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL->glDeleteBuffers(1, &m_uipsColorBufferID);

	// Release the vertex array object.
	GL->glBindVertexArray(0);
	GL->glDeleteVertexArrays(1, &m_uipsVertexArrayID);
	
	PRINT_INFO("Particle System VAO was released");	
	GL_ERROR();

	return TRUE;
}

BOOL ResourceMng::UpdateParticleSystem( int iCount, float* arrPos, float* arrCol )
{
	GL->glBindBuffer(GL_ARRAY_BUFFER, m_uipsVertexBufferID);
	GL->glBufferSubData(GL_ARRAY_BUFFER, 0, iCount * sizeof(float) * 4, arrPos);

	GL->glBindBuffer(GL_ARRAY_BUFFER, m_uipsColorBufferID);
	GL->glBufferSubData(GL_ARRAY_BUFFER, 0, iCount * sizeof(float) * 4, arrCol);

	GL->glBindBuffer(GL_ARRAY_BUFFER, 0);

	GL_ERROR();
	return TRUE;
}
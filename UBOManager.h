#ifndef UBO_MNG_H
#define UBO_MNG_H

#include "OGL4.h"
#include <stdio.h>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include "../EnGBasic/EnGBasic.h"
#include "../EnGBasic/CRenderPack.h"
using namespace EnG;


enum E_UBO_TYPE
{
	uboData,
	uboLights,
	uboBlurWeights,
};
enum E_UBODATA_TYPE
{
	valFLoat = 0,
	valVec3 = 3,
	valVec4 = 4
};

struct tUBOData
{
public:
	tUBOData() {}	
	~tUBOData() {}

	GLuint uiVarSize;
	GLchar** varrNames;
};

class CUBO
{
public:
	CUBO		()
	{
		m_uiIndex = 0;
		m_uiBlockIndex=0;
		m_iBlockSize=0;
		m_byBlockBuffer=NULL;
		m_strUniformName = "";
		m_eType = uboData;
		m_eStatType = valVec4;
		m_bCreated = false;
		m_bFilled = false;
	}
	~CUBO		()
	{
		delete m_byBlockBuffer;
	}
	GLuint		m_uiSize;
	GLuint		m_uiIndex;
	GLuint		m_uiBlockIndex;
	GLint		m_iBlockSize;
	GLubyte*	m_byBlockBuffer;
	string		m_strUniformName;
	bool		m_bCreated;				// Is the UBO was created
	bool		m_bFilled;				// Is the UBO was filled with data.
	E_UBO_TYPE	m_eType;				// special case, ubo for all light source in the scene, fill and updates from LightManager
	E_UBODATA_TYPE m_eStatType;
};


class UBOManager
{
public:
	~UBOManager();
	static UBOManager*		GetInstance();
	void					Kill();
	bool					AddUBOtoMap(const string& strName, CUBO ubo);
	CUBO*					GetUBO(const string& strName);
	CUBO*					GetUBO(int iIndex);
	int						GetUBOCount()	const { return m_mapUBO.size();  }
private:
	UBOManager();
	static bool					m_bInstanceFlag;
	static UBOManager*			m_pInstance;

	map<string, CUBO>			m_mapUBO;

};

#endif


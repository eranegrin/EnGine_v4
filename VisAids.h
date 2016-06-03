#ifndef VIS_AIDS_H
#define VIS_AIDS_H
#include "ResourceMng.h"
#include "..\EnGBasic\ParserXML.h"
#include "..\EnGBasic\ParserFBX.h"
#include "..\EnGBasic\BasicObj.h"

using namespace EnG;

#define		VAOBJ_BALL			0
#define		VAOBJ_CAPSULE		1
#define		VAOBJ_PLANE			2
#define		VAOBJ_ARROW			3
#define		VAOBJ_BOX			4

class BasicVisAidObj
{
public:
	BasicVisAidObj();
	~BasicVisAidObj();

	BOOL			Init(PTriMesh pMesh);

	Mat4			m_matLocal;
	Vec3			m_vPos;
	PTriMesh		m_pMesh;
	UINT			m_uiVBOID;
	ResourceMng*	m_pResMng;
	BOOL			m_bReleaseVAO;

};

typedef BasicVisAidObj* PBasicVisAidObj;
class VisAids
{
public:
	static VisAids*		GetInstance();
	void				Kill();
	~VisAids();

	BOOL			LoadVisAids(vector<PTriMesh> arrMeshes);
	INT				AddVisAids(PTriMesh pMesh);

	vector<PBasicVisAidObj>			m_arrAVObjs;
private:
	VisAids();
	static bool			m_bInstanceFlag;
	static VisAids*		m_pInstance;
	
};

#endif
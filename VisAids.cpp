#include "VisAids.h"


bool					VisAids::m_bInstanceFlag = false;
VisAids*				VisAids::m_pInstance= NULL;

VisAids::VisAids()
{
}

VisAids::~VisAids()
{
	UINT nVAObj = m_arrAVObjs.size();
	for (UINT uiO = 0; uiO < nVAObj; uiO++)
	{
		PBasicVisAidObj pVAObj = m_arrAVObjs[uiO];
		delete pVAObj;
	}
}

BOOL VisAids::LoadVisAids(vector<PTriMesh> arrMeshes)
{
	m_arrAVObjs.clear();
	UINT nMeshes = arrMeshes.size();
	for (UINT uiO = 0; uiO < nMeshes; uiO++)
	{
		PTriMesh pMesh = arrMeshes[uiO];
		const string& strName = pMesh->GetNameStr();
		PBasicVisAidObj pVAObj = new BasicVisAidObj();
		pVAObj->Init(pMesh );
		m_arrAVObjs.push_back(pVAObj);
	}


	return TRUE;
}

INT VisAids::AddVisAids( PTriMesh pMesh )
{
	PBasicVisAidObj pVAObj = new BasicVisAidObj();
	pVAObj->Init( pMesh );
	pVAObj->m_bReleaseVAO = FALSE;
	//return -1;
	m_arrAVObjs.push_back(pVAObj);
	return m_arrAVObjs.size() - 1;

}

VisAids* VisAids::GetInstance()
{
	if (m_bInstanceFlag)
		return m_pInstance;
	else
	{
		m_pInstance = new VisAids();
		m_bInstanceFlag = true;
		return m_pInstance;
	}
}

void VisAids::Kill()
{
	if (m_pInstance)
		delete m_pInstance;
}
//////////////////////////////////////////////////////////////////////////
// BasicVisAidObj
//////////////////////////////////////////////////////////////////////////

BasicVisAidObj::BasicVisAidObj()
{
	m_matLocal.identity();
	m_vPos.ZERO();
	m_pMesh = NULL;
	m_bReleaseVAO = TRUE;
}

BasicVisAidObj::~BasicVisAidObj()
{
	if (m_bReleaseVAO)
	{
		if (m_pResMng)
			m_pResMng->ReleaseMeshVAO(m_pMesh);
		delete m_pMesh;
	}
}

BOOL BasicVisAidObj::Init(PTriMesh pMesh )
{
	m_pResMng = ResourceMng::GetInstance();
	if (!m_pResMng || !pMesh)
		return FALSE;
	m_pMesh = pMesh;
	m_pResMng->CreateVertexBufferArray(m_pMesh->GetRenderPack());
	

	
	return TRUE;
}
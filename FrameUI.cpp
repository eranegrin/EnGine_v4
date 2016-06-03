#include "FrameUI.h"



CFrameUI::CFrameUI()
{
}


CFrameUI::~CFrameUI()
{
}

VOID CFrameUI::InitObj(CChunk* a_pRootChunk, OGL4* pGL)
{
	int uiIcons = a_pRootChunk->GetChunckCount();
	for (int uiObj = 0; uiObj < uiIcons; uiObj++)
	{
		CChunk cIcon = a_pRootChunk->GetChunkXMLByID(uiObj);
		// Skip icons
		string strChName = cIcon.GetNode().name();
		if (strChName.find_first_of('_') == 0 || strChName == "Type")
			continue;

		string strType = "";
		cIcon.ReadChildValue("Type", strType);
		PBasicUI pObjUI = AllocateIconType(strType, pGL);
		pObjUI->InitObj(&cIcon);
		m_arrObjectsUI.push_back(pObjUI);
	}
}

VOID CFrameUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	
}

VOID CFrameUI::Render(const float* fProjMat)
{
	int uiIcons = m_arrObjectsUI.size();
	for (int uiObj = 0; uiObj < uiIcons; uiObj++)
	{
		m_arrObjectsUI[uiObj]->Render(fProjMat);
	}
}

CRadioButtonFrameUI::CRadioButtonFrameUI()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

VOID CRadioButtonFrameUI::InitObj(CChunk* a_pRootChunk, OGL4* pGL)
{
	CFrameUI::InitObj(a_pRootChunk, pGL);
}

VOID CRadioButtonFrameUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	int uiIcons = m_arrObjectsUI.size();
	int uiCheckedID = -1;
	for (int uiObj = 0; uiObj < uiIcons; uiObj++)
	{
		if (m_arrObjectsUI[uiObj]->Update(fmx, fmy, bLButtonJustGotUp))
		{
			uiCheckedID = uiObj;
			// check box is checked, reset other check boxes
		}
	}

	if (uiCheckedID > -1)
	{
		for (int uiObj = 0; uiObj < uiIcons; uiObj++)
		{
			if (uiCheckedID != uiObj)
			{
				(static_cast<PCheckBoxUI>(m_arrObjectsUI[uiObj]))->SetState(btnRegular);
			}
		}
	}
}

VOID CRadioButtonFrameUI::Render(const float* fProjMat)
{
	CFrameUI::Render(fProjMat);
}

//////////////////////////////////////////////////////////////////////////


CScrollrameUI::CScrollrameUI()
{

}

VOID CScrollrameUI::InitObj(CChunk* a_pRootChunk, OGL4* pGL)
{
	CFrameUI::InitObj(a_pRootChunk, pGL);
}

VOID CScrollrameUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	int uiIcons = m_arrObjectsUI.size();
	int uiCheckedID = -1;
	for (int uiObj = 0; uiObj < uiIcons; uiObj++)
	{
		if (m_arrObjectsUI[uiObj]->Update(fmx, fmy, bLButtonJustGotUp))
		{
			uiCheckedID = uiObj;			
		}
	}
}

VOID CScrollrameUI::Render(const float* fProjMat)
{
	CFrameUI::Render(fProjMat);
}

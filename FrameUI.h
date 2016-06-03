#pragma once
#include "BasicUI.h"


class CFrameUI
{
public:
	CFrameUI();
	~CFrameUI();

	virtual VOID					InitObj(CChunk* a_pRootChunk, OGL4* pGL);
	virtual VOID					Update(float fmx, float fmy, bool bLButtonJustGotUp);
	virtual VOID					Render(const float* fProjMat);
	
	vector<PBasicUI>		m_arrObjectsUI;
	string					m_strName;

};
typedef CFrameUI* PFrameUI;

class CRadioButtonFrameUI : public CFrameUI
{
public:
	CRadioButtonFrameUI();
	~CRadioButtonFrameUI() {}

	VOID					InitObj(CChunk* a_pRootChunk, OGL4* pGL) override;
	VOID					Update(float fmx, float fmy, bool bLButtonJustGotUp) override;
	VOID					Render(const float* fProjMat) override;

};
typedef CRadioButtonFrameUI* PRadioButtonFrameUI;

class CScrollrameUI : public CFrameUI
{
public:
	CScrollrameUI();
	~CScrollrameUI() {}

	VOID					InitObj(CChunk* a_pRootChunk, OGL4* pGL) override;
	VOID					Update(float fmx, float fmy, bool bLButtonJustGotUp) override;
	VOID					Render(const float* fProjMat) override;

};
typedef CScrollrameUI* PScrollrameUI;


inline CFrameUI* AllocateFrameType(const string& strTypr)
{
	if (strTypr == "Lights")
		return new CRadioButtonFrameUI();
	if (strTypr == "ScrollTest")
		return new CScrollrameUI();
	else
		return new CFrameUI();

}
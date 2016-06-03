#include "UBOManager.h"

UBOManager*			UBOManager::m_pInstance = NULL;
bool				UBOManager::m_bInstanceFlag = false;

UBOManager::UBOManager()
{
}


UBOManager::~UBOManager()
{
}

UBOManager* UBOManager::GetInstance()
{
	if (m_bInstanceFlag)
		return m_pInstance;
	else
	{
		m_pInstance = new UBOManager();
		m_bInstanceFlag = true;
		return m_pInstance;
	}
}

void UBOManager::Kill()
{
	if (m_pInstance)
		delete m_pInstance;
	m_pInstance = NULL;
}

bool UBOManager::AddUBOtoMap( const string& strName, CUBO ubo )
{
	m_mapUBO.insert(std::make_pair(strName, ubo));
	return true;
}

CUBO* UBOManager::GetUBO( const string& strName )
{
	map<string, CUBO>::iterator i = m_mapUBO.find(strName);
	if (i == m_mapUBO.end())
	{		
		return NULL;
	}
	return &((*i).second);
}

CUBO* UBOManager::GetUBO(int iIndex)
{
	if (iIndex >= 0 && iIndex < m_mapUBO.size())
	{
		int i = 0;
		for (map<string, CUBO>::iterator iter = m_mapUBO.begin(); iter != m_mapUBO.end(); iter++)
		{
			if (i == iIndex)
				return &((*iter).second);
			i++;
		}
	}
	return NULL;	
}

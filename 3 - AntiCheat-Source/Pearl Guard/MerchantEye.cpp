#include "MerchantEye.h"

CUIMerchantEye::CUIMerchantEye()
{

}

CUIMerchantEye::~CUIMerchantEye()
{

}

bool CUIMerchantEye::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	string find = xorstr("btn_cancel");
	btn_cancel = (CN3UIButton*)GetChildByID(find);
	/*find = xorstr("btn_buy");
	btn_buy = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_sell");
	btn_sell = (CN3UIButton*)GetChildByID(find);*/
	

	find = xorstr("search_merchant");
	search_merchant = (CN3UIEdit*)GetChildByID(find);
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	lastChar = "";
	return true;
}


bool CUIMerchantEye::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_cancel)
			Close();
		else if (pSender == btn_sell)
		{
			btn_buy->SetState(UI_STATE_BUTTON_NORMAL);
		}
		else if (pSender == btn_buy)
		{
			btn_sell->SetState(UI_STATE_BUTTON_NORMAL);
		}
	}

	return true;
}


void CUIMerchantEye::Tick()
{
	if (!IsVisible() || !search_merchant->HaveFocus())
		return;
	if (search_merchant->GetString().size() > 21)
		return;
	if (Engine->MerchantID.size() > 0 && search_merchant->GetString().empty())
	{

		Engine->MerchantEyeLockData.lock();
		Engine->MerchantID.clear();
		Engine->MerchantEyeLockData.unlock();
		return;

	}


	if (lastChar == search_merchant->GetString())
		return;
	else
		lastChar = search_merchant->GetString();

	if (lastChar.empty())
		return;
	std::map<uint16, uint16> tmpFile;
	std::map<uint16, uint16> tmpDelete;
	Engine->MerchantEyeLockData.lock();
	foreach(itr, Engine->MerchantEyeData)
	{
		MerchantEyeData item = itr->second;
		for (int i = 0; i < 12; i++)
		{
			if (item.strMerchantItem[i].empty())
				continue;

			if (!Engine->str_contains(item.strMerchantItem[i], lastChar))
				continue;

			Engine->MerchantID.insert(std::pair<uint16, uint16>(itr->first, itr->first));
			tmpFile.insert(std::pair<uint16, uint16>(itr->first, itr->first));
			break;
		}
	}

	foreach(itr, Engine->MerchantID)
	{
		auto it = tmpFile.find(itr->first);
		if (it == tmpFile.end())
			tmpDelete.insert(std::pair<uint16, uint16>(itr->first, itr->first));
	}

	foreach(itr, tmpDelete)
		Engine->MerchantID.erase(itr->first);
	Engine->MerchantEyeLockData.unlock();
}

void CUIMerchantEye::Open()
{
	Engine->MerchantEyeLock.lock();
	Engine->MerchantID.clear();
	Engine->MerchantEyeLock.unlock();
	SetVisible(true);
	lastChar = "";
	search_merchant->HaveFocus();
}

void CUIMerchantEye::Close()
{
	search_merchant->SetString("");
	search_merchant->KillFocus();
	SetVisible(false);
	Engine->MerchantEyeLock.lock();
	Engine->MerchantID.clear();
	Engine->MerchantEyeLock.unlock();
}


#include "stdafx.h"
#include "UISeedHelper.h"
extern bool isHideUser;
extern bool isRenderObject;
extern CSpell* GetSkillBase(int iSkillID);
CUISeedHelperPlug::CUISeedHelperPlug()
{
	vector<int>offsets;
	offsets.push_back(0x38C); // 2369
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	m_btnSkillRange = 0;
	m_CameraZoom = 0;
	m_btn_EmblemView = 0;
	m_btnSaveSettings = 0;
	scroll_sight = 0;

	m_btnWeapon = 0;
	m_btnArmor = 0;
	m_btnAccessory = 0;

	m_btnNormal = 0;
	m_btnUpgrade = 0;
	m_btnUnique = 0;
	m_btnRare = 0;
	m_btnCraft = 0;
	m_btnConsumable = 0;
	m_btnMagic = 0;

	m_editPrice = 0;

	m_btnPriceSave = 0;

	m_bNeedToSave = false;
	m_bNeedToSetGame = true;
	m_bNeedToSetGame2 = true;
	m_GroupGame2 = true;
	m_bNeedToSetLoot = true;
	m_bNeedToSetEffect = true;

	m_dGroupOption = 0;
	m_dGroupGame = 0;
	m_dGroupGame2 = 0;
	m_dGroupLoot = 0;

	m_btnDAllSkillFX = 0;
	m_btnDAreaSkillFX = 0;
	m_btnDHealSkillFX = 0;
	m_btnBellowNova = 0;


	ParseUIElements();
	InitReceiveMessage();
}

CUISeedHelperPlug::~CUISeedHelperPlug()
{
}

typedef DWORD(__stdcall* typeJMP)();
typeJMP oCameraZoom;

float camDistance = 0.0f;
float __cameraLimit = 24.0f;

DWORD __declspec(naked) hkCameraZoom() // 2369
{
	__asm {
		// <moviemode kontrolü>
		mov eax, dword ptr ds : [0xF368E0]
		mov al, byte ptr[eax + 0x208]
		movzx eax, al
		test eax, eax
		jne devamke
		// </moviemode kontrolü>
		fld dword ptr[esp + 8]
		fstp dword ptr camDistance
		movss xmm0, __cameraLimit
		comiss xmm0, camDistance
		jb block
		devamke :
		pop ecx
			mov edx, [0x00A7C679]
			mov ebx, 0xAF
			add edx, ebx
			push edx
			ret
			block :
		fld dword ptr __cameraLimit
			fstp dword ptr[ecx + 0x1C8]
			pop ecx
			mov edx, [0x00A7C679]
			mov ebx, 0xD
			add edx, ebx
			push edx
			ret
	}
}

void SetScrollRange(DWORD ui, int min, int max)
{
	DWORD edx = *(DWORD*)(ui + 0x134);
	*(DWORD*)(edx + 0x140) = min;
	*(DWORD*)(edx + 0x13C) = max;
}

void CUISeedHelperPlug::ParseUIElements()
{
	oCameraZoom = (typeJMP)DetourFunction((PBYTE)0x00A7C679, (PBYTE)hkCameraZoom); // 2369

	std::string find = xorstr("group_option");
	Engine->GetChildByID(m_dVTableAddr, find, m_dGroupOption);

	find = xorstr("group_option_game");
	Engine->GetChildByID(m_dGroupOption, find, m_dGroupGame);

	find = xorstr("group_option_game2");
	Engine->GetChildByID(m_dGroupOption, find, m_dGroupGame2);

	find = xorstr("group_option_grapic");
	Engine->GetChildByID(m_dGroupOption, find, m_dGroupgraphic);


	find = xorstr("btn_state_skillrange");
	Engine->GetChildByID(m_dGroupGame2, find, m_btnSkillRange); __ASSERT(m_btnSkillRange, "NULL Pointer");

	find = xorstr("group_option_effect");
	Engine->GetChildByID(m_dGroupOption, find, m_dGroupEffect); __ASSERT(m_dGroupEffect, "NULL Pointer");
	find = xorstr("group_option_grapic2");
	Engine->GetChildByID(m_dGroupOption, find, group_option_grapic2); __ASSERT(group_option_grapic2, "NULL Pointer");

	// Hide Player And Hide Object
	find = xorstr("btn_hide_player");
	Engine->GetChildByID(group_option_grapic2, find, btn_hide_player); __ASSERT(btn_hide_player, "NULL Pointer");
	find = xorstr("btn_render_objects");
	Engine->GetChildByID(group_option_grapic2, find, btn_render_objects); __ASSERT(btn_render_objects, "NULL Pointer");
	// Hide Player And Hide Object

	// fx ------------------------------------------------------------------------------------------------
	find = xorstr("button1");
	Engine->GetChildByID(m_dGroupGame2, find, m_btnDAllSkillFX); __ASSERT(m_btnDAllSkillFX, "NULL Pointer");
	find = xorstr("button2");
	Engine->GetChildByID(m_dGroupGame2, find, m_btnDAreaSkillFX); __ASSERT(m_btnDAreaSkillFX, "NULL Pointer");
	find = xorstr("button3");
	Engine->GetChildByID(m_dGroupGame2, find, m_btnDHealSkillFX); __ASSERT(m_btnDHealSkillFX, "NULL Pointer");

	find = xorstr("btn_bellow_nova");
	Engine->GetChildByID(m_dGroupGame2, find, m_btnBellowNova); __ASSERT(m_btnBellowNova, "NULL Pointer");

	find = xorstr("btn_death_none");
	Engine->GetChildByID(m_dGroupGame2, find, btn_death_none); __ASSERT(btn_death_none, "NULL Pointer");
	find = xorstr("btn_death_me");
	Engine->GetChildByID(m_dGroupGame2, find, btn_death_me); __ASSERT(btn_death_me, "NULL Pointer");
	find = xorstr("btn_death_party");
	Engine->GetChildByID(m_dGroupGame2, find, btn_death_party); __ASSERT(btn_death_party, "NULL Pointer");
	find = xorstr("btn_death_all");
	Engine->GetChildByID(m_dGroupGame2, find, btn_death_all); __ASSERT(btn_death_all, "NULL Pointer");
	// ----------------------------------------------------------------------------------------------------

	find = xorstr("scroll_camera");
	Engine->GetChildByID(m_dGroupgraphic, find, m_CameraZoom); __ASSERT(m_CameraZoom, "NULL Pointer");
	find = xorstr("text_camera_range");
	Engine->GetChildByID(m_dGroupgraphic, find, m_txt_camera_range); __ASSERT(m_txt_camera_range, "NULL Pointer");
	find = xorstr("scroll_sight");
	Engine->GetChildByID(m_dGroupgraphic, find, scroll_sight); __ASSERT(scroll_sight, "NULL Pointer");

	find = xorstr("group_option_Looting");
	Engine->GetChildByID(m_dGroupOption, find, m_dGroupLoot); __ASSERT(m_dGroupLoot, "NULL Pointer");

	find = xorstr("btn_weapon");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnWeapon); __ASSERT(m_btnWeapon, "NULL Pointer");
	find = xorstr("btn_armor");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnArmor); __ASSERT(m_btnArmor, "NULL Pointer");
	find = xorstr("btn_accessory");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnAccessory); __ASSERT(m_btnAccessory, "NULL Pointer");

	find = xorstr("btn_normal");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnNormal); __ASSERT(m_btnNormal, "NULL Pointer");
	find = xorstr("btn_upgrade");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnUpgrade); __ASSERT(m_btnUpgrade, "NULL Pointer");
	find = xorstr("btn_unique");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnUnique); __ASSERT(m_btnUnique, "NULL Pointer");
	find = xorstr("btn_rare");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnRare); __ASSERT(m_btnRare, "NULL Pointer");
	find = xorstr("btn_craft");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnCraft); __ASSERT(m_btnCraft, "NULL Pointer");
	find = xorstr("btn_pet_count_loot");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnConsumable); __ASSERT(m_btnConsumable, "NULL Pointer");
	find = xorstr("btn_magic");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnMagic); __ASSERT(m_btnMagic, "NULL Pointer");

	find = xorstr("edit_pet_loot_money");
	Engine->GetChildByID(m_dGroupLoot, find, m_editPrice); __ASSERT(m_editPrice, "NULL Pointer");

	find = xorstr("btn_pet_loot_money");
	Engine->GetChildByID(m_dGroupLoot, find, m_btnPriceSave); __ASSERT(m_btnPriceSave, "NULL Pointer");

	find = xorstr("btn_save");
	Engine->GetChildByID(m_dGroupOption, find, m_btnSaveSettings); __ASSERT(m_btnSaveSettings, "NULL Pointer");

	if (scroll_sight)
	{
		SetScrollRange(scroll_sight, 0, 500);
	}

	ApplyGameSettings();
	ApplySettings();

	DWORD Adress = (*(DWORD*)0xF368E0) + 0x1C8;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)Adress, &__cameraLimit, sizeof(__cameraLimit), 0);

	CostimizeEffect(Engine->m_SettingsMgr->m_iBellownova);
}


DWORD uiSeedHelperVTable;
DWORD Func_Seedhelper;
void __stdcall UISeedHelperReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiSeedHelperPlug->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiSeedHelperVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Seedhelper
		CALL EAX
	}
}

bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

extern bool skillRange;
bool CUISeedHelperPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiSeedHelperVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	Engine->SetString(m_txt_camera_range, std::to_string(GetCameraRange()));

	if (m_btnSkillRange != NULL && pSender == (DWORD*)m_btnSkillRange)
	{
		m_bNeedToSave = true;
		skillRange = skillRange == 1 ? skillRange = 0 : skillRange = 1;
	}
	else if (m_btnWeapon != NULL && pSender == (DWORD*)m_btnWeapon)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iWeapon = Engine->m_SettingsMgr->m_iWeapon == 1 ? 0 : 1;
	}
	else if (m_btnArmor != NULL && pSender == (DWORD*)m_btnArmor)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iArmor = Engine->m_SettingsMgr->m_iArmor == 1 ? 0 : 1;
	}
	else if (m_btnAccessory != NULL && pSender == (DWORD*)m_btnAccessory)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iAccessory = Engine->m_SettingsMgr->m_iAccessory == 1 ? 0 : 1;
	}
	else if (m_btnNormal != NULL && pSender == (DWORD*)m_btnNormal)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iNormal = Engine->m_SettingsMgr->m_iNormal == 1 ? 0 : 1;
	}
	else if (m_btnUpgrade != NULL && pSender == (DWORD*)m_btnUpgrade)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iUpgrade = Engine->m_SettingsMgr->m_iUpgrade == 1 ? 0 : 1;
	}
	else if (m_btnUnique != NULL && pSender == (DWORD*)m_btnUnique)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iUnique = Engine->m_SettingsMgr->m_iUnique == 1 ? 0 : 1;
	}
	else if (m_btnRare != NULL && pSender == (DWORD*)m_btnRare)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iRare = Engine->m_SettingsMgr->m_iRare == 1 ? 0 : 1;
	}
	else if (m_btnCraft != NULL && pSender == (DWORD*)m_btnCraft)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iCraft = Engine->m_SettingsMgr->m_iCraft == 1 ? 0 : 1;
	}
	else if (m_btnConsumable != NULL && pSender == (DWORD*)m_btnConsumable)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iConsumable = Engine->m_SettingsMgr->m_iConsumable == 1 ? 0 : 1;
	}
	else if (m_btnMagic != NULL && pSender == (DWORD*)m_btnMagic)
	{
		m_bNeedToSave = true;
		Engine->m_SettingsMgr->m_iMagic = Engine->m_SettingsMgr->m_iMagic == 1 ? 0 : 1;
	}
	else if (m_btnDAllSkillFX != NULL && pSender == (DWORD*)m_btnDAllSkillFX)
	{
		m_bNeedToSave = true;
		Engine->bDisableAllSkillFX = !Engine->bDisableAllSkillFX;
	}
	else if (m_btnDAreaSkillFX != NULL && pSender == (DWORD*)m_btnDAreaSkillFX)
	{
		m_bNeedToSave = true;
		Engine->bDisableAreaSkillFX = !Engine->bDisableAreaSkillFX;
	}
	else if (m_btnDHealSkillFX != NULL && pSender == (DWORD*)m_btnDHealSkillFX)
	{
		m_bNeedToSave = true;
		Engine->bDisableHealFX = !Engine->bDisableHealFX;
	}
	// Alttan Nova
	else if (btn_death_all != NULL && pSender == (DWORD*)btn_death_all)
	{
		m_bNeedToSave = true;
		Engine->DeathAll = !Engine->DeathAll;

	}
	else if (btn_death_me != NULL && pSender == (DWORD*)btn_death_me)
	{
		m_bNeedToSave = true;
		Engine->DeathMe = !Engine->DeathMe;

	}
	else if (btn_death_party != NULL && pSender == (DWORD*)btn_death_party)
	{
		m_bNeedToSave = true;
		Engine->DeathParty = !Engine->DeathParty;

	}
	else if (btn_death_none != NULL && pSender == (DWORD*)btn_death_none)
	{
		m_bNeedToSave = true;
		Engine->DeathNone = !Engine->DeathNone;

	}
	else if (btn_render_objects != NULL && pSender == (DWORD*)btn_render_objects)
	{
		m_bNeedToSave = true;
		isRenderObject = !isRenderObject;

	}
	else if (btn_hide_player != NULL && pSender == (DWORD*)btn_hide_player)
	{
		m_bNeedToSave = true;
		Engine->DeathNone = !Engine->DeathNone;
		isHideUser = !isHideUser;
	}
	else if (m_btnPriceSave != NULL && pSender == (DWORD*)m_btnPriceSave)
	{
		std::string m_count = Engine->GetEditString(m_editPrice);

		uint64 a_count = std::stoll(m_count);
		if (a_count >= INT_MAX) a_count = INT_MAX;

		if (is_number(m_count)) {
			if (a_count != Engine->m_SettingsMgr->m_iPrice)
				m_bNeedToSave = true;

			Engine->m_SettingsMgr->m_iPrice = a_count;
		}
	}
	else if (m_btnSaveSettings != NULL && pSender == (DWORD*)m_btnSaveSettings)
	{
		/*int iPrice = Engine->m_SettingsMgr->PetLoootRead();

		if (iPrice != Engine->m_SettingsMgr->m_iPrice) {
			if (iPrice > 999999999) iPrice = 999999999;
			if (iPrice < 0) iPrice = 0;

			m_bNeedToSave = true;
			Engine->m_SettingsMgr->m_iPrice = iPrice;
		}*/
		if (Engine->IsVisible(m_dGroupGame2))
		{
			if (Engine->GetState(m_btnBellowNova) == UI_STATE_BUTTON_DOWN)
			{
				m_bNeedToSave = true;
				Engine->m_SettingsMgr->m_iBellownova = 1;
			}
			else
			{
				m_bNeedToSave = true;
				Engine->m_SettingsMgr->m_iBellownova = 0;

			}
		}

		if (GetCameraRange() != Engine->m_SettingsMgr->m_iCameraRange) {
			Engine->m_SettingsMgr->m_iCameraRange = GetCameraRange();
			__cameraLimit = Engine->m_SettingsMgr->m_iCameraRange + 10.0f;
			DWORD Adress = (*(DWORD*)0xF368E0) + 0x1C8;
			WriteProcessMemory(GetCurrentProcess(), (LPVOID)Adress, &__cameraLimit, sizeof(__cameraLimit), 0);
			m_bNeedToSave = true;
		}

		if (m_bNeedToSave)
		{
			/*if (m_btnPriceSave != NULL)
				UISeedHelperReceiveMessage_Hook((DWORD*)m_btnPriceSave, UIMSG_BUTTON_CLICK);*/

			ApplySettings();
			Engine->m_SettingsMgr->Save();
			m_bNeedToSave = false;
			if (isRenderObject)*(uint8*)0x00476AD7 = 0x74; else*(uint8*)0x00476AD7 = 0x75;
		}
	}

	return true;
}

void CUISeedHelperPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Seedhelper = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UISeedHelperReceiveMessage_Hook;
}

void CUISeedHelperPlug::Tick()
{


	if (m_dGroupLoot == 0 || m_dGroupGame == 0 || m_dGroupEffect == 0 || m_dGroupOption == 0)
		return;

	if (Engine->IsVisible(m_dGroupgraphic))
		Engine->SetString(m_txt_camera_range, std::to_string(GetCameraRange()));

	if (Engine->IsVisible(m_dGroupLoot))
		ApplyLootSettings();
	else
		m_bNeedToSetLoot = true;

	if (Engine->IsVisible(m_dGroupGame) && Engine->IsVisible(m_dGroupOption))
		ApplyGameSettings();
	else
		m_bNeedToSetGame = true;

	if (Engine->IsVisible(m_dGroupEffect))
		ApplyEffectSettings();
	else
		m_bNeedToSetEffect = true;

	if (Engine->IsVisible(m_dGroupGame2))
		ApplyGame2Settings();
	else
		m_bNeedToSetGame2 = true;




	if (Engine->lastkoScreen.x != Engine->curlastkoScreen.x || Engine->curlastkoScreen.y != Engine->lastkoScreen.y)
	{

		POINT koScreen = *(POINT*)0x00DE297C;
		Engine->curlastkoScreen = koScreen;
		RECT region = Engine->GetUiRegion(Engine->uiChatBarPlug->m_dVTableAddr);
		POINT orta;
		Engine->lastkoScreen = koScreen;
		LONG w = (region.right - region.left);
		orta.x = (koScreen.x / 2) - 513;
		orta.y = region.top + 6;
		Engine->SetUIPos(Engine->uiChatBarPlug->m_dVTableAddr, orta);
		orta.x = (koScreen.x / 2) + 205;
		Engine->SetUIPos(Engine->uiInformationWind->m_dVTableAddr, orta);
	}
	DWORD deger = 500;		// F12 de adjust  & Sis kaldýrýldýðý yer

	*(DWORD*)0xDE2B10 = deger;
	*(DWORD*)0x806620 = deger;
	*(DWORD*)0x806614 = deger;
	*(DWORD*)0x806601 = deger;

	if (scroll_sight)
	{
		SetScrollRange(scroll_sight, 0, deger);
	}
}

void CUISeedHelperPlug::CostimizeEffect(bool isEnabled)
{
	//return;		// Alttan nova için pasif hale geldi. Alttan novayý açmak için burayý kaldýracaðýz.

	std::map<uint32, CSpell>::iterator itr;
	if (isEnabled)
	{
		uint32 CostimizeEffect = 0;
		for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++)
		{
			switch (itr->second.iTargetFX)
			{
			case 2912:
				CostimizeEffect = 32335;
				break;
			case 342:
				CostimizeEffect = 32336;
				break;
			case 2902:
				CostimizeEffect = 32337;
				break;
			case 242:
				CostimizeEffect = 32339;
				break;
			case 2911:
				CostimizeEffect = 32340;
				break;
			case 142:
				CostimizeEffect = 32341;
				break;
			case 355:
				CostimizeEffect = 32342;
				break;
			case 2903:
				CostimizeEffect = 32342;
				break;
			case 2901:
				CostimizeEffect = 32343;
				break;
			case 2913:
				CostimizeEffect = 32344;
				break;
			default:
				CostimizeEffect = 0;
				continue;
				break;
			}
			if (CSpell* spell = GetSkillBase(itr->second.dwID))
			{
				spell->iTargetFX = CostimizeEffect;
				CostimizeEffect = 0;
				auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
				if (itcrc != Engine->skillcrc.end()) {
					SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
					itcrc->second = crc;
				}
			}
		}
	}
	else {

		uint32 CostimizeEffect = 0;
		for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++)
		{
			switch (itr->second.iTargetFX)
			{
			case 32335:
				CostimizeEffect = 2912;
				break;
			case 32336:
				CostimizeEffect = 342;
				break;
			case 32337:
				CostimizeEffect = 2902;
				break;
			case 32339:
				CostimizeEffect = 242;
				break;
			case 32340:
				CostimizeEffect = 2911;
				break;
			case 32341:
				CostimizeEffect = 142;
				break;
			case 32342:
				CostimizeEffect = 355;
				CostimizeEffect = 2903;
				break;
			case 32343:
				CostimizeEffect = 2901;
				break;
			case 32344:
				CostimizeEffect = 2913;
				break;
			default:
				CostimizeEffect = 0;
				continue;
				break;
			}
			if (CSpell* spell = GetSkillBase(itr->second.dwID))
			{
				spell->iTargetFX = CostimizeEffect;
				auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
				if (itcrc != Engine->skillcrc.end()) {
					SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
					itcrc->second = crc;
				}
				CostimizeEffect = 0;
			}
		}
	}

}
void CUISeedHelperPlug::ApplyGame2Settings()
{
	if (!m_bNeedToSetGame2)
		return;

	m_bNeedToSetGame2 = false;

	Engine->SetState(m_btnDAllSkillFX, Engine->bDisableAllSkillFX ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnDAreaSkillFX, Engine->bDisableAreaSkillFX ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnDHealSkillFX, Engine->bDisableHealFX ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnBellowNova, Engine->m_SettingsMgr->m_iBellownova ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnSkillRange, skillRange == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(btn_death_all, Engine->DeathAll ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(btn_death_me, Engine->DeathMe ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(btn_death_party, Engine->DeathParty ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(btn_death_none, Engine->DeathNone ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);

	SetCameraRange(Engine->m_SettingsMgr->m_iCameraRange);
	__cameraLimit = Engine->m_SettingsMgr->m_iCameraRange + 10.0f;

}

void CUISeedHelperPlug::ApplyEffectSettings()
{
	if (!m_bNeedToSetEffect)
		return;



	m_bNeedToSetEffect = false;


}

void CUISeedHelperPlug::ApplyGameSettings()
{
	if (!m_bNeedToSetGame)
		return;

	m_bNeedToSetGame = false;
}

void CUISeedHelperPlug::ApplyLootSettings()
{
	if (!m_bNeedToSetLoot)
		return;

	Engine->SetState(m_btnArmor, Engine->m_SettingsMgr->m_iArmor == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnWeapon, Engine->m_SettingsMgr->m_iWeapon == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnAccessory, Engine->m_SettingsMgr->m_iAccessory == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);

	Engine->SetState(m_btnNormal, Engine->m_SettingsMgr->m_iNormal == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnUpgrade, Engine->m_SettingsMgr->m_iUpgrade == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnCraft, Engine->m_SettingsMgr->m_iCraft == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnRare, Engine->m_SettingsMgr->m_iRare == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnMagic, Engine->m_SettingsMgr->m_iMagic == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	Engine->SetState(m_btnUnique, Engine->m_SettingsMgr->m_iUnique == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);

	Engine->SetState(m_btnConsumable, Engine->m_SettingsMgr->m_iConsumable == 1 ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);

	m_bNeedToSetLoot = false;
}

void CUISeedHelperPlug::SendOption()
{
	Packet pkt(XSafe);
	pkt << uint8(LOOT_SETTINS)
		<< uint8(Engine->m_SettingsMgr->m_iWeapon) << uint8(Engine->m_SettingsMgr->m_iArmor)
		<< uint8(Engine->m_SettingsMgr->m_iAccessory) << uint8(Engine->m_SettingsMgr->m_iNormal)
		<< uint8(Engine->m_SettingsMgr->m_iUpgrade) << uint8(Engine->m_SettingsMgr->m_iCraft)
		<< uint8(Engine->m_SettingsMgr->m_iRare) << uint8(Engine->m_SettingsMgr->m_iMagic)
		<< uint8(Engine->m_SettingsMgr->m_iUnique) << uint8(Engine->m_SettingsMgr->m_iConsumable)
		<< uint32(Engine->m_SettingsMgr->m_iPrice);
	Engine->Send(&pkt);
}

void CUISeedHelperPlug::ApplySettings()
{
	__cameraLimit = Engine->m_SettingsMgr->m_iCameraRange + 10.0f;

	ApplySkillFX();
	SendOption();
}

void CUISeedHelperPlug::SetCameraRange(int val)
{
	Engine->SetScrollValue(m_CameraZoom, val);
}

int CUISeedHelperPlug::GetCameraRange()
{
	return Engine->GetScrollValue(m_CameraZoom);
}

const vector<uint32> minorSkill = { 107705 , 108705 , 207705 , 208705 };
const vector<uint32> healSkills = {
	// Karus
	111001,111005,111500,111509,111518,111527,111536,111545,
	111554,111557,111560,112001,112005,112500,112509,112518,112527,112536,112545,
	112554,112557,112560,
	//El Morad
	211001,211005,211500,211509,211518,211527,211536,211545,211554,211557,
	211560,212001,212005,212500,212509,212518,212527,212536,212545,212554,
	212557,212560
};



void CUISeedHelperPlug::ApplySkillFX()
{
	if (Engine->bDisableAllSkillFX) {
		std::map<uint32, CSpell>::iterator itr;
		for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++) {
			itr->second.iSelfFX1 = 0;
			itr->second.iTargetFX = 0;
			if (CSpell* spell = GetSkillBase(itr->second.dwID)) {
				spell->iSelfFX1 = 0;
				spell->iTargetFX = 0;
				auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
				if (itcrc != Engine->skillcrc.end()) {
					SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
					itcrc->second = crc;
				}
			}
		}
		return;
	}
	else {
		std::map<uint32, CSpell>::iterator itr;
		for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++) {
			auto bkp = Engine->skillmapBackup.find(itr->second.dwID);
			itr->second.iSelfFX1 = bkp->second.iSelfFX1;
			itr->second.iTargetFX = bkp->second.iTargetFX;
			if (CSpell* spell = GetSkillBase(itr->second.dwID)) {
				spell->iSelfFX1 = bkp->second.iSelfFX1;
				spell->iTargetFX = bkp->second.iTargetFX;
				auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
				if (itcrc != Engine->skillcrc.end()) {
					SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
					itcrc->second = crc;
				}
			}
		}
	}

	if (Engine->bDisableAreaSkillFX) {
		std::map<uint32, CSpell>::iterator itr;
		for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++) {
			if (itr->second.iMoral == 10 || itr->second.iMoral == 11 || itr->second.iMoral == 12) {
				itr->second.iTargetFX = 0;
				if (CSpell* spell = GetSkillBase(itr->second.dwID)) {
					spell->iTargetFX = 0;
					auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
					if (itcrc != Engine->skillcrc.end()) {
						SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
						itcrc->second = crc;
					}
				}
			}
		}
	}
	else {
		std::map<uint32, CSpell>::iterator itr;
		for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++) {
			if (itr->second.iMoral == 10 || itr->second.iMoral == 11 || itr->second.iMoral == 12) {
				auto bkp = Engine->skillmapBackup.find(itr->second.dwID);
				itr->second.iTargetFX = bkp->second.iTargetFX;
				if (CSpell* spell = GetSkillBase(itr->second.dwID)) {
					spell->iTargetFX = bkp->second.iTargetFX;
					auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
					if (itcrc != Engine->skillcrc.end()) {
						SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
						itcrc->second = crc;
					}
				}
			}
		}
	}

	if (Engine->bDisableHealFX) {
		for (uint32 skillID : minorSkill) {
			auto itr = Engine->skillmap.find(skillID);
			itr->second.iSelfFX1 = 0;
			itr->second.iTargetFX = 0;
			if (CSpell* spell = GetSkillBase(skillID)) {
				spell->iSelfFX1 = 0;
				spell->iTargetFX = 0;
				auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
				if (itcrc != Engine->skillcrc.end()) {
					SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
					itcrc->second = crc;
				}
			}
		}
		for (uint32 skillID : healSkills) {
			auto itr = Engine->skillmap.find(skillID);
			itr->second.iTargetFX = 0;
			if (CSpell* spell = GetSkillBase(skillID)) {
				spell->iTargetFX = 0;
				auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
				if (itcrc != Engine->skillcrc.end())
				{
					SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
					itcrc->second = crc;
				}
			}
		}
	}
	else {
		for (uint32 skillID : minorSkill) {
			auto itr = Engine->skillmap.find(skillID);
			auto bkp = Engine->skillmapBackup.find(skillID);
			if (itr != Engine->skillmap.end() || bkp != Engine->skillmapBackup.end()) {
				itr->second.iSelfFX1 = bkp->second.iSelfFX1;
				itr->second.iTargetFX = bkp->second.iTargetFX;
				if (CSpell* spell = GetSkillBase(skillID)) {
					spell->iSelfFX1 = bkp->second.iSelfFX1;
					spell->iTargetFX = bkp->second.iTargetFX;
					auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
					if (itcrc != Engine->skillcrc.end())
					{
						SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
						itcrc->second = crc;
					}
				}
			}
		}
		for (uint32 skillID : healSkills) {
			auto itr = Engine->skillmap.find(skillID);
			auto bkp = Engine->skillmapBackup.find(skillID);
			if (itr != Engine->skillmap.end() || bkp != Engine->skillmapBackup.end()) {
				itr->second.iTargetFX = bkp->second.iTargetFX;
				if (CSpell* spell = GetSkillBase(skillID)) {
					spell->iTargetFX = bkp->second.iTargetFX;
					auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
					if (itcrc != Engine->skillcrc.end()) {
						SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
						itcrc->second = crc;
					}
				}
			}
		}
	}
	CostimizeEffect(Engine->m_SettingsMgr->m_iBellownova);
}
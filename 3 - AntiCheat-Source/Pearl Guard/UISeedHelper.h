#pragma once
class CUISeedHelperPlug;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUISeedHelperPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD m_dGroupOption;
	DWORD m_dGroupGame;
	DWORD m_dGroupGame2;
	DWORD m_dGroupgraphic;
	DWORD m_dGroupEffect;
	DWORD m_dGroupLoot;

	DWORD scroll_sight;
	DWORD m_CameraZoom;
	DWORD m_btnSkillRange;
	DWORD m_btn_EmblemView;
	DWORD m_btn_PathosView;
	DWORD m_txt_camera_range;

	DWORD m_btnSaveSettings;

	DWORD m_btnWeapon;
	DWORD btn_render_objects;
	DWORD group_option_grapic2;
	DWORD btn_hide_player;
	DWORD m_btnArmor;
	DWORD m_btnAccessory;

	DWORD m_btnNormal;
	DWORD m_btnUpgrade;
	DWORD m_btnUnique;
	DWORD m_btnRare;
	DWORD m_btnCraft;
	DWORD m_btnConsumable;
	DWORD m_btnMagic;

	DWORD m_editPrice;

	DWORD m_btnPriceSave;

	DWORD m_btnDAllSkillFX;
	DWORD m_btnDAreaSkillFX;
	DWORD m_btnDHealSkillFX;
	DWORD m_btnBellowNova;

	DWORD btn_death_none;
	DWORD btn_death_me;
	DWORD btn_death_party;
	DWORD btn_death_all;
	bool m_bNeedToSave;
	bool m_bNeedToSetLoot;
	bool m_bNeedToSetGame, m_bNeedToSetGame2;
	bool m_bNeedToSetEffect;
	bool m_GroupGame2;
public:
	CUISeedHelperPlug();
	~CUISeedHelperPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void Tick();
	void ApplyLootSettings();
	void ApplyGameSettings();
	void ApplyGame2Settings();
	void CostimizeEffect(bool isEnabled);
	void ApplyEffectSettings();
	void ApplySettings();
	void SendOption();
	void SetCameraRange(int val);
	int GetCameraRange();
	void ApplySkillFX();
private:

};
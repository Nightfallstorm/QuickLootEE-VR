#include "Settings.h"

void Settings::LoadSettings()
{
	auto& settings = GetSingleton();
	LoadGlobal(settings.m_close_in_combat, "QLEECloseInCombat");
	LoadGlobal(settings.m_close_when_empty, "QLEECloseWhenEmpty");
	LoadGlobal(settings.m_dispel_invis, "QLEEDispelInvisibility");
	LoadGlobal(settings.m_open_when_container_unlocked, "QLEEOpenWhenContainerUnlocked");
	LoadGlobal(settings.m_show_book_read, "QLEEIconShowBookRead");
	LoadGlobal(settings.m_show_enchanted, "QLEEIconShowEnchanted");
	LoadGlobal(settings.m_show_dbm_displayed, "QLEEIconShowDBMDisplayed");
	LoadGlobal(settings.m_show_dbm_found, "QLEEIconShowDBMFound");
	LoadGlobal(settings.m_show_dbm_new, "QLEEIconShowDBMNew");
	LoadGlobal(settings.m_disable_for_animals, "QLEEDisableForAnimals");
	LoadGlobal(settings.m_vrhand, "QLEEVRHand");
	LoadGlobal(settings.m_vrscale, "QLEEVRScale");
	LoadGlobal(settings.m_translate_X, "QLEEVRTranslateX");
	LoadGlobal(settings.m_translate_Y, "QLEEVRTranslateY");
	LoadGlobal(settings.m_translate_Z, "QLEEVRTranslateZ");
	LoadGlobal(settings.m_rotate_X, "QLEEVRRotateX");
	LoadGlobal(settings.m_rotate_Y, "QLEEVRRotateY");
	LoadGlobal(settings.m_rotate_Z, "QLEEVRRotateZ");
	LoadGlobal(settings.m_disableForCorpse, "QLEEDisableForCorpse");
}

bool Settings::CloseInCombat()
{
	auto& settings = GetSingleton();
	return settings.m_close_in_combat && settings.m_close_in_combat->value > 0;
}

bool Settings::CloseWhenEmpty()
{
	auto& settings = GetSingleton();
	return settings.m_close_when_empty && settings.m_close_when_empty->value > 0;
}

bool Settings::DispelInvisibility()
{
	auto& settings = GetSingleton();
	return settings.m_dispel_invis && settings.m_dispel_invis->value > 0;
}

bool Settings::OpenWhenContainerUnlocked()
{
	auto& settings = GetSingleton();
	return settings.m_open_when_container_unlocked && settings.m_open_when_container_unlocked->value > 0;
}

bool Settings::DisableForAnimals()
{
	auto& settings = GetSingleton();
	return settings.m_disable_for_animals && settings.m_disable_for_animals->value > 0;
}

bool Settings::DisableForCorpse()
{
	auto& settings = GetSingleton();
	return settings.m_disableForCorpse && settings.m_disableForCorpse->value > 0;
}

bool Settings::ShowBookRead()
{
	auto& settings = GetSingleton();
	return settings.m_show_book_read && settings.m_show_book_read->value > 0;
}

bool Settings::ShowEnchanted()
{
	auto& settings = GetSingleton();
	return settings.m_show_enchanted && settings.m_show_enchanted->value > 0;
}

bool Settings::ShowDBMDisplayed()
{
	auto& settings = GetSingleton();
	return settings.m_show_dbm_displayed && settings.m_show_dbm_displayed->value > 0;
}

bool Settings::ShowDBMFound()
{
	auto& settings = GetSingleton();
	return settings.m_show_dbm_found && settings.m_show_dbm_found->value > 0;
}

bool Settings::ShowDBMNew()
{
	auto& settings = GetSingleton();
	return settings.m_show_dbm_new && settings.m_show_dbm_new->value > 0;
}

float Settings::VRScale()
{
	auto& settings = GetSingleton();
	return settings.m_vrscale ? settings.m_vrscale->value : 0.f;
}

float Settings::VRTranslateX()
{
	auto& settings = GetSingleton();
	return settings.m_translate_X ? settings.m_translate_X->value : 0.f;
}

float Settings::VRTranslateY()
{
	auto& settings = GetSingleton();
	return settings.m_translate_Y ? settings.m_translate_Y->value : 0.f;
}

float Settings::VRTranslateZ()
{
	auto& settings = GetSingleton();
	return settings.m_translate_Z ? settings.m_translate_Z->value : 0.f;
}

float Settings::VRRotateX()
{
	auto& settings = GetSingleton();
	return settings.m_rotate_X ? settings.m_rotate_X->value : 0.f;
}

float Settings::VRRotateY()
{
	auto& settings = GetSingleton();
	return settings.m_rotate_Y ? settings.m_rotate_Y->value : 0.f;
}

float Settings::VRRotateZ()
{
	auto& settings = GetSingleton();
	return settings.m_rotate_Z ? settings.m_rotate_Z->value : 0.f;
}

void Settings::LoadGlobal(const RE::TESGlobal*& global, const char* editor_id)
{
	global = RE::TESForm::LookupByEditorID<RE::TESGlobal>(editor_id);
}

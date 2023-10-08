#include "LOTD.h"

void LOTD::LoadLists()
{
	constexpr char TCC_PLUGIN_NAME[] = "DBM_RelicNotifications.esp";

	auto TESDataHandler = RE::TESDataHandler::GetSingleton(); 

	if (!TESDataHandler) {
		return;
	}

	if (TESDataHandler->LookupLoadedModByName(TCC_PLUGIN_NAME) == nullptr) {
		return;
	}

	auto& lotd = GetSingleton();

	// TODO: MergeMapper support on this
	lotd.m_dbm_new = TESDataHandler->LookupForm<RE::BGSListForm>(0x558285, TCC_PLUGIN_NAME);
	lotd.m_dbm_found = TESDataHandler->LookupForm<RE::BGSListForm>(0x558286, TCC_PLUGIN_NAME);
	lotd.m_dbm_displayed = TESDataHandler->LookupForm<RE::BGSListForm>(0x558287, TCC_PLUGIN_NAME);
}

bool LOTD::IsItemNew(RE::FormID id)
{
	auto& lotd = GetSingleton();
	return lotd.m_dbm_new && lotd.m_dbm_new->HasForm(id);
}

bool LOTD::IsItemFound(RE::FormID id)
{
	auto& lotd = GetSingleton();
	return lotd.m_dbm_found && lotd.m_dbm_found->HasForm(id);
}

bool LOTD::IsItemDisplayed(RE::FormID id)
{
	auto& lotd = GetSingleton();
	return lotd.m_dbm_displayed && lotd.m_dbm_displayed->HasForm(id);
}


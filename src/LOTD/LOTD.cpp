#include "LOTD.h"
#include "MergeMapperPluginAPI.h"

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

	std::pair<const char*, RE::FormID> dbm_new = { TCC_PLUGIN_NAME, 0x558285 };
	std::pair<const char*, RE::FormID> dbm_found = { TCC_PLUGIN_NAME, 0x558286 };
	std::pair<const char*, RE::FormID> dbm_displayed = { TCC_PLUGIN_NAME, 0x558287 };

	if (g_mergeMapperInterface) {
		dbm_new = g_mergeMapperInterface->GetNewFormID(dbm_new.first, dbm_new.second);
		dbm_found = g_mergeMapperInterface->GetNewFormID(dbm_found.first, dbm_found.second);
		dbm_displayed = g_mergeMapperInterface->GetNewFormID(dbm_displayed.first, dbm_displayed.second);
	}

	lotd.m_dbm_new = TESDataHandler->LookupForm<RE::BGSListForm>(dbm_new.second, dbm_new.first);
	lotd.m_dbm_found = TESDataHandler->LookupForm<RE::BGSListForm>(dbm_found.second, dbm_found.first);
	lotd.m_dbm_displayed = TESDataHandler->LookupForm<RE::BGSListForm>(dbm_displayed.second, dbm_displayed.first);
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


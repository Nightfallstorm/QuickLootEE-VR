#include "Loot.h"

#include "HUDManager.h"
#include "Scaleform/LootMenu.h"

void Loot::Close()
{
	if (IsOpen()) {
		auto msgQ = RE::UIMessageQueue::GetSingleton();
		if (msgQ) {
			msgQ->AddMessage(LootMenu::MenuName(), RE::UI_MESSAGE_TYPE::kHide, nullptr);
			auto& hud = HUDManager::GetSingleton();
			hud.Disable();
		}
	}
}

void Loot::Open()
{
	if (ShouldOpen()) {
		auto msgQ = RE::UIMessageQueue::GetSingleton();
		if (msgQ) {
			msgQ->AddMessage(LootMenu::MenuName(), RE::UI_MESSAGE_TYPE::kShow, nullptr);
			auto& hud = HUDManager::GetSingleton();
			hud.Enable();
		}
	}
}

bool Loot::IsShowing() {
	return _showing;
}

void Loot::SetShowing(bool isShowing) {
	_showing = isShowing;
}

void Loot::ModSelectedIndex(double a_mod)
{
	AddTask([a_mod](LootMenu& a_menu) {
		a_menu.ModSelectedIndex(a_mod);
	});
}

void Loot::ModSelectedPage(double a_mod)
{
	AddTask([a_mod](LootMenu& a_menu) {
		a_menu.ModSelectedPage(a_mod);
	});
}

void Loot::SetContainer(RE::ObjectRefHandle a_container)
{
	AddTask([a_container](LootMenu& a_menu) {
		a_menu.SetContainer(a_container);
	});
}

void Loot::TakeStack()
{
	AddTask([](LootMenu& a_menu) {
		a_menu.TakeStack();
	});
}

void Loot::Process(LootMenu& a_menu)
{
	if (!_taskQueue.empty()) {
		for (auto& task : _taskQueue) {
			task(a_menu);
		}
		_taskQueue.clear();
	}

	if (_refreshUI) {
		a_menu.RefreshUI();
	} else if (_refreshInventory) {
		a_menu.RefreshInventory();
	}

	_refreshUI = false;
	_refreshInventory = false;
}

void Loot::AddTask(Tasklet a_task)
{
	Open();
	std::scoped_lock l{ _lock };
	_taskQueue.push_back(std::move(a_task));
}

// VR notes: Due to temporary workarounds, the loot menu is always open, but hidden when not actively looking at a container
// Therefore, we now use a simple boolean to dictate if "IsOpen"
bool Loot::IsOpen() const
{
	return _showing;
}

auto Loot::GetMenu() const
	-> RE::GPtr<LootMenu>
{
	auto ui = RE::UI::GetSingleton();
	return ui ? ui->GetMenu<LootMenu>(LootMenu::MenuName()) : nullptr;
}

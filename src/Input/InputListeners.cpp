#include "Input/InputListeners.h"

#include "Input.h"
#include "Loot.h"

namespace Input
{
	ScrollHandler::ScrollHandler()
	{
		using Device = RE::INPUT_DEVICE;
		using Gamepad = RE::BSWin32GamepadDevice::Key;
		using Keyboard = RE::BSWin32KeyboardDevice::Key;
		using Mouse = RE::BSWin32MouseDevice::Key;

		const auto& groups = ControlGroups::get();

		if (groups[Group::kPageKeys]) {
			auto& mappings = _mappings[Device::kKeyboard];
			mappings.emplace(Keyboard::kPageUp, [] { Loot::GetSingleton().ModSelectedPage(-1.0); });
			mappings.emplace(Keyboard::kPageDown, [] { Loot::GetSingleton().ModSelectedPage(1.0); });
		}

		if (groups[Group::kArrowKeys]) {
			auto& mappings = _mappings[Device::kKeyboard];
			mappings.emplace(Keyboard::kUp, [] { Loot::GetSingleton().ModSelectedIndex(-1.0); });
			mappings.emplace(Keyboard::kDown, [] { Loot::GetSingleton().ModSelectedIndex(1.0); });
			mappings.emplace(Keyboard::kLeft, [] { Loot::GetSingleton().ModSelectedPage(-1.0); });
			mappings.emplace(Keyboard::kRight, [] { Loot::GetSingleton().ModSelectedPage(1.0); });
		}

		if (groups[Group::kMouseWheel]) {
			auto& mappings = _mappings[Device::kMouse];
			mappings.emplace(Mouse::kWheelUp, [] { Loot::GetSingleton().ModSelectedIndex(-1.0); });
			mappings.emplace(Mouse::kWheelDown, [] { Loot::GetSingleton().ModSelectedIndex(1.0); });
		}

		if (groups[Group::kDPAD]) {
			auto& mappings = _mappings[Device::kGamepad];
			mappings.emplace(Gamepad::kUp, [] { Loot::GetSingleton().ModSelectedIndex(-1.0); });
			mappings.emplace(Gamepad::kDown, [] { Loot::GetSingleton().ModSelectedIndex(1.0); });
			mappings.emplace(Gamepad::kLeft, [] { Loot::GetSingleton().ModSelectedPage(-1.0); });
			mappings.emplace(Gamepad::kRight, [] { Loot::GetSingleton().ModSelectedPage(1.0); });
		}
	}

	void TakeHandler::TakeStack()
	{
		auto& loot = Loot::GetSingleton();
		loot.TakeStack();
	}

	void TakeHandler::TryGrab()
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		player->StartGrabObject(RE::VR_DEVICE::kHeadset); // TODO: Take a look at this
		if (!player->IsGrabbing()) {
			return;
		}

		auto playerControls = RE::PlayerControls::GetSingleton();
		auto activateHandler = playerControls ? playerControls->GetActivateHandler() : nullptr;
		if (activateHandler) {
			activateHandler->SetHeldButtonActionSuccess(true);
		}

		auto& loot = Loot::GetSingleton();
		loot.Close();
	}

	void TransferHandler::DoHandle(RE::InputEvent* const& a_event)
	{
		for (auto iter = a_event; iter; iter = iter->next) {
			auto event = iter->AsButtonEvent();
			if (!event) {
				continue;
			}

			auto controlMap = RE::ControlMap::GetSingleton();
			const auto idCode =
				controlMap ?
                    controlMap->GetMappedKey("Ready Weapon"sv, event->GetDevice()) :
                    RE::ControlMap::kInvalid;

			if (event->GetIDCode() == idCode && event->IsDown()) {
				auto player = RE::PlayerCharacter::GetSingleton();
				if (player) {
					player->ActivatePickRef();
				}

				auto& loot = Loot::GetSingleton();
				loot.Close();
				return;
			}
		}
	}
}

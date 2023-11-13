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
		using VR = RE::BSOpenVRControllerDevice::Key;
	}

	void TakeHandler::TakeStack()
	{
		auto& loot = Loot::GetSingleton();
		loot.TakeStack();
	}

	void TransferHandler::DoHandle(RE::InputEvent* const& a_event)
	{
		if (true)
			return;
		using VR = RE::BSOpenVRControllerDevice::Key;
		for (auto iter = a_event; iter; iter = iter->next) {
			auto event = iter->AsButtonEvent();
			if (!event) {
				continue;
			}

			auto controlMap = RE::ControlMap::GetSingleton();
			const auto idCode =
				controlMap ?
					controlMap->GetMappedKey("Activate", event->GetDevice()) :
					RE::ControlMap::kInvalid;

			if (event->GetIDCode() == idCode && event->IsDown()) {
				if (event->IsHeld() && event->HeldDuration() > 1.0f) {

					auto player = RE::PlayerCharacter::GetSingleton();
					auto hand = player->isRightHandMainHand ? RE::VR_DEVICE::kRightController : RE::VR_DEVICE::kLeftController;
					if (player) {
						
						player->ActivatePickRef(hand);
					}

					auto& loot = Loot::GetSingleton();
					loot.Close();
					return;
				}
			}
		}
	}
}

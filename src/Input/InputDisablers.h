#pragma once

#include "Input/Input.h"

namespace Input
{
	class Disablers
	{
	public:
		Disablers() = default;
		Disablers(const Disablers&) = default;
		Disablers(Disablers&&) = default;

		~Disablers() { Disable(); }

		Disablers& operator=(const Disablers&) = default;
		Disablers& operator=(Disablers&&) = default;

		void Enable()
		{
			auto controlMap = RE::ControlMap::GetSingleton();
			if (controlMap) {
				controlMap->ToggleControls(QUICKLOOT_FLAG, false);
				// TODO: Could we only block the thumbstick event itself and not the sneak and other stuff?
				if (const auto pcControls = RE::PlayerControls::GetSingleton()) {
					pcControls->readyWeaponHandler->SetInputEventHandlingEnabled(false);
					pcControls->sneakHandler->SetInputEventHandlingEnabled(false);
					pcControls->autoMoveHandler->SetInputEventHandlingEnabled(false);
					pcControls->shoutHandler->SetInputEventHandlingEnabled(false);
					pcControls->attackBlockHandler->SetInputEventHandlingEnabled(false);
					pcControls->jumpHandler->SetInputEventHandlingEnabled(false);
				}
			}
		}

		void Disable()
		{
			auto controlMap = RE::ControlMap::GetSingleton();
			if (controlMap) {
				controlMap->ToggleControls(QUICKLOOT_FLAG, true);
				if (const auto pcControls = RE::PlayerControls::GetSingleton()) {
					pcControls->readyWeaponHandler->SetInputEventHandlingEnabled(true);
					pcControls->sneakHandler->SetInputEventHandlingEnabled(true);
					pcControls->autoMoveHandler->SetInputEventHandlingEnabled(true);
					pcControls->shoutHandler->SetInputEventHandlingEnabled(true);
					pcControls->attackBlockHandler->SetInputEventHandlingEnabled(true);
					pcControls->jumpHandler->SetInputEventHandlingEnabled(true);
				}
			}
		}
	};
}

#pragma once

#include "Loot.h"

namespace Input
{
	class IHandler
	{
	public:
		virtual ~IHandler() = default;

		void operator()(RE::InputEvent* const& a_event) { DoHandle(a_event); }

	protected:
		virtual void DoHandle(RE::InputEvent* const& a_event) = 0;
	};

	class ScrollHandler :
		public IHandler
	{
	public:
		ScrollHandler();

	protected:
		void DoHandle(RE::InputEvent* const& a_event) override
		{
			using Device = RE::INPUT_DEVICE;

			for (auto iter = a_event; iter; iter = iter->next) {
				auto event = iter->AsIDEvent();
				if (!event || event->GetEventType() != RE::INPUT_EVENT_TYPE::kThumbstick) {
					continue;
				}
				auto thumbStickEvent = static_cast<RE::ThumbstickEvent*>(event);

				if (!thumbStickEvent->IsMainHand()) {
					continue;
				}

				if (!hasScrolled && std::abs(thumbStickEvent->yValue) > 0.9) {
					hasScrolled = true;
					if (thumbStickEvent->yValue > 0) {
						Loot::GetSingleton().ModSelectedIndex(-1.0);
					} else {
						Loot::GetSingleton().ModSelectedIndex(1.0);
					}

				} else if (hasScrolled && std::abs(thumbStickEvent->yValue) < 0.1) {
					hasScrolled = false;
				}
			}
		}

	private:
		bool hasScrolled = false;
	};

	class TakeHandler :
		public IHandler
	{
	protected:
		void DoHandle(RE::InputEvent* const& a_event) override
		{
			using VR = RE::BSOpenVRControllerDevice::Key;
			for (auto iter = a_event; iter; iter = iter->next) {
				const auto event = iter->AsButtonEvent();
				if (!event) {
					continue;
				}

				const auto controlMap = RE::ControlMap::GetSingleton();
				const auto idCode =
					controlMap ?
						controlMap->GetMappedKey("Activate", event->GetDevice()) :
						RE::ControlMap::kInvalid;

				const auto openIdCode =
					controlMap ?
						controlMap->GetMappedKey("Ready Weapon", event->GetDevice()) :
						RE::ControlMap::kInvalid;

				if (event->GetIDCode() == openIdCode) {
					if (event->IsUp()) {
						auto player = RE::PlayerCharacter::GetSingleton();
						if (!player) {
							return;
						}
						auto hand = player->isRightHandMainHand ? RE::VR_DEVICE::kRightController : RE::VR_DEVICE::kLeftController;
						player->ActivatePickRef(hand);

						auto& loot = Loot::GetSingleton();
						loot.Close();
						return;
					}
				}

				if (event->GetIDCode() == idCode) {
					if (!_context && !event->IsDown()) {
						continue;
					}
					_context = true;

					if (event->IsUp()) {
						TakeStack();
						_context = false;
						return;
					}
				}
			}
		}

	private:
		float GetGrabDelay() const
		{
			if (_grabDelay) {
				return _grabDelay->GetFloat();
			} else {
				assert(false);
				return std::numeric_limits<float>::max();
			}
		}

		void TakeStack();

		stl::observer<const RE::Setting*> _grabDelay{ RE::GetINISetting("fZKeyDelay:Controls") };
		bool _context{ false };
	};

	class TransferHandler :
		public IHandler
	{
	protected:
		void DoHandle(RE::InputEvent* const& a_event) override;
	};

	class Listeners :
		public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		Listeners()
		{
			_callbacks.push_back(std::make_unique<TakeHandler>());
			_callbacks.push_back(std::make_unique<ScrollHandler>());
			_callbacks.push_back(std::make_unique<TransferHandler>());
		}

		Listeners(const Listeners&) = default;
		Listeners(Listeners&&) = default;

		~Listeners() { Disable(); }

		Listeners& operator=(const Listeners&) = default;
		Listeners& operator=(Listeners&&) = default;

		void Enable()
		{
			auto input = RE::BSInputDeviceManager::GetSingleton();
			if (input) {
				input->AddEventSink(this);
			}
		}

		void Disable()
		{
			auto input = RE::BSInputDeviceManager::GetSingleton();
			if (input) {
				input->RemoveEventSink(this);
			}
		}

	private:
		using EventResult = RE::BSEventNotifyControl;

		EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override
		{
			if (!Loot::GetSingleton().IsShowing()) {
				return EventResult::kContinue;
			}

			if (a_event) {
				for (auto& callback : _callbacks) {
					(*callback)(*a_event);
				}
			}

			return EventResult::kContinue;
		}

		std::vector<std::unique_ptr<IHandler>> _callbacks{};
	};
}

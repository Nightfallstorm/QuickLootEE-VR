#include "Animation/Animation.h"
#include "Events/Events.h"
#include "Hooks.h"
#include "Input/Input.h"
#include "Items/GFxItem.h"
#include "LOTD/LOTD.h"
#include "Loot.h"
#include "MergeMapperPluginAPI.h"
#include "Scaleform/Scaleform.h"

namespace
{
	class InputHandler :
		public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		static InputHandler* GetSingleton()
		{
			static InputHandler singleton;
			return std::addressof(singleton);
		}

		static void Register()
		{
			auto input = RE::BSInputDeviceManager::GetSingleton();
			input->AddEventSink(GetSingleton());
			logger::info("Registered InputHandler"sv);
		}

	protected:
		using EventResult = RE::BSEventNotifyControl;

		EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override
		{
			using InputType = RE::INPUT_EVENT_TYPE;
			using Keyboard = RE::BSWin32KeyboardDevice::Key;

			if (!a_event) {
				return EventResult::kContinue;
			}

			auto intfcStr = RE::InterfaceStrings::GetSingleton();
			auto ui = RE::UI::GetSingleton();
			if (ui->IsMenuOpen(intfcStr->console)) {
				return EventResult::kContinue;
			}

			for (auto event = *a_event; event; event = event->next) {
				if (event->eventType != InputType::kButton) {
					continue;
				}

				auto button = static_cast<RE::ButtonEvent*>(event);
				if (!button->IsDown() || button->device != RE::INPUT_DEVICE::kKeyboard) {
					continue;
				}

				auto& loot = Loot::GetSingleton();
				switch (button->idCode) {
				case Keyboard::kNum0:
					loot.Enable();
					break;
				case Keyboard::kNum9:
					loot.Disable();
					break;
				default:
					break;
				}
			}

			return EventResult::kContinue;
		}

	private:
		InputHandler() = default;
		InputHandler(const InputHandler&) = delete;
		InputHandler(InputHandler&&) = delete;

		~InputHandler() = default;

		InputHandler& operator=(const InputHandler&) = delete;
		InputHandler& operator=(InputHandler&&) = delete;
	};

	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kDataLoaded:
#ifndef NDEBUG
			InputHandler::Register();
#endif

			Animation::AnimationManager::Install();

			Events::Register();
			Scaleform::Register();

			Settings::LoadSettings();
			LOTD::LoadLists();
			Events::VRCrosshairRefSource::GetSingleton()->QueueEvalute();
			break;
		case SKSE::MessagingInterface::kPostPostLoad:
			{
				Completionist_Integration::RegisterListener();
				MergeMapperPluginAPI::GetMergeMapperInterface001();  // Request interface
				if (g_mergeMapperInterface) {                        // Use Interface
					const auto version = g_mergeMapperInterface->GetBuildNumber();
					logger::info("Got MergeMapper interface buildnumber {}", version);
				} else {
					logger::info("MergeMapper not detected");
				}
			}
			break;
		}
	}

	void InitializeLog()
	{
		auto path = logger::log_directory();
		if (!path) {
			stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log", "quicklootee-vr");
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%H:%M:%S:%e] %g(%#): [%^%l%$] %v"s);
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "quicklootee-vr";
	a_info->version = 1;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver <
		SKSE::RUNTIME_VR_1_4_15) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("loaded plugin");

	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(1 << 6);

	auto message = SKSE::GetMessagingInterface();
	if (!message->RegisterListener(MessageHandler)) {
		return false;
	}

	Hooks::Install();

	return true;
}

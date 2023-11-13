#pragma once

#include "CLIK/Array.h"
#include "CLIK/GFx/Controls/ButtonBar.h"
#include "CLIK/GFx/Controls/ScrollingList.h"
#include "CLIK/TextField.h"
#include "ContainerChangedHandler.h"
#include "Items/GroundItem.h"
#include "Items/InventoryItem.h"
#include "Items/Item.h"
#include "OpenCloseHandler.h"
#include "ViewHandler.h"
#include <numbers>

namespace Scaleform
{
	// VR notes: The LootMenu follows a similar setup to WSActivateRollver (VR menu for activation text on hands)
	class LootMenu :
		public RE::WorldSpaceMenu
	{
	private:
		using super = RE::WorldSpaceMenu;

	public:
		static constexpr std::string_view MenuName() noexcept { return MENU_NAME; }
		static constexpr std::int8_t SortPriority() noexcept { return SORT_PRIORITY; }

		static void Register()
		{
			auto ui = RE::UI::GetSingleton();
			if (ui) {
				ui->Register(MENU_NAME, Creator);
				logger::info("Registered {}"sv, MENU_NAME);
			}
		}

		void ModSelectedIndex(double a_mod)
		{
			const auto maxIdx = static_cast<double>(_itemListImpl.size()) - 1.0;
			if (maxIdx >= 0.0) {
				auto idx = _itemList.SelectedIndex();
				idx += a_mod;
				idx = std::clamp(idx, 0.0, maxIdx);
				_itemList.SelectedIndex(idx);
				UpdateInfoBar();
			}
		}

		void ModSelectedPage(double a_mod)
		{
			auto& inst = _itemList.GetInstance();
			std::array<RE::GFxValue, 1> args;
			args[0] = a_mod;
			[[maybe_unused]] const auto success =
				inst.Invoke("modSelectedPage", args);
			assert(success);
			UpdateInfoBar();
		}

		void SetContainer(RE::ObjectRefHandle a_ref)
		{
			assert(a_ref);
			_src = a_ref;
			_viewHandler->SetSource(a_ref);
			_containerChangedHandler.SetContainer(a_ref);
			_openCloseHandler.SetSource(a_ref);
			_itemList.SelectedIndex(0);
			QueueUIRefresh();
		}

		void RefreshInventory()
		{
			const auto idx = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());

			_itemListImpl.clear();
			auto src = _src.get();
			if (!src) {
				_itemListProvider.ClearElements();
				_itemList.Invalidate();
				_itemList.SelectedIndex(-1.0);
				return;
			}

			const auto stealing = WouldBeStealing();
			auto inv = src->GetInventory(CanDisplay);
			for (auto& [obj, data] : inv) {
				auto& [count, entry] = data;
				if (count > 0 && entry) {
					_itemListImpl.push_back(
						std::make_unique<Items::InventoryItem>(
							count, stealing, std::move(entry), _src));
				}
			}

			auto dropped = src->GetDroppedInventory(CanDisplay);
			for (auto& [obj, data] : dropped) {
				auto& [count, items] = data;
				if (count > 0 && !items.empty()) {
					_itemListImpl.push_back(
						std::make_unique<Items::GroundItems>(
							count, stealing, std::move(items)));
				}
			}

			if (_itemListImpl.size() >= 32 || (Settings::CloseWhenEmpty() && _itemListImpl.empty())) {
				Close();
			} else {
				Sort();
				_itemListProvider.ClearElements();
				for (const auto& elem : _itemListImpl) {
					_itemListProvider.PushBack(elem->GFxValue(*_view));
				}
				_itemList.InvalidateData();

				RestoreIndex(idx);
				UpdateWeight();
				UpdateInfoBar();

				_rootObj.Visible(true);
			}
		}

		void RefreshUI()
		{
			RefreshInventory();
			UpdateTitle();
			UpdateButtonBar();
		}

		void TakeStack()
		{
			auto dst = _dst.get();
			auto pos = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());
			if (dst && 0 <= pos && pos < std::ssize(_itemListImpl)) {
				_openCloseHandler.Open();
				_itemListImpl[static_cast<std::size_t>(pos)]->TakeAll(*dst);
				

				if (Settings::DispelInvisibility()) {
					dst->DispelEffectsWithArchetype(RE::EffectArchetypes::ArchetypeID::kInvisibility, false);
				}
			}

			QueueInventoryRefresh();
		}

		RE::BSEventNotifyControl ProcessEvent([[maybe_unused]] const RE::HudModeChangeEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::HudModeChangeEvent>* a_eventSource) override
		{
			return RE::BSEventNotifyControl::kContinue;
		}

	protected:
		using UIResult = RE::UI_MESSAGE_RESULTS;

		LootMenu() :
			super(0, 0, 1)
		{
			using UI_FLAG = RE::UI_MENU_FLAGS; 
			auto menu = static_cast<super*>(this);
			menu->menuName = MENU_NAME;
			menu->depthPriority = -1;
			menu->menuFlags.set(UI_FLAG::kAlwaysOpen);
			menu->menuFlags.reset(UI_FLAG::kDisablePauseMenu);
			auto scaleformManager = RE::BSScaleformManager::GetSingleton();
			[[maybe_unused]] const auto success =
				scaleformManager->LoadMovieEx(menu, FILE_NAME, [](RE::GFxMovieDef* a_def) -> void {
					a_def->SetState(
						RE::GFxState::StateType::kLog,
						RE::make_gptr<Logger>().get());
				});

			assert(success);
			_viewHandler.emplace(menu, _dst);
			_view = menu->uiMovie;
			_view->SetMouseCursorCount(0);  // disable input, we'll handle it ourselves
			InitExtensions();
		}

		LootMenu(const LootMenu&) = default;
		LootMenu(LootMenu&&) = default;

		~LootMenu()
		{
			super::~WorldSpaceMenu();
			DestroyMenuNode();
			_view.reset();
			_dst.reset();
			_src.reset();
		};

		LootMenu& operator=(const LootMenu&) = default;
		LootMenu& operator=(LootMenu&&) = default;

		static stl::owner<RE::IMenu*> Creator() { return new LootMenu(); }

		// IMenu
		void PostCreate() override
		{
			super::PostCreate();
			OnOpen();
		}

		UIResult ProcessMessage(RE::UIMessage& a_message) override
		{
			using Type = RE::UI_MESSAGE_TYPE;

			// VR Notes: WorldSpaceMenu's ProcessMessage(...) will hide the menu node on kHide, and show it on kShow for us
			switch (*a_message.type) {
			case Type::kHide:
				{
					SetShowing(false);
					break;
				}
			case Type::kShow:
				{
					SetShowing(true);
					RefreshMenuNode(); // We don't *have* to recreate the menu node, we only do it to apply the latest MCM values
					break;
				}
			}

			return super::ProcessMessage(a_message);
		}

		void AdvanceMovie(float a_interval, std::uint32_t a_currentTime) override
		{
			auto src = _src.get();
			if (!src || src->IsActivationBlocked()) {
				Close();
			}

			ProcessDelegate();
			super::AdvanceMovie(a_interval, a_currentTime);
		}

		void RefreshPlatform() override
		{
			UpdateButtonBar();
		}

		RE::NiNode* GetMenuParentNode() override
		{
			return RE::PlayerCharacter::GetSingleton()->UprightHmdNode.get();  // TODO: Is this the best for the menu to attach to?
		}

		RE::NiPointer<RE::NiNode> GetAttachingNode()
		{
			auto player = RE::PlayerCharacter::GetSingleton();

			return player->isRightHandMainHand ? player->RightWandNode : player->LeftWandNode;
		}

		void SetTransform() override
		{
			GetAttachingNode().get()->AttachChild(menuNode.get(), true);
			AdjustMenuNode();
		}

	private:
		class Logger :
			public RE::GFxLog
		{
		public:
			void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override
			{
				std::string fmt(a_fmt ? a_fmt : "");
				while (!fmt.empty() && fmt.back() == '\n') {
					fmt.pop_back();
				}

				std::va_list args;
				va_copy(args, a_argList);
				std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
				std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
				va_end(args);

				logger::info("{}: {}"sv, LootMenu::MenuName(), buf.data());
			}
		};

		[[nodiscard]] static bool CanDisplay(const RE::TESBoundObject& a_object)
		{
			switch (a_object.GetFormType()) {
			case RE::FormType::Scroll:
			case RE::FormType::Armor:
			case RE::FormType::Book:
			case RE::FormType::Ingredient:
			case RE::FormType::Misc:
			case RE::FormType::Weapon:
			case RE::FormType::Ammo:
			case RE::FormType::KeyMaster:
			case RE::FormType::AlchemyItem:
			case RE::FormType::Note:
			case RE::FormType::SoulGem:
				break;
			case RE::FormType::Light:
				{
					auto& light = static_cast<const RE::TESObjectLIGH&>(a_object);
					if (!light.CanBeCarried()) {
						return false;
					}
				}
				break;
			default:
				return false;
			}

			if (!a_object.GetPlayable()) {
				return false;
			}

			auto name = a_object.GetName();
			if (!name || name[0] == '\0') {
				return false;
			}

			return true;
		}

		void AdjustPosition()
		{
			// VR notes: Too high of a height and width causes clipping where only a portion of the menu is inside the menuNode.
			// Too low values result in blurriness, likely due to the game expanding the menu to fit the node
			// 1000 seems the sweet spot for maximum clariy with minimal clipping
			auto def = _view->GetMovieDef();

			if (!def) {
				return;
			}

			_rootObj.Width(1000.0f);

			_rootObj.Height(1000.0f);
		}

		void AdjustMenuNode()
		{
			const float VRScale = Settings::VRScale();
			const float TranslateX = Settings::VRTranslateX();
			const float TranslateY = Settings::VRTranslateY();
			const float TranslateZ = Settings::VRTranslateZ();
			// We use degrees for the MCM, but internally convert to radians for the EulerAngles
			const float RotateX = Settings::VRRotateX() * (std::numbers::pi / 180);
			const float RotateY = Settings::VRRotateY() * (std::numbers::pi / 180);
			const float RotateZ = Settings::VRRotateZ() * (std::numbers::pi / 180);

			menuNode->local.translate = RE::NiPoint3(TranslateX, TranslateY, TranslateZ);

			menuNode->local.rotate.EulerAnglesToAxesZXY(RotateX, RotateY, RotateZ);

			menuNode->local.scale = VRScale;

			RE::NiUpdateData data{ 0 };
			menuNode->Update(data);
		}

		void DestroyMenuNode() {
			// Possible overkill, but better safe than sorry
			if (menuNode.get()) {
				if (menuNode->parent) {
					menuNode->parent->DetachChild2(menuNode.get());
				}
				auto player = RE::PlayerCharacter::GetSingleton();
				player->LeftWandNode.get()->DetachChild2(menuNode.get());
				player->RightWandNode.get()->DetachChild2(menuNode.get());
				GetMenuParentNode()->DetachChild2(menuNode.get());

				menuNode.reset();
			}
		}

		// Recreates the menu node with the new data
		void RefreshMenuNode()
		{
			// First, remove and destroy the old node from the parents if it exists
			DestroyMenuNode();

			// Adjust Position data before creating the new node
			AdjustPosition();

			// Create and attach the new node
			SetupMenuNode();
			GetAttachingNode().get()->AttachChild(menuNode.get(), true);

			// Adjust the new node data
			AdjustMenuNode();
		}

		void Close();

		void InitExtensions()
		{
			const RE::GFxValue boolean{ true };
			[[maybe_unused]] bool success = false;

			success = _view->SetVariable("_global.gfxExtensions", boolean);
			assert(success);
			//success = _view->SetVariable("_global.noInvisibleAdvance", boolean);
			assert(success);
		}

		void OnClose() { return; }

		void OnOpen()
		{
			using element_t = std::pair<std::reference_wrapper<CLIK::Object>, std::string_view>;
			std::array objects{
				element_t{ std::ref(_rootObj), "_root.rootObj"sv },
				element_t{ std::ref(_title), "_root.rootObj.title"sv },
				element_t{ std::ref(_weight), "_root.rootObj.weightContainer.textField"sv },
				element_t{ std::ref(_itemList), "_root.rootObj.itemList"sv },
				element_t{ std::ref(_infoBar), "_root.rootObj.infoBar"sv },
				element_t{ std::ref(_buttonBar), "_root.rootObj.buttonBar"sv }
			};

			for (const auto& [object, path] : objects) {
				auto& instance = object.get().GetInstance();
				[[maybe_unused]] const auto success =
					_view->GetVariable(std::addressof(instance), path.data());
				assert(success && instance.IsObject());
			}

			AdjustPosition();
			AdjustMenuNode();
			_rootObj.Visible(false);

			_title.AutoSize(CLIK::Object{ "left" });
			_title.Visible(false);
			_weight.AutoSize(CLIK::Object{ "left" });
			_weight.Visible(false);

			_view->CreateArray(std::addressof(_itemListProvider));
			_itemList.DataProvider(CLIK::Array{ _itemListProvider });

			_view->CreateArray(std::addressof(_infoBarProvider));
			_infoBar.DataProvider(CLIK::Array{ _infoBarProvider });

			_view->CreateArray(std::addressof(_buttonBarProvider));
			_buttonBar.DataProvider(CLIK::Array{ _buttonBarProvider });

			ProcessDelegate();
		}

		void ProcessDelegate();
		void QueueInventoryRefresh();
		void QueueUIRefresh();
		void SetShowing(bool isShowing);

		void RestoreIndex(std::ptrdiff_t a_oldIdx)
		{
			if (const auto ssize = std::ssize(_itemListImpl); 0 <= a_oldIdx && a_oldIdx < ssize) {
				_itemList.SelectedIndex(static_cast<double>(a_oldIdx));
			} else if (!_itemListImpl.empty()) {
				if (a_oldIdx >= ssize) {
					_itemList.SelectedIndex(static_cast<double>(ssize) - 1.0);
				} else {
					_itemList.SelectedIndex(0.0);
				}
			} else {
				_itemList.SelectedIndex(-1.0);
			}
		}

		void Sort()
		{
			std::stable_sort(
				_itemListImpl.begin(),
				_itemListImpl.end(),
				[&](auto&& a_lhs, auto&& a_rhs) {
					return *a_lhs < *a_rhs;
				});
		}

		void UpdateButtonBar()
		{
			// VR Notes: The button bar works fine, but it would be a lot of work to get the correct VR buttons to show
		}

		void UpdateInfoBar()
		{
			_infoBarProvider.ClearElements();
			const auto idx = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());
			if (0 <= idx && idx < std::ssize(_itemListImpl)) {
				const std::array functors{
					std::function{ [](const Items::Item& a_val) { return fmt::format(FMT_STRING("{:.1f}"), a_val.Weight()); } },
					std::function{ [](const Items::Item& a_val) { return fmt::format(FMT_STRING("{}"), a_val.Value()); } },
				};

				const auto& item = _itemListImpl[static_cast<std::size_t>(idx)];
				std::string str;
				RE::GFxValue obj;
				for (const auto& functor : functors) {
					str = functor(*item);
					obj.SetString(str);
					_infoBarProvider.PushBack(obj);
				}

				const auto ench = item->EnchantmentCharge();
				if (ench >= 0.0) {
					str = fmt::format(FMT_STRING("{:.1f}%"), ench);
					obj.SetString(str);
					_infoBarProvider.PushBack(obj);
				}
			}

			_infoBar.InvalidateData();
		}

		void UpdateTitle()
		{
			auto src = _src.get();
			if (src) {
				_title.HTMLText(
					stl::safe_string(
						src->GetDisplayFullName()));
				_title.Visible(true);
			}
		}

		void UpdateWeight()
		{
			auto dst = _dst.get();
			if (dst) {
				auto inventoryWeight =
					static_cast<std::ptrdiff_t>(dst->GetWeightInContainer());
				auto carryWeight =
					static_cast<std::ptrdiff_t>(dst->GetActorValue(RE::ActorValue::kCarryWeight));
				auto text = std::to_string(inventoryWeight);
				text += " / ";
				text += std::to_string(carryWeight);
				_weight.HTMLText(text);
				_weight.Visible(true);
			}
		}

		[[nodiscard]] bool WouldBeStealing() const
		{
			auto dst = _dst.get();
			auto src = _src.get();
			return dst && src && dst->WouldBeStealing(src.get());
		}

		static constexpr std::string_view FILE_NAME{ "LootMenu" };
		static constexpr std::string_view MENU_NAME{ "LootMenu" };
		static constexpr std::int8_t SORT_PRIORITY{ 3 };

		RE::GPtr<RE::GFxMovieView> _view;
		RE::ActorHandle _dst{ RE::PlayerCharacter::GetSingleton() };
		RE::ObjectRefHandle _src;

		std::optional<ViewHandler> _viewHandler;
		ContainerChangedHandler _containerChangedHandler;
		OpenCloseHandler _openCloseHandler{ _dst };

		CLIK::MovieClip _rootObj;
		CLIK::TextField _title;
		CLIK::TextField _weight;

		CLIK::GFx::Controls::ScrollingList _itemList;
		RE::GFxValue _itemListProvider;
		std::vector<std::unique_ptr<Items::Item>> _itemListImpl;

		CLIK::GFx::Controls::ButtonBar _infoBar;
		RE::GFxValue _infoBarProvider;

		CLIK::GFx::Controls::ButtonBar _buttonBar;
		RE::GFxValue _buttonBarProvider;
	};
}

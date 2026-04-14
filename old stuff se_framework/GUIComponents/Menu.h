#pragma once

class MenuHelper
{
protected:
	struct MenuCallback;

public:
	MenuHelper() = default;
	~MenuHelper() = default;

	static void clear(PopupMenu & popupMenu);

	static void show(PopupMenu * popupMenu, Component * triggeringComponent, PopupMenu::Options options = {});

	static void addCallbackItem(PopupMenu & popupMenu, const String & text, std::function<void()> func, Drawable * image = nullptr);

	static void addGreyedOutItem(PopupMenu & popupMenu, const String & text, Drawable * image = nullptr);

	static void addSubMenu(PopupMenu & popupMenu, const String & text, PopupMenu & submenu);

	static void addSeperator(PopupMenu & popupMenu);

protected:

	struct MenuCallback : public PopupMenu::CustomCallback
	{
		MenuCallback(std::function<void()> callback_) : callback(std::move(callback_)) { }
		bool menuItemTriggered() override
		{
			if (callback != nullptr)
				callback();

			return false;
		}

		std::function<void()> callback;
	};
};

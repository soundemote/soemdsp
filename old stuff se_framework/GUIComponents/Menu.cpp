#include "Menu.h"

void MenuHelper::clear(PopupMenu & popupMenu)
{
	popupMenu.clear();
}

void MenuHelper::show(PopupMenu * popupMenu, Component * triggeringComponent, PopupMenu::Options options)
{
	popupMenu->showMenuAsync(options.withTargetComponent(triggeringComponent), {});
}

void MenuHelper::addCallbackItem(PopupMenu & popupMenu, const String & text, std::function<void()> func, Drawable * image)
{
	PopupMenu::Item item;
	item.itemID = 1;
	item.text = text;
	item.customCallback = new MenuCallback(func);
	item.image.reset(image);
	popupMenu.addItem(item);
}

void MenuHelper::addGreyedOutItem(PopupMenu & popupMenu, const String & text, Drawable * image)
{
	PopupMenu::Item item;
	item.itemID = 1;
	item.text = text;
	item.isEnabled = false;
	item.image.reset(image);
	popupMenu.addItem(item);
}

void MenuHelper::addSubMenu(PopupMenu & popupMenu, const String & text, PopupMenu & submenu)
{
	popupMenu.addSubMenu(text, submenu);
}

void MenuHelper::addSeperator(PopupMenu & popupMenu)
{
	popupMenu.addSeparator();
}

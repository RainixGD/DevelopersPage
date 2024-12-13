#pragma once
#include "./includes.h"

class SocialNetworkButton : public CCMenuItemSpriteExtra {
	std::string link;

	void onClick(CCObject*) {
		CCApplication::sharedApplication()->openURL(link.c_str());
	}

	virtual bool init(CCSprite* sprite, const std::string& link) {
		if (!CCMenuItemSpriteExtra::init(sprite, this, menu_selector(SocialNetworkButton::onClick))) return false;

		this->link = link;

		return true;
	}

public:

	static SocialNetworkButton* create(CCSprite* sprite, const std::string& link) {
		auto ret = new SocialNetworkButton();
		if (ret && ret->init(sprite, link)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}
};
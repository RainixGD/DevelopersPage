#include "./includes.h"
#include "./BrownAlertDelegate.hpp"
#include "./SocialNetworkButton.h"

struct DeveloperSocialNetworkData {
	std::string texture;
	std::string link;
};

struct DeveloperData {
	std::string name;
	std::string role;
	std::string logo;
	std::vector<DeveloperSocialNetworkData*> socialNetworks;
};

class DeveloperNode : public CCNode {

	virtual bool init(DeveloperData* devData) {
		if (!CCNode::init()) return false;

		auto devName = CCLabelBMFont::create(devData->name.c_str(), "bigFont.fnt");
		devName->setPosition({ 80, 40 });
		devName->limitLabelWidth(240, 1, 0.1);
		this->addChild(devName);

		auto devRole = CCLabelBMFont::create(devData->role.c_str(), "goldFont.fnt");
		devRole->setPosition({ 80, 15 });
		devRole->limitLabelWidth(175, 1, 0.1);
		this->addChild(devRole);

		auto devLogo = CCSprite::create(devData->logo.c_str());
		if (devLogo == NULL) {
			devLogo = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
			devLogo->setScale(2.5);
		}
		devLogo->setPosition({ -100, 0 });
		this->addChild(devLogo);

		auto menu = CCMenu::create();
		menu->setPosition(80, -25);
		this->addChild(menu);

		for (auto socialNetwork : devData->socialNetworks) {
			auto socialNetworkBtnSprite = CCSprite::create(socialNetwork->texture.c_str());
			if (socialNetworkBtnSprite == NULL)
				socialNetworkBtnSprite = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
			socialNetworkBtnSprite->setContentSize({ 30.5, 30.5 });
			auto devBtn = SocialNetworkButton::create(socialNetworkBtnSprite, socialNetwork->link);
			menu->addChild(devBtn);
		}

		menu->alignItemsHorizontallyWithPadding(10);

		return true;
	}

public:
	static DeveloperNode* create(DeveloperData* devData) {
		auto ret = new DeveloperNode;
		if (ret && ret->init(devData)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}
};


class DevelopersPopup : public BrownAlertDelegate {
	std::vector<DeveloperData*> devData;
	std::vector<DeveloperNode*> devNodes;
	CCMenuItemSpriteExtra* nextButton;
	CCMenuItemSpriteExtra* previousButton;
	int selectedDev = 0;
protected:
	void setup() override {
		if (devData.size() == 0) return;
		
		auto size = CCDirector::sharedDirector()->getWinSize();

		auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
		nextSprite->setFlipX(true);
		nextButton = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(DevelopersPopup::onNext));
		nextButton->setPosition({ 195, 0 });
		m_pButtonMenu->addChild(nextButton);

		auto previousSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
		previousButton = CCMenuItemSpriteExtra::create(previousSprite, this, menu_selector(DevelopersPopup::onPrevious));
		previousButton->setPosition({ -195, 0 });
		m_pButtonMenu->addChild(previousButton);

		for (auto dev : devData) {
			auto devNode = DeveloperNode::create(dev);
			devNode->setPosition({ size.width / 2, size.height / 2 });
			m_pLayer->addChild(devNode);
			this->devNodes.push_back(devNode);
		}

		updateDevNodes();
		updateControlButtons();

		CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
		this->registerWithTouchDispatcher();
	}

	void updateControlButtons() {
		nextButton->setVisible(selectedDev == devNodes.size() - 1 ? 0 : 1);
		previousButton->setVisible(selectedDev == 0 ? 0 : 1);
	}

	void updateDevNodes() {
		for (int i = 0; i < devNodes.size(); i++)
			devNodes[i]->setVisible(i == selectedDev ? 1 : 0);
	}

	void onNext(CCObject*) {
		if (selectedDev < devData.size() - 1) {
			selectedDev++;
		}
		updateDevNodes();
		updateControlButtons();
	}

	void onPrevious(CCObject*) {
		if (selectedDev > 0) {
			selectedDev--;
		}
		updateDevNodes();
		updateControlButtons();
	}
public:
	static DevelopersPopup* create(const std::vector<DeveloperData*>& devData) {
		auto ret = new DevelopersPopup;
		ret->devData = devData;
		if (ret && ret->init(435, 160, "GJ_square01.png", "")) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
};


class DevelopersPageManager {

	enum DataLoadingResult {
		OK,
		FileNotFound,
		ParsingError,
		TooManyButtons,
		InvalidUrl
	};

	std::vector<DeveloperData*> data;
	DataLoadingResult loadingStatus;
	static DevelopersPageManager* instance;

	void init() {
		loadingStatus = loadData();
	}

	bool isValidURL(const std::string& url) {
		std::regex url_regex(R"(^https?://)");
		return std::regex_search(url, url_regex);
	}

	DataLoadingResult loadData() {

		std::ifstream file("Resources/devPanel.json");
		if (!file) return FileNotFound;
		std::ostringstream buffer;
		buffer << file.rdbuf();
		std::string fileContent = buffer.str();

		file.close();

		try {
			auto root = nlohmann::json::parse(fileContent);
			if (!root.is_array()) return ParsingError;

			for (auto developer : root) {
				if (!developer.contains("name") || !developer["name"].is_string() || 
					!developer.contains("role") || !developer["role"].is_string() ||
					!developer.contains("logo") || !developer["logo"].is_string() ||
					!developer.contains("buttons") || !developer["buttons"].is_array()) return ParsingError;

				auto developerData = new DeveloperData();
				developerData->name = developer["name"];
				developerData->role = developer["role"];
				developerData->logo = developer["logo"];
				

				auto buttons = developer["buttons"];

				if (buttons.size() > 6) return TooManyButtons;

				for (auto btn : buttons) {
					if (!btn.contains("texture") || !btn["texture"].is_string()
						|| !btn.contains("link") || !btn["link"].is_string()) return ParsingError;

					auto socialNetworkData = new DeveloperSocialNetworkData;
					socialNetworkData->texture = btn["texture"];
					socialNetworkData->link = btn["link"];
					if (!isValidURL(socialNetworkData->link)) return InvalidUrl;

					developerData->socialNetworks.push_back(socialNetworkData);
				}

				data.push_back(developerData);
			}
		}
		catch (...) {
			return ParsingError;
		}
		return OK;
	}

	DevelopersPageManager() {};
public:
	void onDevelopersBtn(CCObject*) {
		DevelopersPopup::create(DevelopersPageManager::getInstance()->data)->show();
	}

	void onMenuLayer(MenuLayer* layer) {

		if (loadingStatus != OK) {

			std::string errorText;
			switch (loadingStatus) {
			case FileNotFound:
				errorText = "Can't find 'devPanel.json' in ./Resources";
				break;
			case ParsingError:
				errorText = "Can't parse 'devPanel.json'";
				break;
			case TooManyButtons:
				errorText = "Too many buttons in 'devPanel.json'";
				break;
			case InvalidUrl:
				errorText = "Links for buttons should start with 'http://' or 'https://' in 'devPanel.json'";
				break;
			}

			auto size = CCDirector::sharedDirector()->getWinSize();

			auto errorLabel = CCLabelBMFont::create(errorText.c_str(), "bigFont.fnt");
			errorLabel->setColor({ 255, 0, 0 });
			errorLabel->setScale(0.6);
			errorLabel->setPosition({ size.width / 2, size.height - 10 });
			layer->addChild(errorLabel);

			return;
		}


		int childrenCount = layer->getChildrenCount();
		for (int i = 0; i < childrenCount; i++) {
			int x = (dynamic_cast<CCNode*>(layer->getChildren()->objectAtIndex(i)))->getPositionX();
			int y = (dynamic_cast<CCNode*>(layer->getChildren()->objectAtIndex(i)))->getPositionY();
			auto obj = dynamic_cast<CCNode*>(layer->getChildren()->objectAtIndex(i));

			if (obj->getPosition().x == CCDirector::sharedDirector()->getWinSize().width - 43 && obj->getPosition().y == 45) {
				auto menuChildrenCount = obj->getChildrenCount();
				for (int i = 0; i < menuChildrenCount - 1; i++) {
					auto moreGamesButton = dynamic_cast<CCNode*>(obj->getChildren()->objectAtIndex(i));
					if (moreGamesButton->getPositionX() == 0 && moreGamesButton->getPositionY() == 0) {
						moreGamesButton->removeFromParent();

						auto devSprite = CCSprite::create("developers.png");
						if (devSprite == NULL)
							devSprite = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
						devSprite->setScale(1.5);
						auto devBtn = CCMenuItemSpriteExtra::create(devSprite, layer, menu_selector(DevelopersPageManager::onDevelopersBtn));
						devBtn->setPosition({ 0, 0 });
						obj->addChild(devBtn);
					}
				}
			}
		}
	}

	static DevelopersPageManager* getInstance() {
		if (!instance) {
			instance = new DevelopersPageManager();
			instance->init();
		}
		return instance;
	}

};
DevelopersPageManager* DevelopersPageManager::instance = nullptr;


bool(__thiscall* MenuLayer_init)(MenuLayer* self);
bool __fastcall MenuLayer_init_H(MenuLayer* self, void*) {
	if (!MenuLayer_init(self)) return false;
	DevelopersPageManager::getInstance()->onMenuLayer(self);
	return true;
}

void inject() {
#if _WIN32
	auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

	MH_CreateHook(
		reinterpret_cast<void*>(base + 0x1907b0),
		reinterpret_cast<void*>(&MenuLayer_init_H),
		reinterpret_cast<void**>(&MenuLayer_init)
	);

	MH_EnableHook(MH_ALL_HOOKS);
#endif
}

#if _WIN32
WIN32CAC_ENTRY(inject)
#endif
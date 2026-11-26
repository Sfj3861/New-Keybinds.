#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/ModsLayer.hpp>

using namespace geode::prelude;

// First UI milestone: a dedicated popup entry point and categorized overview.
// Binding capture/editing is intentionally left to the next milestone.
class NewKeybindsPopup : public Popup<> {
protected:
    bool init() override {
        if (!Popup::init(360.f, 260.f)) return false;
        this->setTitle("New Keybinds");

        auto content = this->m_mainLayer;
        auto heading = CCLabelBMFont::create("Keyboard controls", "goldFont.fnt");
        heading->setScale(.55f);
        heading->setPosition({m_size.width / 2, m_size.height - 48.f});
        content->addChild(heading);

        auto makeSection = [&](const char* title, const char* details, float y) {
            auto label = CCLabelBMFont::create(title, "bigFont.fnt");
            label->setScale(.45f);
            label->setAnchorPoint({0.f, .5f});
            label->setPosition({24.f, y});
            content->addChild(label);

            auto info = CCLabelBMFont::create(details, "goldFont.fnt");
            info->setScale(.38f);
            info->setAnchorPoint({0.f, .5f});
            info->setPosition({24.f, y - 22.f});
            info->setColor({190, 190, 190});
            content->addChild(info);
        };

        makeSection("Gameplay", "P1 / P2, practice checkpoints, restart", 175.f);
        makeSection("Editor", "Selection movement and rotation", 112.f);
        makeSection("Navigation", "Navigation shortcuts (planned)", 49.f);

        auto devices = CCLabelBMFont::create("Devices: controller and Smart Controls planned", "goldFont.fnt");
        devices->setScale(.36f);
        devices->setPosition({m_size.width / 2, 20.f});
        devices->setColor({160, 190, 255});
        content->addChild(devices);
        return true;
    }

public:
    static NewKeybindsPopup* create() {
        auto ret = new NewKeybindsPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(NewKeybindsModsLayer, ModsLayer) {
    void onNewKeybinds(CCObject*) {
        NewKeybindsPopup::create()->show();
    }

    bool init() {
        if (!ModsLayer::init()) return false;
        auto menu = this->getChildByID("main-menu");
        if (!menu) menu = this->getChildByID("mods-menu");
        if (!menu) return true;

        if (menu->getChildByID("new-keybinds-entry")) return true;
        auto label = CCLabelBMFont::create("New Keybinds", "goldFont.fnt");
        label->setScale(.55f);
        auto item = CCMenuItemExt::createSpriteExtra(label, [this](CCObject*) {
            this->onNewKeybinds(nullptr);
        });
        item->setID("new-keybinds-entry");
        menu->addChild(item);
        menu->updateLayout();
        return true;
    }
};

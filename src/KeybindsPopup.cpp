#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class InstantActionPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(380.f, 300.f)) return false;
        setTitle("Instant Actions");

        auto header = CCLabelBMFont::create("Choose an action context", "goldFont.fnt");
        header->setScale(.5f);
        header->setPosition({m_size.width / 2, m_size.height - 48.f});
        m_mainLayer->addChild(header);

        auto info = CCLabelBMFont::create("Context selected: none", "goldFont.fnt");
        info->setScale(.38f);
        info->setPosition({m_size.width / 2, 55.f});
        info->setColor({180, 200, 255});
        m_mainLayer->addChild(info);

        const char* contexts[] = {
            "Gameplay", "Practice", "Editor",
            "Editor - Create", "Editor - Edit", "Editor - Delete",
            "Pause Menu", "Main Menu", "Level Select"
        };

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2, m_size.height / 2 - 5.f});
        menu->setContentSize({m_size.width - 40.f, 150.f});

        for (int i = 0; i < 9; ++i) {
            auto label = CCLabelBMFont::create(contexts[i], "goldFont.fnt");
            label->setScale(.42f);
            auto item = CCMenuItemExt::createSpriteExtra(label, [info, context = std::string(contexts[i])](CCObject*) {
                info->setString(("Selected context: " + context).c_str());
            });
            item->setContentSize({110.f, 30.f});
            item->setPosition({(i % 3) * 115.f - 115.f, (1 - i / 3) * 40.f});
            menu->addChild(item);
        }

        m_mainLayer->addChild(menu);

        auto note = CCLabelBMFont::create(
            "Context selection only; execution is not implemented yet", "goldFont.fnt"
        );
        note->setScale(.30f);
        note->setPosition({m_size.width / 2, 20.f});
        note->setColor({160, 160, 160});
        m_mainLayer->addChild(note);
        return true;
    }

public:
    static InstantActionPopup* create() {
        auto ret = new InstantActionPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class CustomKeybindsPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(360.f, 230.f)) return false;
        setTitle("Custom Keybinds");

        auto addSection = [this](const char* title, const char* description, float y) {
            auto heading = CCLabelBMFont::create(title, "bigFont.fnt");
            heading->setScale(.45f);
            heading->setAnchorPoint({0.f, .5f});
            heading->setPosition({24.f, y});
            m_mainLayer->addChild(heading);

            auto detail = CCLabelBMFont::create(description, "goldFont.fnt");
            detail->setScale(.38f);
            detail->setAnchorPoint({0.f, .5f});
            detail->setPosition({24.f, y - 22.f});
            detail->setColor({190, 190, 190});
            m_mainLayer->addChild(detail);
        };

        addSection("Gameplay", "P1/P2, checkpoints and restart", 155.f);
        addSection("Editor", "Selection movement and rotation", 90.f);

        auto planned = CCLabelBMFont::create("Smart Controls planned for Beta-3", "goldFont.fnt");
        planned->setScale(.34f);
        planned->setPosition({m_size.width / 2, 24.f});
        planned->setColor({160, 190, 255});
        m_mainLayer->addChild(planned);
        return true;
    }

public:
    static CustomKeybindsPopup* create() {
        auto ret = new CustomKeybindsPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class NewKeybindsHubPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(300.f, 190.f)) return false;
        setTitle("New Keybinds");

        auto subtitle = CCLabelBMFont::create("Choose a section", "goldFont.fnt");
        subtitle->setScale(.42f);
        subtitle->setPosition({m_size.width / 2, m_size.height - 52.f});
        m_mainLayer->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2, m_size.height / 2 - 5.f});

        auto customLabel = CCLabelBMFont::create("Custom Keybinds", "goldFont.fnt");
        customLabel->setScale(.55f);
        auto customButton = CCMenuItemExt::createSpriteExtra(customLabel, [](CCObject*) {
            CustomKeybindsPopup::create()->show();
        });
        customButton->setPosition({0.f, 25.f});
        menu->addChild(customButton);

        auto instantLabel = CCLabelBMFont::create("Instant Actions", "goldFont.fnt");
        instantLabel->setScale(.55f);
        auto instantButton = CCMenuItemExt::createSpriteExtra(instantLabel, [](CCObject*) {
            InstantActionPopup::create()->show();
        });
        instantButton->setPosition({0.f, -30.f});
        menu->addChild(instantButton);

        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static NewKeybindsHubPopup* create() {
        auto ret = new NewKeybindsHubPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// Add one New Keybinds shortcut to Geode's Mods Library action column.
// ModItemUIEvent is used instead of modifying Geometry Dash's main menu.
$execute {
    static ListenerHandle listener = ModItemUIEvent().listen(
        [](CCNode* item, std::string_view modID, std::optional<Mod*>) {
            if (modID != "sfj3861.new-keybinds" || !item) {
                return ListenerResult::Propagate;
            }

            CCNode* modsLayer = item;
            while (modsLayer && modsLayer->getID() != "ModsLayer") {
                modsLayer = modsLayer->getParent();
            }
            if (!modsLayer) return ListenerResult::Propagate;

            auto actionsMenu = modsLayer->getChildByID("right-actions-menu");
            if (!actionsMenu || actionsMenu->getChildByID("new-keybinds-library-button")) {
                return ListenerResult::Propagate;
            }

            auto logo = createModLogo(Mod::get());
            if (!logo) return ListenerResult::Propagate;

            auto button = CCMenuItemSpriteExtra::create(
                logo, nullptr, [](CCObject*) {
                    NewKeybindsHubPopup::create()->show();
                }
            );
            button->setID("new-keybinds-library-button");
            button->setScale(.8f);
            actionsMenu->addChild(button);
            actionsMenu->updateLayout();

            return ListenerResult::Propagate;
        }
    );
}

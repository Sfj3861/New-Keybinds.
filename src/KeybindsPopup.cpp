#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/GeodeUI.hpp>\n#include <Geode/binding/LevelBrowserLayer.hpp>\n#include <Geode/binding/GJSearchObject.hpp>\n#include <Geode/binding/MenuLayer.hpp>\n#include <Geode/binding/PlayLayer.hpp>\n#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;

class InstantActionPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(390.f, 315.f)) return false;
        setTitle("Instant Actions");

        auto subtitle = CCLabelBMFont::create("Select an action", "goldFont.fnt");
        subtitle->setScale(.48f);
        subtitle->setPosition({m_size.width / 2, m_size.height - 48.f});
        m_mainLayer->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2, m_size.height / 2 - 2.f});

        struct ActionEntry { char const* title; char const* texture; };
        ActionEntry const actions[] = {
            {"Gameplay", "GJ_button_01.png"},
            {"Practice", "GJ_button_02.png"},
            {"My Levels", "GJ_button_03.png"},
            {"Create Level", "GJ_button_04.png"},
            {"Edit Levels", "GJ_button_05.png"},
            {"Manage Levels", "GJ_button_06.png"},
            {"Pause Menu", "GJ_button_01.png"},
            {"Main Menu", "GJ_button_02.png"},
            {"Level Select", "GJ_button_03.png"}
        };

        for (int i = 0; i < 9; ++i) {
            auto sprite = ButtonSprite::create(actions[i].title, "goldFont.fnt", actions[i].texture, .62f);
            auto item = CCMenuItemExt::createSpriteExtra(sprite, [i](CCObject*) {
                // Gameplay and Practice both locate the requested online level directly.
                // Practice mode itself is selected after opening the level's info page.
                if (i == 0 || i == 1 || i == 8) {
                    auto search = GJSearchObject::create(
                        SearchType::Type19, "6508283&gameVersion=22"
                    );
                    switchToScene(LevelBrowserLayer::scene(search));
                    return;
                }
                if (i == 6) {
                    if (auto play = PlayLayer::get()) {
                        play->pauseGame(true);
                    } else {
                        FLAlertLayer::create("Instant Actions", "There is no active level to pause.", "OK")->show();
                    }
                    return;
                }
                if (i == 7) {
                    switchToScene(MenuLayer::scene(false));
                    return;
                }
                // Editor actions open the user's levels list, from which the editor can be entered.
                auto search = GJSearchObject::create(SearchType::MyLevels, "");
                switchToScene(LevelBrowserLayer::scene(search));
            });
            item->setPosition({(i % 3 - 1) * 116.f, (1 - i / 3) * 48.f});
            menu->addChild(item);
        }

        // Each item uses a real ButtonSprite with a bounded clickable area.\n        m_mainLayer->addChild(menu);

        auto note = CCLabelBMFont::create(
            "Gameplay and Practice open level search results", "goldFont.fnt"
        );
        note->setScale(.34f);
        note->setPosition({m_size.width / 2, 22.f});
        note->setColor({180, 195, 220});
        m_mainLayer->addChild(note);
        return true;
    }

public:
    static InstantActionPopup* create() {
        auto ret = new InstantActionPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class CustomKeybindsPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(360.f, 245.f)) return false;
        setTitle("Custom Keybinds");

        auto subtitle = CCLabelBMFont::create("Configure bindings in Geode's settings", "goldFont.fnt");
        subtitle->setScale(.42f);
        subtitle->setPosition({m_size.width / 2, m_size.height - 48.f});
        m_mainLayer->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2, m_size.height / 2 - 8.f});

        auto gameplay = ButtonSprite::create("Gameplay", "goldFont.fnt", "GJ_button_01.png", .7f);
        auto gameplayItem = CCMenuItemExt::createSpriteExtra(gameplay, [](CCObject*) {
            openKeybindsPopup(KeybindCategory::Gameplay, Mod::get());
        });
        gameplayItem->setPosition({0.f, 38.f});
        menu->addChild(gameplayItem);

        auto editor = ButtonSprite::create("Editor", "goldFont.fnt", "GJ_button_02.png", .7f);
        auto editorItem = CCMenuItemExt::createSpriteExtra(editor, [](CCObject*) {
            openKeybindsPopup(KeybindCategory::Editor, Mod::get());
        });
        editorItem->setPosition({0.f, -8.f});
        menu->addChild(editorItem);

        auto settings = ButtonSprite::create("All Mod Settings", "goldFont.fnt", "GJ_button_03.png", .7f);
        auto settingsItem = CCMenuItemExt::createSpriteExtra(settings, [](CCObject*) {
            openSettingsPopup(Mod::get());
        });
        settingsItem->setPosition({0.f, -54.f});
        menu->addChild(settingsItem);
        m_mainLayer->addChild(menu);

        auto note = CCLabelBMFont::create("Bindings are editable and persist through Geode", "goldFont.fnt");
        note->setScale(.32f);
        note->setPosition({m_size.width / 2, 22.f});
        note->setColor({180, 195, 220});
        m_mainLayer->addChild(note);
        return true;
    }
public:
    static CustomKeybindsPopup* create() {
        auto ret = new CustomKeybindsPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class SmartControlsPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(330.f, 220.f)) return false;
        setTitle("Smart Controls");
        auto* description = CCLabelBMFont::create(
            "On-screen jump buttons for mobile gameplay", "goldFont.fnt"
        );
        description->setScale(.38f);
        description->setPosition({m_size.width / 2, m_size.height - 55.f});
        m_mainLayer->addChild(description);

        auto enabled = Mod::get()->getSavedValue<bool>("smart-controls-enabled", false);
        auto* state = CCLabelBMFont::create(enabled ? "Enabled" : "Disabled", "goldFont.fnt");
        state->setScale(.55f);
        state->setPosition({m_size.width / 2, 95.f});
        state->setColor(enabled ? ccColor3B{100, 230, 130} : ccColor3B{230, 130, 100});
        m_mainLayer->addChild(state);

        auto* toggleLabel = CCLabelBMFont::create("Toggle Smart Controls", "goldFont.fnt");
        toggleLabel->setScale(.48f);
        auto* toggle = CCMenuItemExt::createSpriteExtra(toggleLabel, [state](CCObject*) {
            auto current = Mod::get()->getSavedValue<bool>("smart-controls-enabled", false);
            Mod::get()->setSavedValue("smart-controls-enabled", !current);
            state->setString(!current ? "Enabled (restart level)" : "Disabled");
            state->setColor(!current ? ccColor3B{100, 230, 130} : ccColor3B{230, 130, 100});
        });
        auto* menu = CCMenu::create();
        menu->addChild(toggle);
        menu->setPosition({m_size.width / 2, 55.f});
        m_mainLayer->addChild(menu);

        auto* note = CCLabelBMFont::create("Changes apply when gameplay layer is recreated", "goldFont.fnt");
        note->setScale(.30f);
        note->setPosition({m_size.width / 2, 22.f});
        note->setColor({170, 170, 170});
        m_mainLayer->addChild(note);
        return true;
    }
public:
    static SmartControlsPopup* create() {
        auto ret = new SmartControlsPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class NewKeybindsHubPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(320.f, 235.f)) return false;
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
        instantButton->setPosition({0.f, -25.f});
        menu->addChild(instantButton);

        auto smartLabel = CCLabelBMFont::create("Smart Controls", "goldFont.fnt");
        smartLabel->setScale(.55f);
        auto smartButton = CCMenuItemExt::createSpriteExtra(smartLabel, [](CCObject*) {
            SmartControlsPopup::create()->show();
        });
        smartButton->setPosition({0.f, -75.f});
        menu->addChild(smartButton);

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

            auto button = CCMenuItemExt::createSpriteExtra(logo, [](CCObject*) {
                NewKeybindsHubPopup::create()->show();
            });
            button->setID("new-keybinds-library-button");
            button->setScale(.8f);
            actionsMenu->addChild(button);
            actionsMenu->updateLayout();

            return ListenerResult::Propagate;
        }
    );
}

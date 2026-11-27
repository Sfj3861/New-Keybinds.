#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <algorithm>

using namespace geode::prelude;

class $modify(NewKeybindsUILayer, UILayer) {
    bool isGameplayLayerActive() const {
        auto* play = PlayLayer::get();
        auto* scene = CCDirector::sharedDirector()->getRunningScene();
        return play && scene && scene->getChildByType<PlayLayer>(0) == play;
    }

    void replayVanillaKey(enumKeyCodes key, bool down, double timestamp) {
        if (!isGameplayLayerActive()) return;

        auto* manager = GameManager::sharedState();
        auto* dispatcher = CCDirector::sharedDirector()->getKeyboardDispatcher();
        if (!manager || !dispatcher) return;

        auto quickKeys = manager->getGameVariable("0163");
        manager->setGameVariable("0163", true);
        auto oldShift = dispatcher->getShiftKeyPressed();
        auto oldControl = dispatcher->getControlKeyPressed();
        auto oldAlt = dispatcher->getAltKeyPressed();
        auto oldCommand = dispatcher->getCommandKeyPressed();

        dispatcher->updateModifierKeys(false, false, false, false);
        if (down) this->keyDown(key, timestamp);
        else this->keyUp(key, timestamp);

        dispatcher->updateModifierKeys(oldShift, oldControl, oldAlt, oldCommand);
        manager->setGameVariable("0163", quickKeys);
    }

    void forwardBinding(const char* id, enumKeyCodes key) {
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), id),
            [this, key](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !isGameplayLayerActive()) return ListenerResult::Propagate;
                replayVanillaKey(key, down, timestamp);
                return ListenerResult::Stop;
            });
    }

    void actionBinding(const char* id, std::function<void()> action) {
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), id),
            [action = std::move(action)](Keybind const&, bool down, bool repeat, double) {
                if (!down || repeat) return ListenerResult::Propagate;
                action();
                return ListenerResult::Stop;
            });
    }

    void openTargetSearch(bool practice) {
        auto target = Mod::get()->getSettingValue<std::string>("instant-level-id");
        if (target.empty()) target = "6508283";

        // Honest behavior: the shortcut opens the selected ID's search results.
        // The user still selects the result and the mode on the level info screen.
        auto search = GJSearchObject::create(SearchType::Type19, target + "&gameVersion=22");
        switchToScene(LevelBrowserLayer::scene(search));
        (void)practice;
    }

    void createSmartButtons() {
        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("smart-controls-enabled") || !PlayLayer::get())
            return;

        auto* menu = CCMenu::create();
        menu->setID("new-keybinds-smart-controls");
        menu->setPosition({0.f, 0.f});

        auto win = CCDirector::sharedDirector()->getWinSize();
        float scale = mod->getSettingValue<float>("smart-button-size");
        float opacity = mod->getSettingValue<float>("smart-button-opacity");

        auto addButton = [&](bool playerTwo, const char* title, float xOffset, float yOffset) {
            auto sprite = ButtonSprite::create(title, "goldFont.fnt", "GJ_button_01.png", .62f * scale);
            sprite->setOpacity(static_cast<GLubyte>(255.f * opacity));

            auto item = CCMenuItemExt::createSpriteExtra(sprite, [playerTwo](CCObject*) {
                if (auto* activePlay = PlayLayer::get()) {
                    // Balanced tap avoids leaving P1/P2 stuck in the pressed state.
                    activePlay->queueButton(1, true, playerTwo, 0.0);
                    activePlay->queueButton(1, false, playerTwo, 0.0);
                }
            });

            float x = std::clamp(win.width - 78.f + xOffset, 40.f, win.width - 40.f);
            float y = std::clamp(78.f + yOffset, 35.f, win.height - 35.f);
            item->setPosition({x, y});
            menu->addChild(item);
        };

        if (mod->getSettingValue<bool>("smart-p1-enabled"))
            addButton(false, "P1", mod->getSettingValue<float>("smart-p1-x"),
                mod->getSettingValue<float>("smart-p1-y"));
        if (mod->getSettingValue<bool>("smart-p2-enabled"))
            addButton(true, "P2", mod->getSettingValue<float>("smart-p2-x"),
                mod->getSettingValue<float>("smart-p2-y"));

        this->addChild(menu, 1000);
    }

    bool init(GJBaseGameLayer* layer) {
        if (!UILayer::init(layer)) return false;

        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), "jump-p1"),
            [layer](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                layer->queueButton(1, down, false, timestamp);
                return ListenerResult::Stop;
            });
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), "jump-p2"),
            [layer](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                layer->queueButton(1, down, true, timestamp);
                return ListenerResult::Stop;
            });

        forwardBinding("place-checkpoint", KEY_Z);
        forwardBinding("delete-checkpoint", KEY_X);
        forwardBinding("restart-level", KEY_R);

        actionBinding("instant-gameplay", [this] { openTargetSearch(false); });
        actionBinding("instant-practice", [this] { openTargetSearch(true); });
        actionBinding("instant-pause", [] {
            if (auto* play = PlayLayer::get()) play->pauseGame(true);
        });

        createSmartButtons();
        return true;
    }
};

// Universal shortcuts are active in menus as well as gameplay.
$execute {
    static ListenerHandle levelSelect = KeybindSettingPressedEventV3(Mod::get(), "instant-level-select")
        .listen([](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) return ListenerResult::Propagate;
            switchToScene(LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Type19, "")));
            return ListenerResult::Stop;
        });

    static ListenerHandle mainMenu = KeybindSettingPressedEventV3(Mod::get(), "instant-main-menu")
        .listen([](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) return ListenerResult::Propagate;
            switchToScene(MenuLayer::scene(false));
            return ListenerResult::Stop;
        });
}

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
            [this, key, actionID = std::string(id)](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !isGameplayLayerActive()) return ListenerResult::Propagate;
                nkb::recordStep(actionID + (down ? " DOWN" : " UP"));
                replayVanillaKey(key, down, timestamp);
                return ListenerResult::Stop;
            });
    }

    void actionBinding(const char* id, std::function<void()> action) {
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), id),
            [action = std::move(action), actionID = std::string(id)](Keybind const&, bool down, bool repeat, double) {
                if (!down || repeat) return ListenerResult::Propagate;
                nkb::recordStep(actionID);
                action();
                return ListenerResult::Stop;
            });
    }

    void openTargetSearch(bool practice) {
        launchTargetLevel(practice);
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
                    nkb::recordStep(playerTwo ? "Smart Controls P2 tap" : "Smart Controls P1 tap");
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
                nkb::recordStep(down ? "P1 DOWN" : "P1 UP");
                layer->queueButton(1, down, false, timestamp);
                return ListenerResult::Stop;
            });
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), "jump-p2"),
            [layer](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                nkb::recordStep(down ? "P2 DOWN" : "P2 UP");
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

        if (nkb::isRecording()) {
            auto win = CCDirector::sharedDirector()->getWinSize();
            auto menu = CCMenu::create();
            menu->setID("new-keybinds-floating-save-menu");
            menu->setPosition({0.f, 0.f});
            auto sprite = ButtonSprite::create("SAVE STEPS", "goldFont.fnt", "GJ_button_06.png", .58f);
            auto save = CCMenuItemExt::createSpriteExtra(sprite, [](CCObject*) {
                bool saved = nkb::saveRecording();
                FLAlertLayer::create("New Keybinds",
                    saved ? "Sequence saved. Open Instant Actions to review the recorded steps."
                          : "No recorded steps were captured.", "OK")->show();
            });
            save->setPosition({win.width * .5f, win.height - 30.f});
            menu->addChild(save);
            this->addChild(menu, 2000);
        }
        return true;
    }
};

// Practice-mode launch is applied after Geometry Dash creates the level through its own Play action.
class $modify(NewKeybindsPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        if (s_startPracticeMode) {
            s_startPracticeMode = false;
            this->togglePracticeMode(true);
        }
        return true;
    }
}

// Universal shortcuts are active in menus as well as gameplay.
$execute {
    static ListenerHandle levelSelect = KeybindSettingPressedEventV3(Mod::get(), "instant-level-select")
        .listen([](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) return ListenerResult::Propagate;
            nkb::recordStep("Open Online Level Browser");
            switchToScene(LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Type19, "")));
            return ListenerResult::Stop;
        });

    static ListenerHandle mainMenu = KeybindSettingPressedEventV3(Mod::get(), "instant-main-menu")
        .listen([](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) return ListenerResult::Propagate;
            nkb::recordStep("Open Main Menu");
            switchToScene(MenuLayer::scene(false));
            return ListenerResult::Stop;
        });
}

#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

class $modify(NewKeybindsUILayer, UILayer) {
    bool isGameplayLayerActive() const {
        auto* playLayer = PlayLayer::get();
        auto* scene = CCDirector::sharedDirector()->getRunningScene();
        return playLayer && scene && scene->getChildByType<PlayLayer>(0) == playLayer;
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

    void addForwardBinding(char const* id, enumKeyCodes key) {
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), id),
            [this, key](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !this->isGameplayLayerActive()) return ListenerResult::Propagate;
                this->replayVanillaKey(key, down, timestamp);
                return ListenerResult::Stop;
            });
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

        this->addForwardBinding("place-checkpoint", KEY_Z);
        this->addForwardBinding("delete-checkpoint", KEY_X);
        this->addForwardBinding("restart-level", KEY_R);

        // These bindings forward the chosen key event to the game. They do not
        // implement the underlying cheat feature by themselves.
        this->addForwardBinding("noclip-toggle", KEY_N);
        this->addForwardBinding("hitbox-toggle", KEY_H);
        this->addForwardBinding("trail-toggle", KEY_T);
        this->addForwardBinding("speedhack-toggle", KEY_K);
        return true;
    }
};

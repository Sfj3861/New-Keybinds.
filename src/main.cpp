#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

// Keyboard milestone 0.2:
// - P1/P2 jump routing
// - Practice checkpoint shortcuts
// - Level restart shortcut
class $modify(NewKeybindsUILayer, UILayer) {
    bool isGameplayLayerActive() const {
        auto* playLayer = PlayLayer::get();
        auto* scene = CCDirector::sharedDirector()->getRunningScene();
        return playLayer && scene && scene->getChildByType<PlayLayer>(0) == playLayer;
    }

    void replayVanillaKey(enumKeyCodes key, bool down, double timestamp) {
        if (!isGameplayLayerActive()) {
            return;
        }

        auto* manager = GameManager::sharedState();
        auto* dispatcher = CCDirector::sharedDirector()->getKeyboardDispatcher();
        if (!manager || !dispatcher) {
            return;
        }

        auto quickKeys = manager->getGameVariable("0163");
        manager->setGameVariable("0163", true);

        auto oldShift = dispatcher->getShiftKeyPressed();
        auto oldControl = dispatcher->getControlKeyPressed();
        auto oldAlt = dispatcher->getAltKeyPressed();
        auto oldCommand = dispatcher->getCommandKeyPressed();

        dispatcher->updateModifierKeys(false, false, false, false);
        if (down) {
            this->keyDown(key, timestamp);
        } else {
            this->keyUp(key, timestamp);
        }

        dispatcher->updateModifierKeys(oldShift, oldControl, oldAlt, oldCommand);
        manager->setGameVariable("0163", quickKeys);
    }

    bool init(GJBaseGameLayer* layer) {
        if (!UILayer::init(layer)) {
            return false;
        }

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "jump-p1"),
            [layer](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                layer->queueButton(1, down, false, timestamp);
                return ListenerResult::Stop;
            }
        );

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "jump-p2"),
            [layer](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                layer->queueButton(1, down, true, timestamp);
                return ListenerResult::Stop;
            }
        );

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "place-checkpoint"),
            [this](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !this->isGameplayLayerActive()) return ListenerResult::Propagate;
                this->replayVanillaKey(KEY_Z, down, timestamp);
                return ListenerResult::Stop;
            }
        );

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "delete-checkpoint"),
            [this](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !this->isGameplayLayerActive()) return ListenerResult::Propagate;
                this->replayVanillaKey(KEY_X, down, timestamp);
                return ListenerResult::Stop;
            }
        );

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "restart-level"),
            [this](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !this->isGameplayLayerActive()) return ListenerResult::Propagate;
                this->replayVanillaKey(KEY_R, down, timestamp);
                return ListenerResult::Stop;
            }
        );

        return true;
    }
};

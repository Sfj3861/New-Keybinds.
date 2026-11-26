#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

// Keyboard milestone 0.2:
// - P1/P2 jump routing
// - Practice checkpoint shortcuts
// - Level restart shortcut
//
// The keybind definitions live in mod.json. Geode handles capture, persistence,
// multiple bindings per action, and dispatching KeybindSettingPressedEventV3.
class $modify(NewKeybindsUILayer, UILayer) {
    static inline bool s_allowKeypressPassthrough = false;

    static void onModify(auto& self) {
        // Let Geode receive keybind events first. Replayed vanilla keys are
        // explicitly allowed through below.
        (void)self.setHookPriority("UILayer::handleKeypress", Priority::Late);
    }

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

        // Geometry Dash gates some keyboard shortcuts behind Quick Checkpoints.
        // Temporarily enable it only while forwarding this one event, then
        // restore the player's original setting immediately.
        auto quickKeys = manager->getGameVariable("0163");
        manager->setGameVariable("0163", true);

        auto oldShift = dispatcher->getShiftKeyPressed();
        auto oldControl = dispatcher->getControlKeyPressed();
        auto oldAlt = dispatcher->getAltKeyPressed();
        auto oldCommand = dispatcher->getCommandKeyPressed();

        dispatcher->updateModifierKeys(false, false, false, false);
        s_allowKeypressPassthrough = true;

        if (down) {
            this->keyDown(key, timestamp);
        } else {
            this->keyUp(key, timestamp);
        }

        s_allowKeypressPassthrough = false;
        dispatcher->updateModifierKeys(oldShift, oldControl, oldAlt, oldCommand);
        manager->setGameVariable("0163", quickKeys);
    }

    void handleKeypress(enumKeyCodes key, bool down, double timestamp) {
        if (key == KEY_Escape || s_allowKeypressPassthrough) {
            UILayer::handleKeypress(key, down, timestamp);
            s_allowKeypressPassthrough = false;
        }
    }

    bool init(GJBaseGameLayer* layer) {
        if (!UILayer::init(layer)) {
            return false;
        }

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "jump-p1"),
            [layer](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat) {
                    return ListenerResult::Propagate;
                }

                layer->queueButton(1, down, false, timestamp);
                return ListenerResult::Stop;
            }
        );

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "jump-p2"),
            [layer](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat) {
                    return ListenerResult::Propagate;
                }

                layer->queueButton(1, down, true, timestamp);
                return ListenerResult::Stop;
            }
        );

        // These actions are forwarded to Geometry Dash's existing keyboard
        // handlers so checkpoint and restart behavior stays vanilla.
        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "place-checkpoint"),
            [this](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !this->isGameplayLayerActive()) {
                    return ListenerResult::Propagate;
                }

                this->replayVanillaKey(KEY_Z, down, timestamp);
                return ListenerResult::Stop;
            }
        );

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "delete-checkpoint"),
            [this](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !this->isGameplayLayerActive()) {
                    return ListenerResult::Propagate;
                }

                this->replayVanillaKey(KEY_X, down, timestamp);
                return ListenerResult::Stop;
            }
        );

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "restart-level"),
            [this](Keybind const&, bool down, bool repeat, double timestamp) {
                if (repeat || !this->isGameplayLayerActive()) {
                    return ListenerResult::Propagate;
                }

                this->replayVanillaKey(KEY_R, down, timestamp);
                return ListenerResult::Stop;
            }
        );

        return true;
    }
};

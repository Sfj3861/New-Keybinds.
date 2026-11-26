#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

class $modify(NewKeybindsEditorUI, EditorUI) {
    static inline bool s_allowPassThrough = false;

    static void onModify(auto& self) {
        (void)self.setHookPriority("EditorUI::keyDown", Priority::Replace);
        (void)self.setHookPriority("EditorUI::keyUp", Priority::Late);
    }

    bool isTopLevel() {
        // Avoid stealing keyboard input while an IME/text-entry delegate is active.
        auto* ime = CCIMEDispatcher::sharedDispatcher();
        return !ime || !ime->hasDelegate();
    }

    void passThroughKeyDown(enumKeyCodes key, double timestamp) {
        s_allowPassThrough = true;
        this->keyDown(key, timestamp);
    }

    void defineEditorKeybind(std::string_view id, enumKeyCodes key) {
        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), std::string(id)),
            [this, key](Keybind const&, bool down, bool repeat, double timestamp) {
                if (!down || repeat || !this->isTopLevel()) return ListenerResult::Propagate;
                this->passThroughKeyDown(key, timestamp);
                return ListenerResult::Stop;
            }
        );
    }

    bool init(LevelEditorLayer* layer) {
        if (!EditorUI::init(layer)) return false;
        this->defineEditorKeybind("move-obj-up", KEY_W);
        this->defineEditorKeybind("move-obj-left", KEY_A);
        this->defineEditorKeybind("move-obj-down", KEY_S);
        this->defineEditorKeybind("move-obj-right", KEY_D);
        this->defineEditorKeybind("rotate-ccw", KEY_Q);
        this->defineEditorKeybind("rotate-cw", KEY_E);
        return true;
    }

    void keyDown(enumKeyCodes key, double timestamp) {
        if (key == KEY_Escape) {
            EditorUI::keyDown(key, timestamp);
            return;
        }
        if (s_allowPassThrough) {
            s_allowPassThrough = false;
            EditorUI::keyDown(key, timestamp);
        }
    }

    void keyUp(enumKeyCodes key, double timestamp) {
        if (s_allowPassThrough) {
            s_allowPassThrough = false;
            EditorUI::keyUp(key, timestamp);
        }
    }
};

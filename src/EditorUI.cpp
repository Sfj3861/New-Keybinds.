#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>

using namespace geode::prelude;

class $modify(NewKeybindsEditorUI, EditorUI) {
    static inline bool s_allowPassThrough = false;

    static void onModify(auto& self) {
        (void)self.setHookPriority("EditorUI::keyDown", Priority::Replace);
        (void)self.setHookPriority("EditorUI::keyUp", Priority::Late);
    }

    bool isTopLevel() {
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
                if (!down || repeat || !this->isTopLevel())
                    return ListenerResult::Propagate;
                this->passThroughKeyDown(key, timestamp);
                return ListenerResult::Stop;
            });
    }

    bool init(LevelEditorLayer* layer) {
        if (!EditorUI::init(layer)) return false;

        defineEditorKeybind("move-obj-up", KEY_W);
        defineEditorKeybind("move-obj-left", KEY_A);
        defineEditorKeybind("move-obj-down", KEY_S);
        defineEditorKeybind("move-obj-right", KEY_D);
        defineEditorKeybind("rotate-ccw", KEY_Q);
        defineEditorKeybind("rotate-cw", KEY_E);

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "instant-my-levels"),
            [](Keybind const&, bool down, bool repeat, double) {
                if (!down || repeat) return ListenerResult::Propagate;
                switchToScene(LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MyLevels, "")));
                return ListenerResult::Stop;
            });
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

#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

// First implementation milestone:
// route the two configurable jump actions through Geode's keybind events.
// Editor actions and the remaining gameplay controls will be added in later milestones.
class $modify(NewKeybindsUILayer, UILayer) {
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

        return true;
    }
};

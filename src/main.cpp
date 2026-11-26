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
        if (down) this->keyDown(key, timestamp); else this->keyUp(key, timestamp);
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
    void addSmartControls() {
        if (!Mod::get()->getSavedValue<bool>("smart-controls-enabled", false)) return;
        auto* play = PlayLayer::get();
        if (!play) return;
        auto* menu = CCMenu::create();
        menu->setID("new-keybinds-smart-controls");
        menu->setPosition({0.f, 0.f});
        auto makeButton = [menu, play](const char* title, CCPoint pos, bool player2) {
            auto* label = CCLabelBMFont::create(title, "goldFont.fnt");
            label->setScale(.48f);
            auto* item = CCMenuItemExt::createSpriteExtra(label, [play, player2](CCObject*) {
                play->queueButton(1, true, player2, 0.0);
                play->queueButton(1, false, player2, 0.0);
            });
            item->setPosition(pos);
            item->setContentSize({82.f, 54.f});
            menu->addChild(item);
        };
        auto win = CCDirector::sharedDirector()->getWinSize();
        makeButton("P1 JUMP", {win.width - 65.f, 82.f}, false);
        makeButton("P2 JUMP", {win.width - 65.f, 145.f}, true);
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
        this->addForwardBinding("place-checkpoint", KEY_Z);
        this->addForwardBinding("delete-checkpoint", KEY_X);
        this->addForwardBinding("restart-level", KEY_R);
        this->addSmartControls();
        return true;
    }
};

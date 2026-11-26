#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/modify/UILayer.hpp>
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
        if (down) this->keyDown(key, timestamp); else this->keyUp(key, timestamp);
        dispatcher->updateModifierKeys(oldShift, oldControl, oldAlt, oldCommand);
        manager->setGameVariable("0163", quickKeys);
    }
    void forwardBinding(const char* id, enumKeyCodes key) {
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(), id),
            [this,key](Keybind const&,bool down,bool repeat,double ts) {
                if(repeat||!isGameplayLayerActive()) return ListenerResult::Propagate;
                replayVanillaKey(key,down,ts); return ListenerResult::Stop;
            });
    }
    void actionBinding(const char* id, std::function<void()> action) {
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(),id),
            [action](Keybind const&,bool down,bool repeat,double) {
                if(!down||repeat) return ListenerResult::Propagate;
                action(); return ListenerResult::Stop;
            });
    }
    void openTarget(bool practice) {
        auto target=Mod::get()->getSettingValue<std::string>("instant-level-id");
        if(target.empty()) target="6508283";
        auto search=GJSearchObject::create(SearchType::Type19,target+"&gameVersion=22");
        switchToScene(LevelBrowserLayer::scene(search));
    }
    void createSmartButtons() {
        auto mod=Mod::get();
        if(!mod->getSettingValue<bool>("smart-controls-enabled")) return;
        auto* play=PlayLayer::get(); if(!play) return;
        auto* menu=CCMenu::create(); menu->setID("new-keybinds-smart-controls");
        menu->setPosition({0,0});
        auto win=CCDirector::sharedDirector()->getWinSize();
        float scale=mod->getSettingValue<float>("smart-button-size");
        float opacity=mod->getSettingValue<float>("smart-button-opacity");
        auto add=[&](bool p2,const char* text,float x,float y) {
            auto label=CCLabelBMFont::create(text,"goldFont.fnt"); label->setScale(.48f*scale);
            auto item=CCMenuItemExt::createSpriteExtra(label,[play,p2](CCObject*) {
                play->queueButton(1,true,p2,0.0); play->queueButton(1,false,p2,0.0);
            });
            item->setPosition({win.width+x,y}); item->setContentSize({82.f*scale,54.f*scale});
            item->setOpacity(static_cast<GLubyte>(255.f*opacity)); menu->addChild(item);
        };
        if(mod->getSettingValue<bool>("smart-p1-enabled")) add(false,"P1 JUMP",mod->getSettingValue<float>("smart-p1-x"),mod->getSettingValue<float>("smart-p1-y"));
        if(mod->getSettingValue<bool>("smart-p2-enabled")) add(true,"P2 JUMP",mod->getSettingValue<float>("smart-p2-x"),mod->getSettingValue<float>("smart-p2-y"));
        this->addChild(menu,1000);
    }
    bool init(GJBaseGameLayer* layer) {
        if(!UILayer::init(layer)) return false;
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(),"jump-p1"),[layer](Keybind const&,bool down,bool repeat,double ts){if(repeat)return ListenerResult::Propagate;layer->queueButton(1,down,false,ts);return ListenerResult::Stop;});
        this->addEventListener(KeybindSettingPressedEventV3(Mod::get(),"jump-p2"),[layer](Keybind const&,bool down,bool repeat,double ts){if(repeat)return ListenerResult::Propagate;layer->queueButton(1,down,true,ts);return ListenerResult::Stop;});
        forwardBinding("place-checkpoint",KEY_Z); forwardBinding("delete-checkpoint",KEY_X); forwardBinding("restart-level",KEY_R);
        actionBinding("instant-gameplay",[this]{openTarget(false);});
        actionBinding("instant-practice",[this]{openTarget(true);});
        actionBinding("instant-level-select",[]{switchToScene(LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Type19,"")));});
        actionBinding("instant-my-levels",[]{switchToScene(LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MyLevels,"")));});
        actionBinding("instant-pause",[]{if(auto p=PlayLayer::get())p->pauseGame(true);});
        actionBinding("instant-main-menu",[]{switchToScene(MenuLayer::scene(false));});
        createSmartButtons(); return true;
    }
};

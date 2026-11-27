#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/LevelDownloadDelegate.hpp>
#include "SequenceRecorder.hpp"
#include "Localization.hpp"
#include "InstantActions.hpp"
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>

using namespace geode::prelude;

namespace {
bool s_startPracticeMode = false;

void enterTargetLevel(GJGameLevel* level, bool practice) {
    if (!level || level->m_levelString.empty() || level->m_levelNotDownloaded) {
        FLAlertLayer::create("New Keybinds", "The level data is not ready to play.", "OK")->show();
        return;
    }
    s_startPracticeMode = practice;
    auto info = LevelInfoLayer::create(level, false);
    if (!info) {
        s_startPracticeMode = false;
        FLAlertLayer::create("New Keybinds", "Could not create the level launch screen.", "OK")->show();
        return;
    }
    info->onPlay(nullptr);
}

class TargetLevelDownloadHandler : public CCNode, public LevelDownloadDelegate {
    int m_targetID = 0;
    bool m_practice = false;
    LevelDownloadDelegate* m_previous = nullptr;
public:
    static TargetLevelDownloadHandler* create(int id, bool practice, LevelDownloadDelegate* previous) {
        auto ret = new TargetLevelDownloadHandler();
        ret->m_targetID = id;
        ret->m_practice = practice;
        ret->m_previous = previous;
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
    bool init() override { return CCNode::init(); }
    void restoreDelegate() {
        auto manager = GameLevelManager::sharedState();
        if (manager && manager->m_levelDownloadDelegate == this)
            manager->m_levelDownloadDelegate = m_previous;
    }
    void levelDownloadFinished(GJGameLevel* downloaded) override {
        auto manager = GameLevelManager::sharedState();
        restoreDelegate();
        auto level = manager ? manager->getSavedLevel(m_targetID) : nullptr;
        if (!level || level->m_levelString.empty() || level->m_levelNotDownloaded)
            level = downloaded;
        if (!level || level->m_levelString.empty() || level->m_levelNotDownloaded) {
            FLAlertLayer::create("New Keybinds", "Geometry Dash returned level information without playable level data.", "OK")->show();
        } else {
            enterTargetLevel(level, m_practice);
        }
        this->removeFromParentAndCleanup(true);
    }
    void levelDownloadFailed(int levelID) override {
        restoreDelegate();
        auto message = "Could not download level " + std::to_string(levelID) + ". Check the ID and your connection.";
        FLAlertLayer::create("New Keybinds", message.c_str(), "OK")->show();
        this->removeFromParentAndCleanup(true);
    }
};

void launchTargetLevel(bool practice) {
    auto rawID = Mod::get()->getSettingValue<std::string>("instant-level-id");
    if (rawID.empty()) rawID = "6508283";
    if (!std::all_of(rawID.begin(), rawID.end(), [](unsigned char ch) { return std::isdigit(ch); })) {
        FLAlertLayer::create("New Keybinds", "Target Level ID must contain digits only.", "OK")->show();
        return;
    }
    int levelID = std::atoi(rawID.c_str());
    if (levelID <= 0) {
        FLAlertLayer::create("New Keybinds", "Enter a valid Target Level ID.", "OK")->show();
        return;
    }
    auto manager = GameLevelManager::sharedState();
    if (!manager) return;
    if (auto saved = manager->getSavedLevel(levelID);
        saved && !saved->m_levelString.empty() && !saved->m_levelNotDownloaded) {
        enterTargetLevel(saved, practice);
        return;
    }
    if (manager->m_levelDownloadDelegate) {
        FLAlertLayer::create("New Keybinds", "Another level download is already running. Try again when it finishes.", "OK")->show();
        return;
    }
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;
    auto handler = TargetLevelDownloadHandler::create(levelID, practice, manager->m_levelDownloadDelegate);
    if (!handler) return;
    scene->addChild(handler);
    manager->m_levelDownloadDelegate = handler;
    manager->downloadLevel(levelID, false, 0);
}
}

namespace nkb {
class SequencePlayback : public CCNode {
    std::vector<std::pair<long long, std::string>> m_steps;
    size_t m_index = 0;
    std::chrono::steady_clock::time_point m_start;
    bool m_p1 = false, m_p2 = false;
    void finish() {
        if (auto play = PlayLayer::get()) {
            if (m_p1) play->queueButton(1, false, false, 0.0);
            if (m_p2) play->queueButton(1, false, true, 0.0);
        }
        unscheduleUpdate();
        removeFromParentAndCleanup(true);
    }
    void apply(std::string const& action) {
        auto play = PlayLayer::get();
        if (!play) { finish(); return; }
        if (action == "P1 DOWN") { play->queueButton(1, true, false, 0.0); m_p1=true; }
        else if (action == "P1 UP") { play->queueButton(1, false, false, 0.0); m_p1=false; }
        else if (action == "P2 DOWN") { play->queueButton(1, true, true, 0.0); m_p2=true; }
        else if (action == "P2 UP") { play->queueButton(1, false, true, 0.0); m_p2=false; }
        else if (action == "Smart Controls P1 tap") { play->queueButton(1,true,false,0.0); play->queueButton(1,false,false,0.0); }
        else if (action == "Smart Controls P2 tap") { play->queueButton(1,true,true,0.0); play->queueButton(1,false,true,0.0); }
        else if (action == "restart-level DOWN") { play->resetLevel(); }
    }
public:
    bool init() override {
        if (!CCNode::init()) return false;
        auto raw=Mod::get()->getSavedValue<std::vector<std::string>>("instant-sequence-draft",{});
        for (auto const& line : raw) {
            auto sep=line.find(" ms | ");
            if(sep==std::string::npos) continue;
            try { auto ms=std::stoll(line.substr(0,sep)); m_steps.emplace_back(ms,line.substr(sep+6)); } catch (...) {}
        }
        if(m_steps.empty()) return false;
        m_start=std::chrono::steady_clock::now();
        scheduleUpdate();
        return true;
    }
    void update(float) override {
        if(!PlayLayer::get()) { finish(); return; }
        auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-m_start).count();
        while(m_index<m_steps.size() && elapsed>=m_steps[m_index].first) apply(m_steps[m_index++].second);
        if(m_index>=m_steps.size()) finish();
    }
    static SequencePlayback* create() {
        auto ret=new SequencePlayback();
        if(ret->init()) { ret->autorelease(); return ret; }
        delete ret; return nullptr;
    }
};
void replaySequence() {
    if(!PlayLayer::get()) { FLAlertLayer::create("New Keybinds","Start a level before replaying a saved sequence.","OK")->show(); return; }
    auto playback=SequencePlayback::create();
    if(!playback) { FLAlertLayer::create("New Keybinds","The saved sequence contains no replayable input events.","OK")->show(); return; }
    PlayLayer::get()->addChild(playback,2000);
}
}

namespace nkb {
void launchTargetLevel(bool practice) {
    ::launchTargetLevel(practice);
}

void showRecorderButton() {
    auto play = PlayLayer::get();
    if (!play || play->getChildByID("new-keybinds-floating-save-menu")) return;
    auto win = CCDirector::sharedDirector()->getWinSize();
    auto menu = CCMenu::create();
    menu->setID("new-keybinds-floating-save-menu");
    menu->setPosition({0.f, 0.f});
    auto sprite = ButtonSprite::create(tr("Save Steps").c_str(), "goldFont.fnt", "GJ_button_06.png", .58f);
    auto save = CCMenuItemExt::createSpriteExtra(sprite, [](CCObject* sender) {
        bool saved = saveRecording();
        if (auto parent = static_cast<CCNode*>(sender)->getParent())
            parent->setVisible(false);
        FLAlertLayer::create("New Keybinds",
            saved ? "Sequence saved. Open Instant Actions to review the recorded steps."
                  : "No recorded steps were captured.", "OK")->show();
    });
    save->setPosition({win.width * .5f, win.height - 30.f});
    menu->addChild(save);
    play->addChild(menu, 2000);
}
}

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

        if (nkb::isRecording()) nkb::showRecorderButton();
        return true;
    }
};

// Practice-mode launch is applied after Geometry Dash creates the level through its own Play action.
class $modify(NewKeybindsPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool dontCreateObjects) {
        if (!PlayLayer::init(level, dontCreateObjects)) return false;
        if (s_startPracticeMode) {
            s_startPracticeMode = false;
            this->togglePracticeMode(true);
        }
        return true;
    }
};

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

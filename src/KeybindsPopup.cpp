#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/GeodeUI.hpp>\n#include <Geode/binding/LevelBrowserLayer.hpp>\n#include <Geode/binding/GJSearchObject.hpp>\n#include <Geode/binding/MenuLayer.hpp>\n#include <Geode/binding/PlayLayer.hpp>\n#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;


// Prototype sequence workspace: persists a user-authored sequence draft.
// This workspace deliberately does not replay gameplay or automate level runs.
class InstantSequencePopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(360.f, 300.f)) return false;
        setTitle("Instant Actions - Sequence Draft");
        auto mod = Mod::get();
        auto steps = mod->getSavedValue<std::vector<std::string>>("instant-sequence-draft", {});
        auto info = CCLabelBMFont::create("Build and save a UI sequence draft", "goldFont.fnt");
        info->setScale(.42f);
        info->setPosition({m_size.width/2, m_size.height-48.f});
        m_mainLayer->addChild(info);
        auto list = CCLabelBMFont::create(steps.empty() ? "No steps yet" : "", "goldFont.fnt");
        list->setScale(.4f);
        list->setPosition({m_size.width/2, m_size.height-92.f});
        m_mainLayer->addChild(list);
        if (!steps.empty()) {
            std::string text;
            for (size_t i=0;i<steps.size();++i) text += std::to_string(i+1)+". "+steps[i]+"\n";
            list->setString(text.c_str());
        }
        auto menu=CCMenu::create();
        menu->setPosition({m_size.width/2, 65.f});
        const char* labels[]={"Add: Open Pause UI","Add: Open Level Browser","Add: Main Menu Context","Clear Draft"};
        for(int i=0;i<4;++i){
            auto item=CCMenuItemExt::createSpriteExtra(ButtonSprite::create(labels[i],"goldFont.fnt","GJ_button_01.png",.55f),[i](CCObject*){
                auto m=Mod::get();
                auto v=m->getSavedValue<std::vector<std::string>>("instant-sequence-draft",{});
                if(i==0) v.push_back("Open Pause UI (draft)");
                else if(i==1) v.push_back("Open Level Browser (draft)");
                else if(i==2) v.push_back("Main Menu context (draft)");
                else v.clear();
                m->setSavedValue("instant-sequence-draft",v);
                InstantSequencePopup::create()->show();
            });
            item->setPosition({0.f,-i*34.f});
            menu->addChild(item);
        }
        m_mainLayer->addChild(menu);
        auto note=CCLabelBMFont::create("Draft only: steps are stored, not replayed.", "goldFont.fnt");
        note->setScale(.33f); note->setPosition({m_size.width/2,20.f}); note->setColor({180,195,220});
        m_mainLayer->addChild(note);
        return true;
    }
public:
    static InstantSequencePopup* create(){auto ret=new InstantSequencePopup();if(ret->init()){ret->autorelease();return ret;}delete ret;return nullptr;}
};

class InstantActionPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(390.f, 315.f)) return false;
        setTitle("Instant Actions");

        auto subtitle = CCLabelBMFont::create("Select an action", "goldFont.fnt");
        subtitle->setScale(.48f);
        subtitle->setPosition({m_size.width / 2, m_size.height - 48.f});
        m_mainLayer->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2, m_size.height / 2 - 2.f});

        struct ActionEntry { char const* title; char const* texture; };
        ActionEntry const actions[] = {
            {"Gameplay", "GJ_button_01.png"},
            {"Practice", "GJ_button_02.png"},
            {"My Levels", "GJ_button_03.png"},
            {"Create Level", "GJ_button_04.png"},
            {"Edit Levels", "GJ_button_05.png"},
            {"Manage Levels", "GJ_button_06.png"},
            {"Pause Menu", "GJ_button_01.png"},
            {"Main Menu", "GJ_button_02.png"},
            {"Level Select", "GJ_button_03.png"}
        };

        for (int i = 0; i < 9; ++i) {
            auto sprite = ButtonSprite::create(actions[i].title, "goldFont.fnt", actions[i].texture, .62f);
            auto item = CCMenuItemExt::createSpriteExtra(sprite, [i](CCObject*) {
                // Gameplay and Practice both locate the requested online level directly.
                // Practice mode itself is selected after opening the level's info page.
                if (i == 0 || i == 1 || i == 8) {
                    auto search = GJSearchObject::create(
                        SearchType::Type19, "6508283&gameVersion=22"
                    );
                    switchToScene(LevelBrowserLayer::scene(search));
                    return;
                }
                if (i == 6) {
                    if (auto play = PlayLayer::get()) {
                        play->pauseGame(true);
                    } else {
                        FLAlertLayer::create("Instant Actions", "There is no active level to pause.", "OK")->show();
                    }
                    return;
                }
                if (i == 7) {
                    switchToScene(MenuLayer::scene(false));
                    return;
                }
                // Editor actions open the user's levels list, from which the editor can be entered.
                auto search = GJSearchObject::create(SearchType::MyLevels, "");
                switchToScene(LevelBrowserLayer::scene(search));
            });
            item->setPosition({(i % 3 - 1) * 116.f, (1 - i / 3) * 48.f});
            menu->addChild(item);
        }

        auto draftItem = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Sequence Draft", "goldFont.fnt", "GJ_button_01.png", .65f), [](CCObject*) { InstantSequencePopup::create()->show(); });
        draftItem->setPosition({0.f, -170.f});
        menu->addChild(draftItem);

        // Each item uses a real ButtonSprite with a bounded clickable area.\n        m_mainLayer->addChild(menu);

        auto note = CCLabelBMFont::create(
            "Gameplay and Practice open level search results", "goldFont.fnt"
        );
        note->setScale(.34f);
        note->setPosition({m_size.width / 2, 22.f});
        note->setColor({180, 195, 220});
        m_mainLayer->addChild(note);
        return true;
    }

public:
    static InstantActionPopup* create() {
        auto ret = new InstantActionPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class CustomKeybindsPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(360.f, 245.f)) return false;
        setTitle("Custom Keybinds");

        auto subtitle = CCLabelBMFont::create("Configure bindings in Geode's settings", "goldFont.fnt");
        subtitle->setScale(.42f);
        subtitle->setPosition({m_size.width / 2, m_size.height - 48.f});
        m_mainLayer->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2, m_size.height / 2 - 8.f});

        auto gameplay = ButtonSprite::create("Gameplay", "goldFont.fnt", "GJ_button_01.png", .7f);
        auto gameplayItem = CCMenuItemExt::createSpriteExtra(gameplay, [](CCObject*) {
            openKeybindsPopup(KeybindCategory::Gameplay, Mod::get());
        });
        gameplayItem->setPosition({0.f, 38.f});
        menu->addChild(gameplayItem);

        auto editor = ButtonSprite::create("Editor", "goldFont.fnt", "GJ_button_02.png", .7f);
        auto editorItem = CCMenuItemExt::createSpriteExtra(editor, [](CCObject*) {
            openKeybindsPopup(KeybindCategory::Editor, Mod::get());
        });
        editorItem->setPosition({0.f, -8.f});
        menu->addChild(editorItem);

        auto settings = ButtonSprite::create("All Mod Settings", "goldFont.fnt", "GJ_button_03.png", .7f);
        auto settingsItem = CCMenuItemExt::createSpriteExtra(settings, [](CCObject*) {
            openSettingsPopup(Mod::get());
        });
        settingsItem->setPosition({0.f, -54.f});
        menu->addChild(settingsItem);
        m_mainLayer->addChild(menu);

        auto note = CCLabelBMFont::create("Bindings are editable and persist through Geode", "goldFont.fnt");
        note->setScale(.32f);
        note->setPosition({m_size.width / 2, 22.f});
        note->setColor({180, 195, 220});
        m_mainLayer->addChild(note);
        return true;
    }
public:
    static CustomKeybindsPopup* create() {
        auto ret = new CustomKeybindsPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class SmartControlsPopup : public Popup {
    void refresh() { SmartControlsPopup::create()->show(); }
protected:
    bool init() override {
        if(!Popup::init(360.f,300.f)) return false;
        setTitle("Smart Controls");
        auto mod=Mod::get();
        auto info=CCLabelBMFont::create("Internal touch-control settings","goldFont.fnt");
        info->setScale(.42f); info->setPosition({m_size.width/2,m_size.height-42.f}); m_mainLayer->addChild(info);
        auto menu=CCMenu::create(); menu->setPosition({m_size.width/2, m_size.height/2-5.f});
        auto add=[&](std::string label,float y,std::function<void()> fn){
            auto item=CCMenuItemExt::createSpriteExtra(ButtonSprite::create(label.c_str(),"goldFont.fnt","GJ_button_01.png",.62f),[fn](CCObject*){fn();});
            item->setPosition({0,y});menu->addChild(item);
        };
        add(mod->getSettingValue<bool>("smart-controls-enabled")?"Smart Controls: ON":"Smart Controls: OFF",65,[mod]{
            mod->setSettingValue("smart-controls-enabled",!mod->getSettingValue<bool>("smart-controls-enabled"));
            FLAlertLayer::create("Smart Controls","Enable state changed. Re-enter a level to refresh.","OK")->show();
        });
        add(mod->getSettingValue<bool>("smart-p1-enabled")?"Player 1: ON":"Player 1: OFF",20,[mod]{
            mod->setSettingValue("smart-p1-enabled",!mod->getSettingValue<bool>("smart-p1-enabled"));
            FLAlertLayer::create("Smart Controls","Player 1 button toggled. Re-enter a level to refresh.","OK")->show();
        });
        add(mod->getSettingValue<bool>("smart-p2-enabled")?"Player 2: ON":"Player 2: OFF",-25,[mod]{
            mod->setSettingValue("smart-p2-enabled",!mod->getSettingValue<bool>("smart-p2-enabled"));
            FLAlertLayer::create("Smart Controls","Player 2 button toggled. Re-enter a level to refresh.","OK")->show();
        });
        auto adjust=[mod](const char* key,float delta,float min,float max){
            float v=mod->getSettingValue<float>(key);mod->setSettingValue(key,std::clamp(v+delta,min,max));
        };
        add("Button size +", -70,[adjust]{adjust("smart-button-size",.1f,.5f,2.f);});
        add("Button size -", -108,[adjust]{adjust("smart-button-size",-.1f,.5f,2.f);});
        add("Opacity +", -146,[adjust]{adjust("smart-button-opacity",.05f,.2f,1.f);});
        add("Opacity -", -184,[adjust]{adjust("smart-button-opacity",-.05f,.2f,1.f);});
        m_mainLayer->addChild(menu);
        return true;
    }
public:
    static SmartControlsPopup* create(){auto ret=new SmartControlsPopup();if(ret->init()){ret->autorelease();return ret;}delete ret;return nullptr;}
};

class GlobalProfilesPopup : public Popup {
    static std::vector<std::string> keys() {
        return {"smart-controls-enabled","smart-p1-enabled","smart-p2-enabled",
                "smart-button-size","smart-button-opacity","smart-p1-x","smart-p1-y",
                "smart-p2-x","smart-p2-y","instant-level-id"};
    }
    static void saveSlot(int slot) {
        auto m=Mod::get(); std::vector<std::string> data;
        for(auto const& k:keys()) {
            if(k=="instant-level-id") data.push_back(m->getSettingValue<std::string>(k));
            else if(k.find("-enabled")!=std::string::npos) data.push_back(m->getSettingValue<bool>(k)?"1":"0");
            else data.push_back(std::to_string(m->getSettingValue<float>(k)));
        }
        m->setSavedValue("profile-"+std::to_string(slot),data);
    }
    static void applySlot(int slot) {
        auto m=Mod::get(); auto data=m->getSavedValue<std::vector<std::string>>("profile-"+std::to_string(slot),{});
        auto k=keys(); if(data.size()!=k.size()) {FLAlertLayer::create("Profiles","This profile slot is empty.","OK")->show();return;}
        for(size_t i=0;i<k.size();++i) {
            if(k[i]=="instant-level-id") m->setSettingValue(k[i],data[i]);
            else if(k[i].find("-enabled")!=std::string::npos) m->setSettingValue(k[i],data[i]=="1");
            else { try {m->setSettingValue(k[i],std::stof(data[i]));}catch(...){} }
        }
        FLAlertLayer::create("Profiles","Profile applied. Re-enter gameplay to refresh Smart Controls.","OK")->show();
    }
protected:
    bool init() override {
        if(!Popup::init(360.f,290.f)) return false;
        setTitle("Global Profiles");
        auto info=CCLabelBMFont::create("Save or apply shared-device control layouts","goldFont.fnt");
        info->setScale(.38f); info->setPosition({m_size.width/2,m_size.height-42});m_mainLayer->addChild(info);
        auto menu=CCMenu::create(); menu->setPosition({m_size.width/2,80});
        for(int s=1;s<=3;s++) {
            auto label="Profile "+std::to_string(s);
            auto save=CCMenuItemExt::createSpriteExtra(ButtonSprite::create(("Save "+label).c_str(),"goldFont.fnt","GJ_button_01.png",.55f),[s](CCObject*){saveSlot(s);FLAlertLayer::create("Profiles","Profile saved.","OK")->show();});
            auto load=CCMenuItemExt::createSpriteExtra(ButtonSprite::create(("Apply "+label).c_str(),"goldFont.fnt","GJ_button_02.png",.55f),[s](CCObject*){applySlot(s);});
            save->setPosition({-90.f,30.f-(s-1)*58.f});load->setPosition({90.f,30.f-(s-1)*58.f});menu->addChild(save);menu->addChild(load);
        }
        m_mainLayer->addChild(menu);
        auto note=CCLabelBMFont::create("Slots store Smart Controls and selected scalar settings","goldFont.fnt");
        note->setScale(.32f);note->setPosition({m_size.width/2,18});note->setColor({180,195,220});m_mainLayer->addChild(note);
        return true;
    }
public:
    static GlobalProfilesPopup* create(){auto r=new GlobalProfilesPopup();if(r->init()){r->autorelease();return r;}delete r;return nullptr;}
};

class NewKeybindsHubPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(320.f, 235.f)) return false;
        setTitle("New Keybinds");

        auto subtitle = CCLabelBMFont::create("Choose a section", "goldFont.fnt");
        subtitle->setScale(.42f);
        subtitle->setPosition({m_size.width / 2, m_size.height - 52.f});
        m_mainLayer->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2, m_size.height / 2 - 5.f});

        auto customLabel = CCLabelBMFont::create("Custom Keybinds", "goldFont.fnt");
        customLabel->setScale(.55f);
        auto customButton = CCMenuItemExt::createSpriteExtra(customLabel, [](CCObject*) {
            CustomKeybindsPopup::create()->show();
        });
        customButton->setPosition({0.f, 25.f});
        menu->addChild(customButton);

        auto instantLabel = CCLabelBMFont::create("Instant Actions", "goldFont.fnt");
        instantLabel->setScale(.55f);
        auto instantButton = CCMenuItemExt::createSpriteExtra(instantLabel, [](CCObject*) {
            InstantActionPopup::create()->show();
        });
        instantButton->setPosition({0.f, -25.f});
        menu->addChild(instantButton);

        auto smartLabel = CCLabelBMFont::create("Smart Controls", "goldFont.fnt");
        smartLabel->setScale(.55f);
        auto smartButton = CCMenuItemExt::createSpriteExtra(smartLabel, [](CCObject*) {
            SmartControlsPopup::create()->show();
        });
        smartButton->setPosition({0.f, -75.f});
        menu->addChild(smartButton);

        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static NewKeybindsHubPopup* create() {
        auto ret = new NewKeybindsHubPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// Add one New Keybinds shortcut to Geode's Mods Library action column.
// ModItemUIEvent is used instead of modifying Geometry Dash's main menu.
$execute {
    static ListenerHandle listener = ModItemUIEvent().listen(
        [](CCNode* item, std::string_view modID, std::optional<Mod*>) {
            if (modID != "sfj3861.new-keybinds" || !item) {
                return ListenerResult::Propagate;
            }

            CCNode* modsLayer = item;
            while (modsLayer && modsLayer->getID() != "ModsLayer") {
                modsLayer = modsLayer->getParent();
            }
            if (!modsLayer) return ListenerResult::Propagate;

            auto actionsMenu = modsLayer->getChildByID("right-actions-menu");
            if (!actionsMenu || actionsMenu->getChildByID("new-keybinds-library-button")) {
                return ListenerResult::Propagate;
            }

            auto logo = createModLogo(Mod::get());
            if (!logo) return ListenerResult::Propagate;

            auto button = CCMenuItemExt::createSpriteExtra(logo, [](CCObject*) {
                NewKeybindsHubPopup::create()->show();
            });
            button->setID("new-keybinds-library-button");
            button->setScale(.8f);
            actionsMenu->addChild(button);
            actionsMenu->updateLayout();

            return ListenerResult::Propagate;
        }
    );
}

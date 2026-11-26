#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/CCDirector.hpp>
using namespace geode::prelude;

class InstantActionPopup : public geode::Popup {
protected:
 bool init() override {
  if (!Popup::init(380.f,300.f)) return false;
  setTitle("Instant Action");
  auto c=m_mainLayer;
  auto h=CCLabelBMFont::create("Select a capture context","goldFont.fnt");
  h->setScale(.5f); h->setPosition({m_size.width/2,m_size.height-48.f}); c->addChild(h);
  auto info=CCLabelBMFont::create("Choose a context to configure capture.","goldFont.fnt");
  info->setScale(.38f); info->setPosition({m_size.width/2,55.f}); info->setColor({180,200,255}); c->addChild(info);
  const char* contexts[]={"Gameplay","Practice","Editor","Editor - Create","Editor - Edit","Editor - Delete","Pause Menu","Main Menu","Level Select"};
  auto menu=CCMenu::create(); menu->setPosition({m_size.width/2,m_size.height/2-5.f}); menu->setContentSize({m_size.width-40.f,150.f});
  for (int i=0;i<9;i++) {
   auto label=CCLabelBMFont::create(contexts[i],"goldFont.fnt"); label->setScale(.42f);
   auto item=CCMenuItemExt::createSpriteExtra(label,[info,context=std::string(contexts[i])](CCObject*) {
    info->setString(("Selected context: "+context+"\nCapture implementation is a foundation milestone.").c_str());
   });
   item->setContentSize({110.f,30.f});
   item->setPosition({(i%3)*115.f-115.f,(1-i/3)*40.f});
   menu->addChild(item);
  }
  c->addChild(menu);
  auto note=CCLabelBMFont::create("Context selection UI foundation - action execution not yet implemented","goldFont.fnt");
  note->setScale(.30f); note->setPosition({m_size.width/2,20.f}); note->setColor({160,160,160}); c->addChild(note);
  return true;
 }
public:
 static InstantActionPopup* create(){auto r=new InstantActionPopup();if(r->init()){r->autorelease();return r;}delete r;return nullptr;}
};

class NewKeybindsPopup : public geode::Popup {
protected:
 bool init() override {
  if (!Popup::init(360.f,280.f)) return false;
  setTitle("New Keybinds");
  auto c=m_mainLayer;
  auto h=CCLabelBMFont::create("Keyboard controls","goldFont.fnt");
  h->setScale(.55f); h->setPosition({m_size.width/2,m_size.height-48.f}); c->addChild(h);
  auto section=[&](const char*t,const char*d,float y){
   auto l=CCLabelBMFont::create(t,"bigFont.fnt"); l->setScale(.45f); l->setAnchorPoint({0.f,.5f}); l->setPosition({24.f,y}); c->addChild(l);
   auto i=CCLabelBMFont::create(d,"goldFont.fnt"); i->setScale(.38f); i->setAnchorPoint({0.f,.5f}); i->setPosition({24.f,y-22.f}); i->setColor({190,190,190}); c->addChild(i);
  };
  section("Gameplay","P1 / P2, practice checkpoints, restart",190.f);
  section("Editor","Selection movement and rotation",130.f);
  section("Instant Action","Context capture foundation (Alpha-2)",70.f);
  auto ia=CCLabelBMFont::create("Open Instant Action","goldFont.fnt"); ia->setScale(.42f);
  auto item=CCMenuItemExt::createSpriteExtra(ia,[](CCObject*){InstantActionPopup::create()->show();});
  auto menu=CCMenu::create();menu->addChild(item);menu->setPosition({m_size.width/2,30.f});c->addChild(menu);
  auto dev=CCLabelBMFont::create("Smart Controls planned for Beta","goldFont.fnt"); dev->setScale(.34f); dev->setPosition({m_size.width/2,12.f}); dev->setColor({160,190,255}); c->addChild(dev);
  return true;
 }
public:
 static NewKeybindsPopup* create(){auto r=new NewKeybindsPopup();if(r->init()){r->autorelease();return r;}delete r;return nullptr;}
};
#include <Geode/modify/MenuLayer.hpp>

class $modify(NewKeybindsMenuLayer, MenuLayer) {
 bool init() {
  if (!MenuLayer::init()) return false;
  auto menu = this->getChildByID("bottom-menu");
  if (!menu || menu->getChildByID("new-keybinds-entry")) return true;
  auto label = CCLabelBMFont::create("New Keybinds", "goldFont.fnt");
  label->setScale(.45f);
  auto item = CCMenuItemExt::createSpriteExtra(label, [](CCObject*) {
   NewKeybindsPopup::create()->show();
  });
  item->setID("new-keybinds-entry");
  menu->addChild(item);
  menu->updateLayout();
  return true;
 }
};

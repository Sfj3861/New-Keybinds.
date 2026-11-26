#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/CCDirector.hpp>
using namespace geode::prelude;

class NewKeybindsPopup : public Popup<> {
protected:
 bool init() override {
  if (!Popup::init(360.f,260.f)) return false;
  setTitle("New Keybinds");
  auto c=m_mainLayer;
  auto h=CCLabelBMFont::create("Keyboard controls","goldFont.fnt");
  h->setScale(.55f); h->setPosition({m_size.width/2,m_size.height-48.f}); c->addChild(h);
  auto section=[&](const char*t,const char*d,float y){
   auto l=CCLabelBMFont::create(t,"bigFont.fnt"); l->setScale(.45f); l->setAnchorPoint({0.f,.5f}); l->setPosition({24.f,y}); c->addChild(l);
   auto i=CCLabelBMFont::create(d,"goldFont.fnt"); i->setScale(.38f); i->setAnchorPoint({0.f,.5f}); i->setPosition({24.f,y-22.f}); i->setColor({190,190,190}); c->addChild(i);
  };
  section("Gameplay","P1 / P2, practice checkpoints, restart",175.f);
  section("Editor","Selection movement and rotation",112.f);
  section("Navigation","Navigation shortcuts (planned)",49.f);
  auto dev=CCLabelBMFont::create("Controller and Smart Controls planned","goldFont.fnt"); dev->setScale(.36f); dev->setPosition({m_size.width/2,20.f}); dev->setColor({160,190,255}); c->addChild(dev);
  return true;
 }
public:
 static NewKeybindsPopup* create(){auto r=new NewKeybindsPopup();if(r->init()){r->autorelease();return r;}delete r;return nullptr;}
};
static void addNewKeybindsButton(CCScene* scene){
 auto layer=scene?scene->getChildByID("ModsLayer"):nullptr;
 auto menu=layer?layer->getChildByID("actions-menu"):nullptr;
 if(!menu||menu->getChildByID("new-keybinds-entry"))return;
 auto label=CCLabelBMFont::create("New Keybinds","goldFont.fnt");label->setScale(.5f);
 auto item=CCMenuItemExt::createSpriteExtra(label,[](CCObject*){NewKeybindsPopup::create()->show();});
 item->setID("new-keybinds-entry");menu->addChild(item);menu->updateLayout();
}
class $modify(NewKeybindsDirector,CCDirector){
 void setNextScene(){CCDirector::setNextScene();addNewKeybindsButton(getRunningScene());}
};

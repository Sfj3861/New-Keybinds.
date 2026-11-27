#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace {
struct BindingEntry {
    char const* id;
    char const* label;
    char const* page;
    char const* context;
};

constexpr std::array<BindingEntry, 17> kBindingEntries {{
    {"jump-p1", "Jump - Player 1", "Gameplay", "gameplay"},
    {"jump-p2", "Jump - Player 2", "Gameplay", "gameplay"},
    {"place-checkpoint", "Place Practice Checkpoint", "Gameplay", "gameplay"},
    {"delete-checkpoint", "Delete Practice Checkpoint", "Gameplay", "gameplay"},
    {"restart-level", "Restart Level", "Gameplay", "gameplay"},
    {"instant-gameplay", "Find Target Level (Gameplay)", "Gameplay", "gameplay"},
    {"instant-practice", "Find Target Level (Practice)", "Gameplay", "gameplay"},
    {"move-obj-up", "Move Selection Up", "Editor", "editor"},
    {"move-obj-left", "Move Selection Left", "Editor", "editor"},
    {"move-obj-down", "Move Selection Down", "Editor", "editor"},
    {"move-obj-right", "Move Selection Right", "Editor", "editor"},
    {"rotate-ccw", "Rotate 90 Degrees Left", "Editor", "editor"},
    {"rotate-cw", "Rotate 90 Degrees Right", "Editor", "editor"},
    {"instant-my-levels", "Open My Levels", "Editor", "editor"},
    {"instant-level-select", "Open Online Level Browser", "Navigation", "universal"},
    {"instant-pause", "Open Pause Menu", "Navigation", "gameplay"},
    {"instant-main-menu", "Open Main Menu", "Navigation", "universal"},
}};

bool isModifierKey(enumKeyCodes key) {
    return key == KEY_Shift || key == KEY_Control || key == KEY_Alt ||
        key == KEY_LeftShift || key == KEY_RightShift ||
        key == KEY_LeftControl || key == KEY_RightContol ||
        key == KEY_LeftMenu || key == KEY_RightMenu;
}

BindingEntry const* findEntry(std::string_view id) {
    auto it = std::find_if(kBindingEntries.begin(), kBindingEntries.end(),
        [id](BindingEntry const& entry) { return id == entry.id; });
    return it == kBindingEntries.end() ? nullptr : &*it;
}

std::string bindingText(std::string_view id) {
    auto bindings = Mod::get()->getSettingValue<std::vector<Keybind>>(id);
    if (bindings.empty()) return "Unassigned";

    std::string result;
    for (auto const& binding : bindings) {
        if (!result.empty()) result += " / ";
        result += binding.toString();
    }
    return result;
}

void setBindings(std::string_view id, std::vector<Keybind> const& bindings) {
    if (auto setting = cast::typeinfo_pointer_cast<KeybindSettingV3>(Mod::get()->getSetting(id))) {
        setting->setValue(bindings);
    }
}

void addButton(CCMenu* menu, std::string const& text, CCPoint position,
               std::function<void()> callback, float scale = .48f,
               char const* texture = "GJ_button_01.png") {
    auto sprite = ButtonSprite::create(text.c_str(), "goldFont.fnt", texture, scale);
    auto item = CCMenuItemExt::createSpriteExtra(sprite,
        [callback = std::move(callback)](CCObject*) { callback(); });
    item->setPosition(position);
    menu->addChild(item);
}

std::string makeProfileSnapshot() {
    auto mod = Mod::get();
    auto state = matjson::Value::object();

    // Serialize the actual current value of every registered setting.
    // This automatically includes future settings added to mod.json.
    for (auto const& key : mod->getSettingKeys()) {
        auto setting = mod->getSetting(key);
        if (!setting) continue;

        matjson::Value value;
        if (setting->save(value)) state.set(key, std::move(value));
    }
    return state.dump();
}

bool applyProfileSnapshot(std::string const& serialized) {
    auto parsed = matjson::parse(serialized);
    if (!parsed) return false;

    auto state = parsed.unwrap();
    auto mod = Mod::get();

    for (auto const& key : mod->getSettingKeys()) {
        auto setting = mod->getSetting(key);
        if (!setting) continue;

        auto stored = state.get(key);
        if (!stored) continue;
        auto const& value = stored.unwrap();

        // Typed setters fire Geode's setting-changed events and persist values.
        if (auto bind = cast::typeinfo_pointer_cast<KeybindSettingV3>(setting)) {
            auto result = value.as<std::vector<Keybind>>();
            if (result) bind->setValue(result.unwrap());
        }
        else if (auto boolean = cast::typeinfo_pointer_cast<BoolSettingV3>(setting)) {
            auto result = value.as<bool>();
            if (result) boolean->setValue(result.unwrap());
        }
        else if (auto number = cast::typeinfo_pointer_cast<FloatSettingV3>(setting)) {
            auto result = value.as<double>();
            if (result) number->setValue(result.unwrap());
        }
        else if (auto integer = cast::typeinfo_pointer_cast<IntSettingV3>(setting)) {
            auto result = value.as<int64_t>();
            if (result) integer->setValue(result.unwrap());
        }
        else if (auto string = cast::typeinfo_pointer_cast<StringSettingV3>(setting)) {
            auto result = value.as<std::string>();
            if (result) string->setValue(result.unwrap());
        }
        else {
            setting->load(value);
        }
    }
    return true;
}

class KeybindPagePopup : public Popup {
    std::string m_page;
    std::string m_captureID;
    bool m_replace = false;
    CCLabelBMFont* m_status = nullptr;
    std::vector<std::pair<std::string, CCLabelBMFont*>> m_valueLabels;

    std::vector<BindingEntry const*> pageEntries() const {
        std::vector<BindingEntry const*> result;
        for (auto const& entry : kBindingEntries) {
            if (m_page == entry.page) result.push_back(&entry);
        }
        return result;
    }

    std::string conflictFor(Keybind const& binding, std::string_view currentID) const {
        auto* current = findEntry(currentID);
        if (!current) return {};

        for (auto const& entry : kBindingEntries) {
            if (std::string_view(entry.id) == currentID ||
                std::string_view(entry.context) != current->context) continue;

            auto assigned = Mod::get()->getSettingValue<std::vector<Keybind>>(entry.id);
            if (std::find(assigned.begin(), assigned.end(), binding) != assigned.end())
                return entry.label;
        }
        return {};
    }

    void updateValues() {
        for (auto const& [id, label] : m_valueLabels) {
            auto value = bindingText(id);
            label->setString(value.c_str());
        }
    }

    void startCapture(std::string const& id) {
        m_captureID = id;
        auto entry = findEntry(id);
        std::string prompt = "Press a key for ";
        prompt += entry ? entry->label : id;
        prompt += ". Escape cancels.";
        m_status->setString(prompt.c_str());
    }

    bool onKeyboardInput(KeyboardInputData& data) {
        if (m_captureID.empty()) return false;

        // Eat the capture event so it cannot also trigger gameplay/editor actions.
        if (data.action != KeyboardInputData::Action::Press) return true;

        if (data.key == KEY_Escape) {
            m_captureID.clear();
            m_status->setString("Capture cancelled.");
            return true;
        }

        if (data.key == KEY_None || data.key == KEY_Unknown || isModifierKey(data.key))
            return true;

        Keybind pressed(data.key, data.modifiers);
        auto conflict = conflictFor(pressed, m_captureID);
        if (!conflict.empty()) {
            auto message = "Already assigned to " + conflict + ". Choose another key or clear that assignment.";
            m_status->setString(message.c_str());
            return true;
        }

        auto mod = Mod::get();
        auto bindings = mod->getSettingValue<std::vector<Keybind>>(m_captureID);
        if (m_replace) bindings.clear();

        if (std::find(bindings.begin(), bindings.end(), pressed) == bindings.end()) {
            if (bindings.size() >= 6) {
                m_status->setString("A maximum of 6 keys is allowed per action.");
                m_captureID.clear();
                return true;
            }
            bindings.push_back(pressed);
        }

        setBindings(m_captureID, bindings);
        auto assigned = pressed.toString();
        m_captureID.clear();
        updateValues();
        auto message = "Saved: " + assigned;
        m_status->setString(message.c_str());
        return true;
    }

protected:
    bool init() override {
        if (!Popup::init(450.f, 380.f)) return false;
        setTitle(("Custom Keybinds - " + m_page).c_str());

        auto info = CCLabelBMFont::create(
            "Press Set, then press a physical key. Modifiers such as Ctrl/Shift are captured.",
            "goldFont.fnt");
        info->setScale(.31f);
        info->setPosition({m_size.width / 2.f, m_size.height - 38.f});
        m_mainLayer->addChild(info);

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});

        addButton(menu, "ADD / REPLACE", {m_size.width - 91.f, m_size.height - 68.f},
            [this] {
                m_replace = !m_replace;
                m_status->setString(m_replace ?
                    "REPLACE mode: the next key replaces this action's keys." :
                    "ADD mode: the next key is appended to this action.");
            }, .38f, "GJ_button_02.png");

        auto entries = pageEntries();
        float startY = m_size.height - 105.f;
        for (size_t i = 0; i < entries.size(); ++i) {
            auto const& entry = *entries[i];
            float y = startY - static_cast<float>(i) * 39.f;

            auto name = CCLabelBMFont::create(entry.label, "goldFont.fnt");
            name->setAnchorPoint({0.f, .5f});
            name->setScale(.32f);
            name->setPosition({16.f, y});
            m_mainLayer->addChild(name);

            auto current = CCLabelBMFont::create(bindingText(entry.id).c_str(), "goldFont.fnt");
            current->setAnchorPoint({0.f, .5f});
            current->setScale(.25f);
            current->setPosition({16.f, y - 14.f});
            m_mainLayer->addChild(current);
            m_valueLabels.emplace_back(entry.id, current);

            addButton(menu, "Set", {330.f, y - 2.f},
                [this, id = std::string(entry.id)] { startCapture(id); }, .40f);
            addButton(menu, "Clear", {399.f, y - 2.f},
                [this, id = std::string(entry.id)] {
                    setBindings(id, {});
                    if (m_captureID == id) m_captureID.clear();
                    updateValues();
                    m_status->setString("All keys for this action were cleared.");
                }, .35f, "GJ_button_06.png");
        }

        m_status = CCLabelBMFont::create("Ready. ADD mode is active.", "goldFont.fnt");
        m_status->setScale(.30f);
        m_status->setPosition({m_size.width / 2.f, 43.f});
        m_status->setColor({180, 220, 255});
        m_mainLayer->addChild(m_status);

        auto note = CCLabelBMFont::create(
            "Duplicate keys in the same context are blocked. Up to 6 bindings per action.",
            "goldFont.fnt");
        note->setScale(.27f);
        note->setPosition({m_size.width / 2.f, 20.f});
        note->setColor({170, 180, 195});
        m_mainLayer->addChild(note);

        m_mainLayer->addChild(menu);
        this->addEventListener(KeyboardInputEvent(),
            [this](KeyboardInputData& data) { return onKeyboardInput(data); });
        return true;
    }

public:
    static KeybindPagePopup* create(std::string page) {
        auto ret = new KeybindPagePopup();
        ret->m_page = std::move(page);
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class CustomKeybindsPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(370.f, 280.f)) return false;
        setTitle("Custom Keybinds");

        auto info = CCLabelBMFont::create(
            "Configure the controls here. No Geode settings popup is needed.",
            "goldFont.fnt");
        info->setScale(.35f);
        info->setPosition({m_size.width / 2.f, m_size.height - 45.f});
        m_mainLayer->addChild(info);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2.f, m_size.height / 2.f - 4.f});
        addButton(menu, "Gameplay", {0.f, 54.f},
            [] { KeybindPagePopup::create("Gameplay")->show(); }, .68f);
        addButton(menu, "Editor", {0.f, 5.f},
            [] { KeybindPagePopup::create("Editor")->show(); }, .68f, "GJ_button_02.png");
        addButton(menu, "Navigation", {0.f, -44.f},
            [] { KeybindPagePopup::create("Navigation")->show(); }, .68f, "GJ_button_03.png");

        auto note = CCLabelBMFont::create(
            "Multiple keys, Ctrl/Shift/Alt combinations, conflict detection and saving.",
            "goldFont.fnt");
        note->setScale(.28f);
        note->setPosition({m_size.width / 2.f, 20.f});
        note->setColor({180, 195, 220});
        m_mainLayer->addChild(note);
        m_mainLayer->addChild(menu);
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
    CCLabelBMFont* m_status = nullptr;

    void adjust(std::string const& key, float delta, float min, float max) {
        auto mod = Mod::get();
        auto value = mod->getSettingValue<float>(key);
        mod->setSettingValue<float>(key, std::clamp(value + delta, min, max));
        auto message = key + " = " + std::to_string(mod->getSettingValue<float>(key));
        m_status->setString(message.c_str());
    }

protected:
    bool init() override {
        if (!Popup::init(440.f, 390.f)) return false;
        setTitle("Smart Controls");

        auto mod = Mod::get();
        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});

        auto add = [&](std::string const& label, float x, float y, std::function<void()> action) {
            addButton(menu, label, {x, y}, std::move(action), .40f);
        };

        add(mod->getSettingValue<bool>("smart-controls-enabled") ?
            "Smart Controls: ON" : "Smart Controls: OFF", 220.f, 340.f, [this, mod] {
                auto enabled = !mod->getSettingValue<bool>("smart-controls-enabled");
                mod->setSettingValue<bool>("smart-controls-enabled", enabled);
                m_status->setString(enabled ? "Smart Controls enabled." : "Smart Controls disabled.");
            });
        add(mod->getSettingValue<bool>("smart-p1-enabled") ? "P1: ON" : "P1: OFF",
            110.f, 294.f, [this, mod] {
                auto enabled = !mod->getSettingValue<bool>("smart-p1-enabled");
                mod->setSettingValue<bool>("smart-p1-enabled", enabled);
                m_status->setString(enabled ? "Player 1 button enabled." : "Player 1 button disabled.");
            });
        add(mod->getSettingValue<bool>("smart-p2-enabled") ? "P2: ON" : "P2: OFF",
            330.f, 294.f, [this, mod] {
                auto enabled = !mod->getSettingValue<bool>("smart-p2-enabled");
                mod->setSettingValue<bool>("smart-p2-enabled", enabled);
                m_status->setString(enabled ? "Player 2 button enabled." : "Player 2 button disabled.");
            });

        add("Size -", 110.f, 245.f, [this] { adjust("smart-button-size", -.1f, .5f, 2.f); });
        add("Size +", 330.f, 245.f, [this] { adjust("smart-button-size", .1f, .5f, 2.f); });
        add("Opacity -", 110.f, 200.f, [this] { adjust("smart-button-opacity", -.05f, .2f, 1.f); });
        add("Opacity +", 330.f, 200.f, [this] { adjust("smart-button-opacity", .05f, .2f, 1.f); });

        auto offset = [this, mod](std::string key, float delta) {
            auto value = mod->getSettingValue<float>(key);
            mod->setSettingValue<float>(key, std::clamp(value + delta, -500.f, 500.f));
            auto message = key + " = " + std::to_string(mod->getSettingValue<float>(key));
            m_status->setString(message.c_str());
        };

        add("P1 X -", 75.f, 150.f, [offset] { offset("smart-p1-x", -10.f); });
        add("P1 X +", 165.f, 150.f, [offset] { offset("smart-p1-x", 10.f); });
        add("P1 Y -", 255.f, 150.f, [offset] { offset("smart-p1-y", -10.f); });
        add("P1 Y +", 345.f, 150.f, [offset] { offset("smart-p1-y", 10.f); });
        add("P2 X -", 75.f, 105.f, [offset] { offset("smart-p2-x", -10.f); });
        add("P2 X +", 165.f, 105.f, [offset] { offset("smart-p2-x", 10.f); });
        add("P2 Y -", 255.f, 105.f, [offset] { offset("smart-p2-y", -10.f); });
        add("P2 Y +", 345.f, 105.f, [offset] { offset("smart-p2-y", 10.f); });

        m_status = CCLabelBMFont::create(
            "Touch controls are recreated when the next level loads.",
            "goldFont.fnt");
        m_status->setScale(.29f);
        m_status->setPosition({m_size.width / 2.f, 45.f});
        m_status->setColor({180, 205, 230});
        m_mainLayer->addChild(m_status);

        auto note = CCLabelBMFont::create(
            "Current buttons send taps (press + release), not hold gestures.",
            "goldFont.fnt");
        note->setScale(.27f);
        note->setPosition({m_size.width / 2.f, 22.f});
        note->setColor({170, 180, 195});
        m_mainLayer->addChild(note);

        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static SmartControlsPopup* create() {
        auto ret = new SmartControlsPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class GlobalProfilesPopup : public Popup {
    TextInput* m_nameInput = nullptr;
    std::string m_selected;
    int m_page = 0;
    CCLabelBMFont* m_status = nullptr;

    std::vector<std::string> profileIDs() const {
        return Mod::get()->getSavedValue<std::vector<std::string>>("profile-ids", {});
    }

    std::string profileName(std::string const& id) const {
        return Mod::get()->getSavedValue<std::string>("profile-name-" + id, "Unnamed");
    }

    bool hasSelection() const {
        auto ids = profileIDs();
        return !m_selected.empty() &&
            std::find(ids.begin(), ids.end(), m_selected) != ids.end();
    }

    std::string nextID() {
        auto mod = Mod::get();
        int next = mod->getSavedValue<int>("profile-next-id", 1);
        mod->setSavedValue("profile-next-id", next + 1);
        return std::to_string(next);
    }

    void createProfile() {
        auto name = m_nameInput ? m_nameInput->getString() : "";
        if (name.empty()) { m_status->setString("Enter a profile name first."); return; }

        auto mod = Mod::get();
        auto id = nextID();
        auto ids = profileIDs();
        ids.push_back(id);
        mod->setSavedValue("profile-ids", ids);
        mod->setSavedValue("profile-name-" + id, name);
        mod->setSavedValue("profile-data-" + id, makeProfileSnapshot());
        m_selected = id;
        m_status->setString("Profile created and saved. Reopen this panel to refresh the list.");
    }

    void saveSelected() {
        if (!hasSelection()) { m_status->setString("Select a profile first."); return; }
        Mod::get()->setSavedValue("profile-data-" + m_selected, makeProfileSnapshot());
        m_status->setString("All current mod settings saved to this profile.");
    }

    void loadSelected() {
        if (!hasSelection()) { m_status->setString("Select a profile first."); return; }
        auto data = Mod::get()->getSavedValue<std::string>("profile-data-" + m_selected, "");
        if (data.empty() || !applyProfileSnapshot(data)) {
            m_status->setString("The selected profile is empty or invalid.");
            return;
        }
        m_status->setString("Profile applied, including keybinds and Smart Controls.");
    }

    void duplicateSelected() {
        if (!hasSelection()) { m_status->setString("Select a profile first."); return; }
        auto name = m_nameInput ? m_nameInput->getString() : "";
        if (name.empty()) { m_status->setString("Enter a name for the duplicate first."); return; }

        auto mod = Mod::get();
        auto id = nextID();
        auto ids = profileIDs();
        ids.push_back(id);
        mod->setSavedValue("profile-ids", ids);
        mod->setSavedValue("profile-name-" + id, name);
        mod->setSavedValue("profile-data-" + id,
            mod->getSavedValue<std::string>("profile-data-" + m_selected, ""));
        m_selected = id;
        m_status->setString("Profile duplicated. Reopen this panel to refresh the list.");
    }

    void renameSelected() {
        if (!hasSelection()) { m_status->setString("Select a profile first."); return; }
        auto name = m_nameInput ? m_nameInput->getString() : "";
        if (name.empty()) { m_status->setString("Enter the new profile name first."); return; }
        Mod::get()->setSavedValue("profile-name-" + m_selected, name);
        m_status->setString("Profile renamed. Reopen this panel to refresh the list.");
    }

    void deleteSelected() {
        if (!hasSelection()) { m_status->setString("Select a profile first."); return; }
        auto mod = Mod::get();
        auto ids = profileIDs();
        ids.erase(std::remove(ids.begin(), ids.end(), m_selected), ids.end());
        mod->setSavedValue("profile-ids", ids);
        mod->setSavedValue("profile-data-" + m_selected, std::string{});
        mod->setSavedValue("profile-name-" + m_selected, std::string{});
        m_selected.clear();
        m_status->setString("Profile deleted. Reopen this panel to refresh the list.");
    }

protected:
    bool init() override {
        if (!Popup::init(470.f, 400.f)) return false;
        setTitle("Global Profiles");

        auto mod = Mod::get();
        m_selected = mod->getSavedValue<std::string>("profiles-selected", "");
        m_page = std::max(0, mod->getSavedValue<int>("profiles-page", 0));

        m_nameInput = TextInput::create(245.f, "Profile name / duplicate name");
        m_nameInput->setMaxCharCount(28);
        m_nameInput->setPosition({m_size.width / 2.f, m_size.height - 48.f});
        m_mainLayer->addChild(m_nameInput);

        auto ids = profileIDs();
        int pageCount = std::max(1, static_cast<int>((ids.size() + 3) / 4));
        m_page = std::min(m_page, pageCount - 1);

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});

        float listTop = m_size.height - 105.f;
        size_t first = static_cast<size_t>(m_page * 4);
        for (size_t row = 0; row < 4; ++row) {
            size_t index = first + row;
            if (index >= ids.size()) break;

            auto id = ids[index];
            auto name = profileName(id);
            if (id == m_selected) name = "> " + name;
            addButton(menu, name, {m_size.width / 2.f, listTop - row * 32.f},
                [this, id] {
                    m_selected = id;
                    Mod::get()->setSavedValue("profiles-selected", id);
                    m_status->setString(("Selected: " + profileName(id)).c_str());
                }, .41f, id == m_selected ? "GJ_button_02.png" : "GJ_button_01.png");
        }

        addButton(menu, "Previous", {80.f, 145.f}, [this] {
            m_page = std::max(0, m_page - 1);
            Mod::get()->setSavedValue("profiles-page", m_page);
            m_status->setString("Page changed. Reopen this panel to refresh the list.");
        }, .36f, "GJ_button_03.png");
        addButton(menu, "Next", {390.f, 145.f}, [this, pageCount] {
            m_page = std::min(pageCount - 1, m_page + 1);
            Mod::get()->setSavedValue("profiles-page", m_page);
            m_status->setString("Page changed. Reopen this panel to refresh the list.");
        }, .36f, "GJ_button_03.png");

        addButton(menu, "Save", {85.f, 96.f}, [this] { saveSelected(); }, .39f);
        addButton(menu, "Apply", {235.f, 96.f}, [this] { loadSelected(); }, .39f, "GJ_button_02.png");
        addButton(menu, "Duplicate", {385.f, 96.f}, [this] { duplicateSelected(); }, .36f, "GJ_button_03.png");
        addButton(menu, "Rename", {85.f, 56.f}, [this] { renameSelected(); }, .36f);
        addButton(menu, "Delete", {235.f, 56.f}, [this] { deleteSelected(); }, .36f, "GJ_button_06.png");
        addButton(menu, "Create", {385.f, 56.f}, [this] { createProfile(); }, .36f, "GJ_button_04.png");

        auto selectedText = hasSelection() ? ("Selected: " + profileName(m_selected)) : "No profile selected";
        auto selectedLabel = CCLabelBMFont::create(selectedText.c_str(), "goldFont.fnt");
        selectedLabel->setScale(.29f);
        selectedLabel->setPosition({m_size.width / 2.f, 32.f});
        selectedLabel->setColor({180, 205, 230});
        m_mainLayer->addChild(selectedLabel);

        m_status = CCLabelBMFont::create(
            "Profiles capture every setting declared by New Keybinds.",
            "goldFont.fnt");
        m_status->setScale(.26f);
        m_status->setPosition({m_size.width / 2.f, 14.f});
        m_status->setColor({170, 180, 195});
        m_mainLayer->addChild(m_status);

        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static GlobalProfilesPopup* create() {
        auto ret = new GlobalProfilesPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class InstantSequencePopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(360.f, 285.f)) return false;
        setTitle("Instant Actions - Sequence Draft");

        auto steps = Mod::get()->getSavedValue<std::vector<std::string>>("instant-sequence-draft", {});
        std::string text = steps.empty() ? "No draft steps yet." : "";
        for (size_t i = 0; i < steps.size(); ++i)
            text += std::to_string(i + 1) + ". " + steps[i] + "\n";

        auto list = CCLabelBMFont::create(text.c_str(), "goldFont.fnt");
        list->setScale(.34f);
        list->setPosition({m_size.width / 2.f, m_size.height - 78.f});
        m_mainLayer->addChild(list);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2.f, 65.f});
        const char* labels[] = {"Add Pause Menu Step", "Add Level Browser Step", "Add Main Menu Step", "Clear Draft"};
        for (int i = 0; i < 4; ++i) {
            auto item = CCMenuItemExt::createSpriteExtra(
                ButtonSprite::create(labels[i], "goldFont.fnt", "GJ_button_01.png", .48f),
                [i](CCObject*) {
                    auto mod = Mod::get();
                    auto values = mod->getSavedValue<std::vector<std::string>>("instant-sequence-draft", {});
                    if (i == 0) values.push_back("Open Pause Menu");
                    else if (i == 1) values.push_back("Open Level Browser");
                    else if (i == 2) values.push_back("Open Main Menu");
                    else values.clear();
                    mod->setSavedValue("instant-sequence-draft", values);
                    FLAlertLayer::create("Sequence Draft",
                        "Draft saved. Reopen this panel to view the changes. Drafts are not automatically replayed.",
                        "OK")->show();
                });
            item->setPosition({0.f, -i * 34.f});
            menu->addChild(item);
        }

        auto note = CCLabelBMFont::create(
            "Stores UI action notes only; it does not automate gameplay.",
            "goldFont.fnt");
        note->setScale(.28f);
        note->setPosition({m_size.width / 2.f, 18.f});
        note->setColor({170, 180, 195});
        m_mainLayer->addChild(note);
        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static InstantSequencePopup* create() {
        auto ret = new InstantSequencePopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class InstantActionPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(400.f, 300.f)) return false;
        setTitle("Instant Actions");

        auto info = CCLabelBMFont::create(
            "Only actions with an implemented destination are shown.",
            "goldFont.fnt");
        info->setScale(.34f);
        info->setPosition({m_size.width / 2.f, m_size.height - 42.f});
        m_mainLayer->addChild(info);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2.f, m_size.height / 2.f - 2.f});

        struct ActionEntry { char const* title; char const* texture; std::function<void()> action; };
        ActionEntry actions[] = {
            {"Find Gameplay Level", "GJ_button_01.png", [] {
                auto target = Mod::get()->getSettingValue<std::string>("instant-level-id");
                if (target.empty()) target = "6508283";
                switchToScene(LevelBrowserLayer::scene(
                    GJSearchObject::create(SearchType::Type19, target + "&gameVersion=22")));
            }},
            {"Find Practice Level", "GJ_button_02.png", [] {
                auto target = Mod::get()->getSettingValue<std::string>("instant-level-id");
                if (target.empty()) target = "6508283";
                switchToScene(LevelBrowserLayer::scene(
                    GJSearchObject::create(SearchType::Type19, target + "&gameVersion=22")));
            }},
            {"Online Level Browser", "GJ_button_03.png", [] {
                switchToScene(LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Type19, "")));
            }},
            {"My Levels", "GJ_button_04.png", [] {
                switchToScene(LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MyLevels, "")));
            }},
            {"Pause Menu", "GJ_button_05.png", [] {
                if (auto play = PlayLayer::get()) play->pauseGame(true);
                else FLAlertLayer::create("Instant Actions", "No level is currently active.", "OK")->show();
            }},
            {"Main Menu", "GJ_button_06.png", [] { switchToScene(MenuLayer::scene(false)); }}
        };

        for (int i = 0; i < 6; ++i) {
            auto sprite = ButtonSprite::create(actions[i].title, "goldFont.fnt", actions[i].texture, .52f);
            auto item = CCMenuItemExt::createSpriteExtra(sprite,
                [action = actions[i].action](CCObject*) { action(); });
            item->setPosition({(i % 2 == 0) ? -98.f : 98.f, 78.f - (i / 2) * 63.f});
            menu->addChild(item);
        }

        auto draft = CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create("Sequence Draft", "goldFont.fnt", "GJ_button_01.png", .56f),
            [](CCObject*) { InstantSequencePopup::create()->show(); });
        draft->setPosition({0.f, -124.f});
        menu->addChild(draft);

        auto note = CCLabelBMFont::create(
            "Gameplay/Practice open the target search; select the level to continue.",
            "goldFont.fnt");
        note->setScale(.27f);
        note->setPosition({m_size.width / 2.f, 18.f});
        note->setColor({170, 180, 195});
        m_mainLayer->addChild(note);
        m_mainLayer->addChild(menu);
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

class NewKeybindsHubPopup : public Popup {
protected:
    bool init() override {
        if (!Popup::init(350.f, 310.f)) return false;
        setTitle("New Keybinds");

        auto subtitle = CCLabelBMFont::create("Choose a section", "goldFont.fnt");
        subtitle->setScale(.42f);
        subtitle->setPosition({m_size.width / 2.f, m_size.height - 48.f});
        m_mainLayer->addChild(subtitle);

        auto menu = CCMenu::create();
        menu->setPosition({m_size.width / 2.f, m_size.height / 2.f - 5.f});
        addButton(menu, "Custom Keybinds", {0.f, 79.f},
            [] { CustomKeybindsPopup::create()->show(); }, .62f);
        addButton(menu, "Instant Actions", {0.f, 29.f},
            [] { InstantActionPopup::create()->show(); }, .62f, "GJ_button_02.png");
        addButton(menu, "Smart Controls", {0.f, -21.f},
            [] { SmartControlsPopup::create()->show(); }, .62f, "GJ_button_03.png");
        addButton(menu, "Global Profiles", {0.f, -71.f},
            [] { GlobalProfilesPopup::create()->show(); }, .62f, "GJ_button_04.png");
        m_mainLayer->addChild(menu);
        return true;
    }

public:
    static NewKeybindsHubPopup* create() {
        auto ret = new NewKeybindsHubPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};
} // namespace

// Put a single New Keybinds entry in the Mods Library action column.
$execute {
    static ListenerHandle listener = ModItemUIEvent().listen(
        [](CCNode* item, std::string_view modID, std::optional<Mod*>) {
            if (modID != "sfj3861.new-keybinds" || !item)
                return ListenerResult::Propagate;

            CCNode* modsLayer = item;
            while (modsLayer && modsLayer->getID() != "ModsLayer")
                modsLayer = modsLayer->getParent();
            if (!modsLayer) return ListenerResult::Propagate;

            auto actionsMenu = modsLayer->getChildByID("right-actions-menu");
            if (!actionsMenu || actionsMenu->getChildByID("new-keybinds-library-button"))
                return ListenerResult::Propagate;

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

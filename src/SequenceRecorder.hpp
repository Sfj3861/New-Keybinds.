#pragma once
#include <Geode/Geode.hpp>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>

namespace nkb {
inline std::chrono::steady_clock::time_point& recordingStart() {
    static auto start = std::chrono::steady_clock::now();
    return start;
}
inline bool isRecording() {
    return geode::Mod::get()->getSavedValue<bool>("instant-recorder-active", false);
}
inline void startRecording() {
    auto mod = geode::Mod::get();
    mod->setSavedValue("instant-recorder-buffer", std::vector<std::string>{});
    recordingStart() = std::chrono::steady_clock::now();
    mod->setSavedValue("instant-recorder-active", true);
}
inline void recordStep(std::string const& action) {
    auto mod = geode::Mod::get();
    if (!isRecording()) return;
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - recordingStart()).count();
    std::ostringstream line;
    line << elapsed << " ms | " << action;
    auto steps = mod->getSavedValue<std::vector<std::string>>("instant-recorder-buffer", {});
    steps.push_back(line.str());
    mod->setSavedValue("instant-recorder-buffer", steps);
}
inline bool saveRecording() {
    auto mod = geode::Mod::get();
    if (!isRecording()) return false;
    auto steps = mod->getSavedValue<std::vector<std::string>>("instant-recorder-buffer", {});
    mod->setSavedValue("instant-sequence-draft", steps);
    mod->setSavedValue("instant-recorder-buffer", std::vector<std::string>{});
    mod->setSavedValue("instant-recorder-active", false);
    return !steps.empty();
}
}

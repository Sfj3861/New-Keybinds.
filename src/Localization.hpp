#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

namespace nkb {
inline std::string tr(std::string_view key) {
    auto lang = geode::Mod::get()->getSavedValue<std::string>("ui-language", "en");
    static const std::unordered_map<std::string, std::string> es = {
        {"Choose a section", "Elige una sección"}, {"Custom Keybinds", "Teclas personalizadas"},
        {"Instant Actions", "Acciones instantáneas"}, {"Smart Controls", "Controles inteligentes"},
        {"Global Profiles", "Perfiles globales"}, {"Language", "Idioma"},
        {"Gameplay", "Jugabilidad"}, {"Editor", "Editor"}, {"Navigation", "Navegación"},
        {"Configure the controls here. No Geode settings popup is needed.", "Configura aquí los controles. No necesitas abrir los ajustes de Geode."},
        {"Touch controls are recreated when the next level loads.", "Los controles táctiles se actualizan al cargar el siguiente nivel."},
        {"Current buttons send taps (press + release), not hold gestures.", "Los botones actuales envían toques, no gestos de mantener pulsado."},
        {"Button size", "Tamaño del botón"}, {"Button opacity", "Opacidad del botón"},
        {"Only actions with an implemented destination are shown.", "Solo se muestran acciones con un destino implementado."},
        {"Launch target level (Normal)", "Iniciar nivel objetivo (Normal)"},
        {"Launch target level (Practice)", "Iniciar nivel objetivo (Práctica)"},
        {"Online Level Browser", "Buscador de niveles"}, {"My Levels", "Mis niveles"},
        {"Pause Menu", "Menú de pausa"}, {"Main Menu", "Menú principal"},
        {"Record Steps", "Grabar pasos"}, {"Stop Recording", "Detener grabación"},
        {"Save Steps", "Guardar pasos"}, {"Saved sequence", "Secuencia guardada"},
        {"No steps recorded yet.", "Todavía no hay pasos grabados."},
        {"Recording started. Play, then tap the red floating button to save.", "Grabación iniciada. Juega y pulsa el botón rojo flotante para guardar."},
        {"Recording saved.", "Grabación guardada."}, {"Clear Saved Steps", "Borrar pasos guardados"},
        {"Start Recording", "Iniciar grabación"}, {"Close", "Cerrar"},
        {"English", "Inglés"}, {"Spanish", "Español"}, {"French", "Francés"}, {"Russian", "Ruso"},
        {"Profile name / duplicate name", "Nombre del perfil / copia"}
    };
    static const std::unordered_map<std::string, std::string> fr = {
        {"Choose a section", "Choisir une section"}, {"Custom Keybinds", "Touches personnalisées"},
        {"Instant Actions", "Actions instantanées"}, {"Smart Controls", "Commandes tactiles"},
        {"Global Profiles", "Profils globaux"}, {"Language", "Langue"},
        {"Gameplay", "Jouabilité"}, {"Editor", "Éditeur"}, {"Navigation", "Navigation"},
        {"Configure the controls here. No Geode settings popup is needed.", "Configurez les commandes ici, sans ouvrir les paramètres Geode."},
        {"Touch controls are recreated when the next level loads.", "Les commandes tactiles sont recréées au chargement du prochain niveau."},
        {"Current buttons send taps (press + release), not hold gestures.", "Les boutons envoient des pressions brèves, pas des maintiens."},
        {"Button size", "Taille du bouton"}, {"Button opacity", "Opacité du bouton"},
        {"Only actions with an implemented destination are shown.", "Seules les actions disponibles sont affichées."},
        {"Launch target level (Normal)", "Lancer le niveau cible (Normal)"},
        {"Launch target level (Practice)", "Lancer le niveau cible (Entraînement)"},
        {"Online Level Browser", "Navigateur de niveaux"}, {"My Levels", "Mes niveaux"},
        {"Pause Menu", "Menu pause"}, {"Main Menu", "Menu principal"},
        {"Record Steps", "Enregistrer les étapes"}, {"Stop Recording", "Arrêter l'enregistrement"},
        {"Save Steps", "Enregistrer les étapes"}, {"Saved sequence", "Séquence enregistrée"},
        {"No steps recorded yet.", "Aucune étape enregistrée."},
        {"Recording started. Play, then tap the red floating button to save.", "Enregistrement lancé. Jouez puis appuyez sur le bouton rouge pour enregistrer."},
        {"Recording saved.", "Enregistrement sauvegardé."}, {"Clear Saved Steps", "Effacer les étapes"},
        {"Start Recording", "Démarrer l'enregistrement"}, {"Close", "Fermer"},
        {"English", "Anglais"}, {"Spanish", "Espagnol"}, {"French", "Français"}, {"Russian", "Russe"},
        {"Profile name / duplicate name", "Nom du profil / copie"}
    };
    static const std::unordered_map<std::string, std::string> ru = {
        {"Choose a section", "Выберите раздел"}, {"Custom Keybinds", "Свои клавиши"},
        {"Instant Actions", "Мгновенные действия"}, {"Smart Controls", "Умное управление"},
        {"Global Profiles", "Общие профили"}, {"Language", "Язык"},
        {"Gameplay", "Игра"}, {"Editor", "Редактор"}, {"Navigation", "Навигация"},
        {"Configure the controls here. No Geode settings popup is needed.", "Настройте управление здесь, без окна настроек Geode."},
        {"Touch controls are recreated when the next level loads.", "Сенсорные кнопки обновятся при загрузке следующего уровня."},
        {"Current buttons send taps (press + release), not hold gestures.", "Кнопки выполняют короткое нажатие, а не удержание."},
        {"Button size", "Размер кнопки"}, {"Button opacity", "Прозрачность кнопки"},
        {"Only actions with an implemented destination are shown.", "Показаны только доступные действия."},
        {"Launch target level (Normal)", "Запустить уровень (Обычный)"},
        {"Launch target level (Practice)", "Запустить уровень (Практика)"},
        {"Online Level Browser", "Поиск уровней"}, {"My Levels", "Мои уровни"},
        {"Pause Menu", "Меню паузы"}, {"Main Menu", "Главное меню"},
        {"Record Steps", "Записать шаги"}, {"Stop Recording", "Остановить запись"},
        {"Save Steps", "Сохранить шаги"}, {"Saved sequence", "Сохранённая последовательность"},
        {"No steps recorded yet.", "Шаги ещё не записаны."},
        {"Recording started. Play, then tap the red floating button to save.", "Запись началась. Играйте и нажмите красную кнопку для сохранения."},
        {"Recording saved.", "Запись сохранена."}, {"Clear Saved Steps", "Очистить шаги"},
        {"Start Recording", "Начать запись"}, {"Close", "Закрыть"},
        {"English", "Английский"}, {"Spanish", "Испанский"}, {"French", "Французский"}, {"Russian", "Русский"},
        {"Profile name / duplicate name", "Имя профиля / копии"}
    };
    const std::unordered_map<std::string, std::string>* table = nullptr;
    if (lang == "es") table = &es;
    else if (lang == "fr") table = &fr;
    else if (lang == "ru") table = &ru;
    if (table) {
        auto it = table->find(std::string(key));
        if (it != table->end()) return it->second;
    }
    return std::string(key);
}
}

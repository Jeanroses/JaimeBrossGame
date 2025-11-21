#pragma once
#include "raylib.h"
#include "Levels.h"
#include "DbContext.h"
#include <vector>
#include <string>

class Menu
{
public:
    Animations animations;

    bool score_saved = false;

    // Métodos (solo cambia ending)
    void set_audio_volume();
    void apply_master_volume(class Player& player, class Levels& level_1, class Levels& level_2);
    void init_animation();
    void draw();
    void draw_login();
    void draw_keybindings();
    void draw_settings(class Player& player, class Levels& level_1, class Levels& level_2);
    void draw_scores();
    void check_button();
    void pause();
    void pause_draw(Player& player, Levels& level_1, Levels& level_2);

    // ACTUALIZADO: Recibe Player para guardar el puntaje
    void ending(class Player& player, Levels& level_1, Levels& level_2);

    void reset_lvl(Levels& level_1, Levels& level_2);
    void toggle_fullscreen();
    void toggle_vsync();
    void toggle_fps();
    void apply_video_settings();

    // ... (resto de variables igual) ...
    Font font = LoadFont("resources/graphics/fonts/font.ttf");
    Color color = { 255, 255, 255, 0 };
    Music menu_music = LoadMusicStream("resources/audio/music/menu.mp3");
    Music ending_music = LoadMusicStream("resources/audio/music/ending.mp3");
    Sound button_sound = LoadSound("resources/audio/sound/button.wav");
    Texture2D ending_background = LoadTexture("resources/ending.png");
    Texture2D menu_background1 = LoadTexture("resources/graphics/backgrounds/sky_background1.png");
    Texture2D menu_background2 = LoadTexture("resources/graphics/backgrounds/sky_background2.png");
    Texture2D logo = LoadTexture("resources/logo.png");

    Rectangle ending_ = { 0, 0, 1920, 1080 };
    Rectangle logo_src = { 0, 0, 64, 48 };
    Rectangle logo_disp = { 700, 30, 500, 400 };
    Rectangle background_src = { 0, 0, 320, 180 };
    Rectangle background_disp = { 0, 0, 1920, 1080 };

    Rectangle start_button = { 780, 460, 360, 80 };
    Rectangle scores_button = { 780, 560, 360, 80 };
    Rectangle keybindings_button = { 780, 660, 360, 80 };
    Rectangle settings_button = { 780, 760, 360, 80 };
    Rectangle exit_button = { 780, 860, 360, 80 };

    Rectangle keybindings_background = { 500, 430, 900, 350 };
    Rectangle return_button = { 640, 845, 600, 100 };
    Rectangle continue_button = { 755, 400, 400, 100 };
    Rectangle exit_to_menu_button = { 730, 520, 450, 100 };

    Vector2 mouse_pos = { 0, 0 };
    Vector2 ending_pos1 = { 70, 200 };
    Vector2 ending_pos2 = { 230, 350 };

    Vector2 start_game_pos = { 840, 485 };
    Vector2 scores_pos_text = { 900, 585 };
    Vector2 keybindings_pos = { 830, 685 };
    Vector2 settings_pos = { 880, 785 };
    Vector2 exit_pos = { 865, 885 };

    Vector2 return_pos = { 700, 875 };
    Vector2 pause_pos = { 0, 0 };
    Vector2 pause_pos2 = { 0, 0 };
    Vector2 movement_pos = { 625, 670 };
    Vector2 sprint_pos = { 950 , 670 };
    Vector2 jump_pos = { 1200, 670 };
    Vector2 origin = { 0, 0 };

    int logoPositionX = 832;
    int logoPositionY = 412;
    int framesCounter = 0;
    int lettersCount = 0;
    int topSideRecWidth = 16;
    int leftSideRecHeight = 16;
    int bottomSideRecWidth = 16;
    int rightSideRecHeight = 16;
    int state = 0;
    float alpha = 1.0f;

    bool login_screen = true;
    bool scores = false;
    bool keybindings = false;
    bool settings = false;
    bool start_game = false;
    bool exit_game = false;
    bool init = true;
    bool pause_ = false;

    char nameInput[16] = "\0";
    int inputLetterCount = 0;
    int current_user_id = -1;

    std::vector<ScoreEntry> top_scores;
    bool scores_loaded = false;

    bool play_button_sound = false;
    float menu_volume = 0.06;
    float ending_volume = 0.07;
    float button_volume = 0.08;
    float master_volume = 0.15f;
    bool vsync_enabled = true;
    bool fullscreen = true;
    bool show_fps = false;
    int resolution_index = 2;

    bool suppress_menu_click_one_frame = false;
};
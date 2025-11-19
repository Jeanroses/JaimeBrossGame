#include "Menu.h"
#include "Player.h"
#include "raylib.h"

static float SliderHorizontal(Rectangle bounds, float value, float minValue, float maxValue)
{
    float t = (value - minValue) / (maxValue - minValue);
    if (t < 0) t = 0; if (t > 1) t = 1;
    const float knobW = 20.0f;
    Rectangle bar = { bounds.x, bounds.y + bounds.height/2 - 4, bounds.width, 8 };
    Rectangle knob = { bounds.x + t * (bounds.width - knobW), bounds.y + bounds.height/2 - knobW/2, knobW, knobW };
    DrawRectangleRounded(bar, 0.4f, 0, LIGHTGRAY);
    DrawRectangleRounded(knob, 0.5f, 0, DARKGRAY);
    Vector2 m = GetMousePosition();
    if (CheckCollisionPointRec(m, bounds) && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        float nt = (m.x - bounds.x) / bounds.width;
        if (nt < 0) nt = 0; if (nt > 1) nt = 1;
        value = minValue + nt * (maxValue - minValue);
    }
    return value;
}

void Menu::set_audio_volume()
{
    SetMasterVolume(master_volume);
    SetMusicVolume(menu_music, master_volume);
    SetMusicVolume(ending_music, master_volume);
    SetSoundVolume(button_sound, master_volume);
}

void Menu::apply_master_volume(Player& player, Levels& level_1, Levels& level_2)
{
    SetMasterVolume(master_volume);
    SetMusicVolume(menu_music, master_volume);
    SetMusicVolume(ending_music, master_volume);
    SetMusicVolume(level_1.level1_music, master_volume);
    SetMusicVolume(level_2.level2_music, master_volume);
    SetSoundVolume(button_sound, master_volume);
    SetSoundVolume(level_1.coin_sound, master_volume);
    SetSoundVolume(level_1.life_up_sound, master_volume);
    SetSoundVolume(level_2.coin_sound, master_volume);
    SetSoundVolume(level_2.life_up_sound, master_volume);
    player.jump_volume = master_volume;
    player.hit_volume = master_volume;
    player.death_volume = master_volume;
    player.set_audio_volume();
}

void Menu::toggle_fullscreen()
{
    fullscreen = !fullscreen;
    if (fullscreen && !IsWindowFullscreen()) ToggleFullscreen();
    else if (!fullscreen && IsWindowFullscreen()) ToggleFullscreen();
}

void Menu::toggle_vsync()
{
    vsync_enabled = !vsync_enabled; 
}

void Menu::toggle_fps()
{
    show_fps = !show_fps;
}

void Menu::apply_video_settings()
{
    struct Res { int w; int h; }; static Res resolutions[] = { {1280,720},{1600,900},{1920,1080} };
    if (resolution_index < 0) resolution_index = 0; if (resolution_index > 2) resolution_index = 2;
    int targetW = resolutions[resolution_index].w;
    int targetH = resolutions[resolution_index].h;
    if (!fullscreen && (GetScreenWidth() != targetW || GetScreenHeight() != targetH))
        SetWindowSize(targetW, targetH);
    if (show_fps) DrawFPS(10,10);
}

void Menu::init_animation()
{
    if (state == 0)
    {
        framesCounter++;
        if (framesCounter == 120) { state = 1; framesCounter = 0; }
    }
    else if (state == 1)
    {
        topSideRecWidth += 4;
        leftSideRecHeight += 4;
        if (topSideRecWidth >= 256) state = 2;
    }
    else if (state == 2)
    {
        bottomSideRecWidth += 4;
        rightSideRecHeight += 4;
        if (bottomSideRecWidth >= 256) state = 3;
    }
    else if (state == 3)
    {
        framesCounter++;
        if (framesCounter / 12) { lettersCount++; framesCounter = 0; }
        if (lettersCount >= 10)
        {
            alpha -= 0.02f;
            if (alpha <= 0.0f) { alpha = 0.0f; state = 4; }
        }
    }
    else if (state == 4) init = false;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (state == 0)
    {
        if ((framesCounter / 15) % 2) DrawRectangle(logoPositionX, logoPositionY, 16, 16, BLACK);
    }
    else if (state <= 3)
    {
        DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(BLACK, alpha));
        DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(BLACK, alpha));
        DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(BLACK, alpha));
        DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(BLACK, alpha));
        if (state == 3)
        {
            DrawRectangle(GetScreenWidth()/2 - 112, GetScreenHeight()/2 - 112, 224, 224, Fade(RAYWHITE, alpha));
            DrawText(TextSubtext("raylib", 0, lettersCount), GetScreenWidth()/2 - 44, GetScreenHeight()/2 + 48, 50, Fade(BLACK, alpha));
        }
    }
    EndDrawing();
}

void Menu::draw()
{
    UpdateMusicStream(menu_music);
    mouse_pos = GetMousePosition();

    ClearBackground(SKYBLUE);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(menu_background2, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);
    DrawRectangleRounded(start_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRounded(keybindings_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRounded(settings_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRounded(exit_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(start_button, 0.3f, 6, BLACK);
    DrawRectangleRoundedLines(keybindings_button, 0.3f, 6, BLACK);
    DrawRectangleRoundedLines(settings_button, 0.3f, 6, BLACK);
    DrawRectangleRoundedLines(exit_button, 0.3f, 6, BLACK);
    DrawTextEx(font, "Start game", start_game_pos, 50, 5, BLACK);
    DrawTextEx(font, "Keybindings", keybindings_pos, 50, 5, BLACK);
    DrawTextEx(font, "Settings", settings_pos, 50, 5, BLACK);
    DrawTextEx(font, "Exit game", exit_pos, 50, 5, BLACK);
    apply_video_settings();
}

void Menu::draw_keybindings()
{
    UpdateMusicStream(menu_music);
    mouse_pos = GetMousePosition();
    ClearBackground(SKYBLUE);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(menu_background2, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);
    DrawRectangleRounded(return_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(return_button, 0.3f, 6, BLACK);
    DrawRectangleRounded(keybindings_background, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(keybindings_background, 0.3f, 6, BLACK);
    animations.keybindings();
    DrawTextEx(font, "Return to menu", return_pos, 50, 5, BLACK);
    DrawTextEx(font, "movement", movement_pos, 30, 3, BLACK);
    DrawTextEx(font, "sprint", sprint_pos, 30, 3, BLACK);
    DrawTextEx(font, "jump", jump_pos, 30, 3, BLACK);
    apply_video_settings();
}

void Menu::draw_settings(Player& player, Levels& level_1, Levels& level_2)
{
    if (!IsMusicStreamPlaying(menu_music)) PlayMusicStream(menu_music);
    StopMusicStream(level_1.level1_music);
    StopMusicStream(level_2.level2_music);
    StopMusicStream(ending_music);
    UpdateMusicStream(menu_music);

    mouse_pos = GetMousePosition();
    ClearBackground(SKYBLUE);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(menu_background2, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);

    Rectangle panel = { 420, 320, 1080, 640 };
    DrawRectangleRounded(panel, 0.2f, 0, RAYWHITE);
    DrawRectangleRoundedLines(panel, 0.2f, 6, BLACK);
    DrawTextEx(font, "Settings", Vector2{ panel.x + 40, panel.y + 30 }, 60, 6, BLACK);

    float y = panel.y + 120;
    float xLabel = panel.x + 40;
    float xSlider = panel.x + 340;
    float sliderW = panel.width - 380;

    DrawTextEx(font, "Master Volume", Vector2{ xLabel, y }, 40, 4, BLACK);
    master_volume = SliderHorizontal(Rectangle{ xSlider, y + 50, sliderW, 44 }, master_volume, 0.0f, 1.0f);
    y += 140;
    DrawTextEx(font, TextFormat("Fullscreen (F): %s", fullscreen ? "On" : "Off"), Vector2{ xLabel, y }, 30, 3, BLACK);
    if (IsKeyPressed(KEY_F)) toggle_fullscreen();
    y += 50;
    DrawTextEx(font, TextFormat("Show FPS (G): %s", show_fps ? "Yes" : "No"), Vector2{ xLabel, y }, 30, 3, BLACK);
    if (IsKeyPressed(KEY_G)) toggle_fps();
    y += 50;
    DrawTextEx(font, TextFormat("Resolution (H/J): %dx%d", resolution_index==0?1280:resolution_index==1?1600:1920, resolution_index==0?720:resolution_index==1?900:1080), Vector2{ xLabel, y }, 30, 3, BLACK);
    if (IsKeyPressed(KEY_H)) resolution_index = (resolution_index + 1) % 3;
    if (IsKeyPressed(KEY_J)) resolution_index = (resolution_index + 2) % 3;

    apply_master_volume(player, level_1, level_2);
    apply_video_settings();

    Rectangle backBtn = { panel.x + panel.width/2 - 200, panel.y + panel.height - 90, 400, 70 };
    DrawRectangleRounded(backBtn, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(backBtn, 0.3f, 6, BLACK);
    DrawTextEx(font, "Return to menu", Vector2{ backBtn.x + 40, backBtn.y + 20 }, 40, 4, BLACK);

    if (CheckCollisionPointRec(mouse_pos, backBtn))
    {
        DrawRectangleRounded(backBtn, 0.3f, 0, GRAY);
        DrawRectangleRoundedLines(backBtn, 0.3f, 6, BLACK);
        DrawTextEx(font, "Return to menu", Vector2{ backBtn.x + 40, backBtn.y + 20 }, 40, 4, BLACK);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) settings = false;
    }
}

void Menu::check_button()
{
    if (!keybindings && !settings)
    {
        if (CheckCollisionPointRec(mouse_pos, start_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(start_button, 0.3f, 0, GRAY);
            DrawTextEx(font, "Start game", start_game_pos, 50, 5, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) start_game = true;
        }
        else if (CheckCollisionPointRec(mouse_pos, keybindings_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(keybindings_button, 0.3f, 0, GRAY);
            DrawTextEx(font, "Keybindings", keybindings_pos, 50, 5, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) keybindings = true;
        }
        else if (CheckCollisionPointRec(mouse_pos, settings_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(settings_button, 0.3f, 0, GRAY);
            DrawTextEx(font, "Settings", settings_pos, 50, 5, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) settings = true;
        }
        else if (CheckCollisionPointRec(mouse_pos, exit_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(exit_button, 0.3f, 0, GRAY);
            DrawTextEx(font, "Exit game", exit_pos, 50, 5, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) exit_game = true;
        }
        else play_button_sound = true;
    }
    if (keybindings)
    {
        if (CheckCollisionPointRec(mouse_pos, return_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(return_button, 0.3f, 0, GRAY);
            DrawTextEx(font, "Return to menu", return_pos, 50, 5, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) keybindings = false;
        }
        else play_button_sound = true;
    }
}

void Menu::pause()
{
    if (IsKeyPressed(KEY_ESCAPE)) pause_ = !pause_;
}

void Menu::pause_draw(Player& player, Levels& level_1, Levels& level_2)
{
    mouse_pos = GetMousePosition();
    if (pause_)
    {
        pause_pos = GetScreenToWorld2D(Vector2{ 300, 400 }, player.camera);
        pause_pos2 = GetScreenToWorld2D(Vector2{ 380, 700 }, player.camera);
        DrawTextEx(font, "PAUSED", pause_pos, 300, 15, BLACK);
        DrawTextEx(font, "Press Q to leave to menu", pause_pos2, 50, 15, BLACK);
        if (IsKeyPressed(KEY_Q))
        {
            start_game = false;
            pause_ = false;
            StopMusicStream(level_1.level1_music);
            StopMusicStream(level_2.level2_music);
            StopMusicStream(ending_music);
            if (!IsMusicStreamPlaying(menu_music)) PlayMusicStream(menu_music);
        }
    }
    if (pause_) apply_video_settings();
}

void Menu::ending(Levels& level_1, Levels& level_2)
{
    UpdateMusicStream(ending_music);
    ClearBackground(RAYWHITE);
    DrawTextureRec(ending_background, ending_, Vector2{ 0, 0 }, color);
    if (color.a < 255) color.a++;
    if (color.a == 255)
    {
        DrawTextEx(font, "Thank you for playing", ending_pos1, 120, 10, BLACK);
        DrawTextEx(font, "Press Escape to return to menu", ending_pos2, 60, 10, BLACK);
        if (IsKeyPressed(KEY_ESCAPE))
        {
            StopMusicStream(level_1.level1_music);
            StopMusicStream(level_2.level2_music);
            StopMusicStream(ending_music);
            if (!IsMusicStreamPlaying(menu_music)) PlayMusicStream(menu_music);
            start_game = false;
            level_1.completed = false;
            level_2.completed = false;
        }
    }
    apply_video_settings();
}

void Menu::reset_lvl(Levels& level_1, Levels& level_2)
{
    color.a = 0;
    level_1.completed = false;
    level_1.start_level = false;
    level_2.start_level = false;
    level_1.init_maps();
    level_2.init_maps();
}

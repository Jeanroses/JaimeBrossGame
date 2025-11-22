#include "Menu.h"
#include "Player.h"
#include "raylib.h"

static int activeTextBox = 0;
static char passwordInput[16] = "";
static int passwordLetterCount = 0;
static std::string loginMessage = "";

static float SliderHorizontal(Rectangle bounds, float value, float minValue, float maxValue)
{
    float t = (value - minValue) / (maxValue - minValue);
    if (t < 0) t = 0; if (t > 1) t = 1;
    const float knobW = 20.0f;
    Rectangle bar = { bounds.x, bounds.y + bounds.height / 2 - 4, bounds.width, 8 };
    Rectangle knob = { bounds.x + t * (bounds.width - knobW), bounds.y + bounds.height / 2 - knobW / 2, knobW, knobW };
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

void Menu::toggle_vsync() { vsync_enabled = !vsync_enabled; }
void Menu::toggle_fps() { show_fps = !show_fps; }

void Menu::apply_video_settings()
{
    struct Res { int w; int h; }; static Res resolutions[] = { {1280,720},{1600,900},{1920,1080} };
    if (resolution_index < 0) resolution_index = 0; if (resolution_index > 2) resolution_index = 2;
    int targetW = resolutions[resolution_index].w;
    int targetH = resolutions[resolution_index].h;
    if (!fullscreen && (GetScreenWidth() != targetW || GetScreenHeight() != targetH))
        SetWindowSize(targetW, targetH);
}

void Menu::init_animation()
{
    if (state == 0) { framesCounter++; if (framesCounter == 120) { state = 1; framesCounter = 0; } }
    else if (state == 1) { topSideRecWidth += 4; leftSideRecHeight += 4; if (topSideRecWidth >= 256) state = 2; }
    else if (state == 2) { bottomSideRecWidth += 4; rightSideRecHeight += 4; if (bottomSideRecWidth >= 256) state = 3; }
    else if (state == 3) {
        framesCounter++;
        if (framesCounter / 12) { lettersCount++; framesCounter = 0; }
        if (lettersCount >= 10) { alpha -= 0.02f; if (alpha <= 0.0f) { alpha = 0.0f; state = 4; } }
    }
    else if (state == 4) init = false;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (state == 0) { if ((framesCounter / 15) % 2) DrawRectangle(logoPositionX, logoPositionY, 16, 16, BLACK); }
    else if (state <= 3) {
        DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(BLACK, alpha));
        DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(BLACK, alpha));
        DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(BLACK, alpha));
        DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(BLACK, alpha));
        if (state == 3) {
            DrawRectangle(GetScreenWidth() / 2 - 112, GetScreenHeight() / 2 - 112, 224, 224, Fade(RAYWHITE, alpha));
            DrawText(TextSubtext("raylib", 0, lettersCount), GetScreenWidth() / 2 - 44, GetScreenHeight() / 2 + 48, 50, Fade(BLACK, alpha));
        }
    }
    EndDrawing();
}

void Menu::draw_login()
{
    set_audio_volume();
    UpdateMusicStream(menu_music);

    if (IsKeyPressed(KEY_TAB)) {
        activeTextBox = (activeTextBox + 1) % 2;
    }

    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125)) {
            if (activeTextBox == 0 && (inputLetterCount < 15)) {
                nameInput[inputLetterCount] = (char)key;
                nameInput[inputLetterCount + 1] = '\0';
                inputLetterCount++;
            }
            else if (activeTextBox == 1 && (passwordLetterCount < 15)) {
                passwordInput[passwordLetterCount] = (char)key;
                passwordInput[passwordLetterCount + 1] = '\0';
                passwordLetterCount++;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (activeTextBox == 0) {
            inputLetterCount--;
            if (inputLetterCount < 0) inputLetterCount = 0;
            nameInput[inputLetterCount] = '\0';
        }
        else if (activeTextBox == 1) {
            passwordLetterCount--;
            if (passwordLetterCount < 0) passwordLetterCount = 0;
            passwordInput[passwordLetterCount] = '\0';
        }
    }

    if (IsKeyPressed(KEY_ENTER) && inputLetterCount > 0 && passwordLetterCount > 0)
    {
        DbContext db;
        if (db.Connect()) {
            current_user_id = db.LoginOrRegister(std::string(nameInput), std::string(passwordInput));
            db.Disconnect();

            if (current_user_id > 0) {
                loginMessage = "";
                login_screen = false;
                init = false;
            }
            else if (current_user_id == -2) {
                loginMessage = "Invalid username or password";
            }
            else {
                loginMessage = "Database connection error";
            }
        }
        else {
            loginMessage = "Failed to connect to database";
        }
    }

    ClearBackground(BLACK);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);

    Rectangle panel = { 660, 450, 600, 450 };
    DrawRectangleRounded(panel, 0.2f, 0, Fade(RAYWHITE, 0.9f));
    DrawRectangleRoundedLines(panel, 0.2f, 6, BLACK);

    Vector2 titleSize = MeasureTextEx(font, "LOGIN / REGISTER", 40, 4);
    Vector2 titlePos = { panel.x + panel.width / 2 - titleSize.x / 2, panel.y + 20 };
    DrawTextEx(font, "LOGIN / REGISTER", titlePos, 40, 4, BLACK);

    DrawText("Username", (int)panel.x + 50, (int)panel.y + 90, 20, DARKGRAY);
    Rectangle userBox = { panel.x + 50, panel.y + 120, panel.width - 100, 50 };
    DrawRectangleRec(userBox, LIGHTGRAY);
    DrawRectangleLinesEx(userBox, 2, activeTextBox == 0 ? BLACK : DARKGRAY);
    Vector2 userTextPos = { userBox.x + 10, userBox.y + (userBox.height / 2) - (MeasureTextEx(font, nameInput, 40, 4).y / 2) };
    DrawTextEx(font, nameInput, userTextPos, 40, 4, MAROON);

    DrawText("Password", (int)panel.x + 50, (int)panel.y + 190, 20, DARKGRAY);
    Rectangle passBox = { panel.x + 50, panel.y + 220, panel.width - 100, 50 };
    DrawRectangleRec(passBox, LIGHTGRAY);
    DrawRectangleLinesEx(passBox, 2, activeTextBox == 1 ? BLACK : DARKGRAY);

    std::string maskedPassword(passwordLetterCount, '*');
    Vector2 passTextPos = { passBox.x + 10, passBox.y + (passBox.height / 2) - (MeasureTextEx(font, maskedPassword.c_str(), 40, 4).y / 2) };
    DrawTextEx(font, maskedPassword.c_str(), passTextPos, 40, 4, MAROON);

    if ((framesCounter / 20) % 2 == 0) {
        if (activeTextBox == 0 && inputLetterCount < 15) {
            float w = MeasureTextEx(font, nameInput, 40, 4).x;
            Vector2 caretPos = { userBox.x + 10 + w, userBox.y + (userBox.height / 2) - (MeasureTextEx(font, "|", 40, 4).y / 2) };
            DrawTextEx(font, "|", caretPos, 40, 4, BLACK);
        }
        else if (activeTextBox == 1 && passwordLetterCount < 15) {
            float w = MeasureTextEx(font, maskedPassword.c_str(), 40, 4).x;
            Vector2 caretPos = { passBox.x + 10 + w, passBox.y + (passBox.height / 2) - (MeasureTextEx(font, "|", 40, 4).y / 2) };
            DrawTextEx(font, "|", caretPos, 40, 4, BLACK);
        }
    }
    framesCounter++;

    if (!loginMessage.empty()) {
        Vector2 msgSize = MeasureTextEx(font, loginMessage.c_str(), 20, 0);
        DrawText(loginMessage.c_str(), (int)(panel.x + panel.width / 2 - msgSize.x / 2), (int)passBox.y + 60, 20, RED);
    }

    Vector2 enterSize = MeasureTextEx(font, "Press ENTER to start", 30, 3);
    DrawTextEx(font, "Press ENTER to start", Vector2{ panel.x + panel.width / 2 - enterSize.x / 2, panel.y + panel.height - 80 }, 30, 3, DARKGRAY);
    Vector2 tabSize = MeasureTextEx(font, "Press TAB to switch fields", 20, 3);
    DrawTextEx(font, "Press TAB to switch fields", Vector2{ panel.x + panel.width / 2 - tabSize.x / 2, panel.y + panel.height - 40 }, 20, 3, DARKGRAY);
}

void Menu::draw()
{
    UpdateMusicStream(menu_music);
    mouse_pos = GetMousePosition();

    ClearBackground(BLACK);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);

    DrawRectangleRounded(start_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRounded(scores_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRounded(keybindings_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRounded(settings_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRounded(exit_button, 0.3f, 0, RAYWHITE);

    DrawRectangleRoundedLines(start_button, 0.3f, 6, BLACK);
    DrawRectangleRoundedLines(scores_button, 0.3f, 6, BLACK);
    DrawRectangleRoundedLines(keybindings_button, 0.3f, 6, BLACK);
    DrawRectangleRoundedLines(settings_button, 0.3f, 6, BLACK);
    DrawRectangleRoundedLines(exit_button, 0.3f, 6, BLACK);

    Vector2 ts;
    ts = MeasureTextEx(font, "Start game", 40, 4);
    DrawTextEx(font, "Start game", Vector2{ start_button.x + start_button.width / 2 - ts.x / 2, start_button.y + start_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
    ts = MeasureTextEx(font, "Scores", 40, 4);
    DrawTextEx(font, "Scores", Vector2{ scores_button.x + scores_button.width / 2 - ts.x / 2, scores_button.y + scores_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
    ts = MeasureTextEx(font, "Keybindings", 40, 4);
    DrawTextEx(font, "Keybindings", Vector2{ keybindings_button.x + keybindings_button.width / 2 - ts.x / 2, keybindings_button.y + keybindings_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
    ts = MeasureTextEx(font, "Settings", 40, 4);
    DrawTextEx(font, "Settings", Vector2{ settings_button.x + settings_button.width / 2 - ts.x / 2, settings_button.y + settings_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
    ts = MeasureTextEx(font, "Exit game", 40, 4);
    DrawTextEx(font, "Exit game", Vector2{ exit_button.x + exit_button.width / 2 - ts.x / 2, exit_button.y + exit_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);

    DrawTextEx(font, TextFormat("Player: %s", nameInput), Vector2{ 20, 40 }, 30, 3, BLACK);

    apply_video_settings();
}

void Menu::draw_keybindings()
{
    UpdateMusicStream(menu_music);
    mouse_pos = GetMousePosition();
    ClearBackground(BLACK);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);
    DrawRectangleRounded(return_button, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(return_button, 0.3f, 6, BLACK);
    DrawRectangleRounded(keybindings_background, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(keybindings_background, 0.3f, 6, BLACK);
    animations.keybindings();
    Vector2 rt = MeasureTextEx(font, "Return to menu", 50, 5);
    DrawTextEx(font, "Return to menu", Vector2{ return_button.x + return_button.width / 2 - rt.x / 2, return_button.y + return_button.height / 2 - rt.y / 2 }, 50, 5, BLACK);
    Vector2 mv = MeasureTextEx(font, "movement", 30, 3);
    DrawTextEx(font, "movement", Vector2{ movement_pos.x - mv.x / 2, movement_pos.y - mv.y / 2 }, 30, 3, BLACK);
    Vector2 sp = MeasureTextEx(font, "sprint", 30, 3);
    DrawTextEx(font, "sprint", Vector2{ sprint_pos.x - sp.x / 2, sprint_pos.y - sp.y / 2 }, 30, 3, BLACK);
    Vector2 jm = MeasureTextEx(font, "jump", 30, 3);
    DrawTextEx(font, "jump", Vector2{ jump_pos.x - jm.x / 2, jump_pos.y - jm.y / 2 }, 30, 3, BLACK);
    apply_video_settings();
}

void Menu::draw_scores()
{
    UpdateMusicStream(menu_music);
    mouse_pos = GetMousePosition();

    if (!scores_loaded) {
        DbContext db;
        if (db.Connect()) {
            top_scores = db.GetTopScores();
            db.Disconnect();
        }
        scores_loaded = true;
    }

    ClearBackground(BLACK);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);

    Rectangle panel = { 410, 430, 1100, 500 };
    DrawRectangleRounded(panel, 0.2f, 0, RAYWHITE);
    DrawRectangleRoundedLines(panel, 0.2f, 6, BLACK);

    Vector2 titleSize = MeasureTextEx(font, "Top Scores", 60, 5);
    DrawTextEx(font, "Top Scores", Vector2{ panel.x + panel.width / 2 - titleSize.x / 2, panel.y + 10 }, 60, 5, BLACK);

    DrawTextEx(font, "Rank", Vector2{ panel.x + 50, panel.y + 100 }, 30, 3, DARKGRAY);
    DrawTextEx(font, "Name", Vector2{ panel.x + 230, panel.y + 100 }, 30, 3, DARKGRAY);
    DrawTextEx(font, "Score", Vector2{ panel.x + 610, panel.y + 100 }, 30, 3, DARKGRAY);
    DrawTextEx(font, "Level", Vector2{ panel.x + 820, panel.y + 100 }, 30, 3, DARKGRAY);
    DrawLine(panel.x + 20, panel.y + 140, panel.x + panel.width - 20, panel.y + 140, BLACK);

    int yOffset = panel.y + 150;
    int rank = 1;
    for (const auto& entry : top_scores) {
        DrawTextEx(font, TextFormat("%d", rank), Vector2{ panel.x + 60, (float)yOffset }, 30, 2, BLACK);
        DrawTextEx(font, entry.username.c_str(), Vector2{ panel.x + 230, (float)yOffset }, 30, 2, BLACK);
        DrawTextEx(font, TextFormat("%d", entry.score), Vector2{ panel.x + 610, (float)yOffset }, 30, 2, BLACK);
        DrawTextEx(font, TextFormat("%d", entry.level), Vector2{ panel.x + 820, (float)yOffset }, 30, 2, BLACK);
        yOffset += 40;
        rank++;
        if (rank > 8) break;
    }

    Rectangle backBtn = { panel.x + panel.width / 2 - 200, panel.y + panel.height - 90, 400, 70 };
    DrawRectangleRounded(backBtn, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(backBtn, 0.3f, 6, BLACK);
    Vector2 backSize = MeasureTextEx(font, "Return to menu", 40, 4);
    DrawTextEx(font, "Return to menu", Vector2{ backBtn.x + backBtn.width / 2 - backSize.x / 2, backBtn.y + backBtn.height / 2 - backSize.y / 2 }, 40, 4, BLACK);

    if (CheckCollisionPointRec(mouse_pos, backBtn))
    {
        DrawRectangleRounded(backBtn, 0.3f, 0, GRAY);
        DrawRectangleRoundedLines(backBtn, 0.3f, 6, BLACK);
        DrawTextEx(font, "Return to menu", Vector2{ backBtn.x + backBtn.width / 2 - backSize.x / 2, backBtn.y + backBtn.height / 2 - backSize.y / 2 }, 40, 4, BLACK);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            scores = false;
            scores_loaded = false;
            suppress_menu_click_one_frame = true;
        }
    }
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
    ClearBackground(BLACK);
    DrawTexturePro(menu_background1, background_src, background_disp, origin, 0, RAYWHITE);
    DrawTexturePro(logo, logo_src, logo_disp, origin, 0, RAYWHITE);

    Rectangle panel = { 360, 450, 1200, 550 };
    DrawRectangleRounded(panel, 0.2f, 0, RAYWHITE);
    DrawRectangleRoundedLines(panel, 0.2f, 6, BLACK);

    Vector2 settingsTitle = MeasureTextEx(font, "Settings", 60, 6);
    DrawTextEx(font, "Settings", Vector2{ panel.x + panel.width / 2 - settingsTitle.x / 2, panel.y + 30 }, 60, 6, BLACK);

    float y = panel.y + 120;
    float xLabel = panel.x + 40;
    float xSlider = panel.x + 340;
    float sliderW = panel.width - 380;

    DrawTextEx(font, "Master Volume", Vector2{ xLabel, y }, 40, 4, BLACK);
    master_volume = SliderHorizontal(Rectangle{ xSlider, y + 50, sliderW, 44 }, master_volume, 0.0f, 1.0f);
    y += 110;
    DrawTextEx(font, TextFormat("Fullscreen (F): %s", fullscreen ? "On" : "Off"), Vector2{ xLabel, y }, 30, 3, BLACK);
    if (IsKeyPressed(KEY_F)) toggle_fullscreen();
    y += 50;
    DrawTextEx(font, TextFormat("Show FPS (G): %s", show_fps ? "Yes" : "No"), Vector2{ xLabel, y }, 30, 3, BLACK);
    if (IsKeyPressed(KEY_G)) toggle_fps();
    y += 50;
    DrawTextEx(font, TextFormat("Resolution (H/J): %dx%d", resolution_index == 0 ? 1280 : resolution_index == 1 ? 1600 : 1920, resolution_index == 0 ? 720 : resolution_index == 1 ? 900 : 1080), Vector2{ xLabel, y }, 30, 3, BLACK);
    if (IsKeyPressed(KEY_H)) resolution_index = (resolution_index + 1) % 3;
    if (IsKeyPressed(KEY_J)) resolution_index = (resolution_index + 2) % 3;

    apply_master_volume(player, level_1, level_2);
    apply_video_settings();

    Rectangle backBtn = { panel.x + panel.width / 2 - 280, panel.y + panel.height - 90, 560, 70 };
    DrawRectangleRounded(backBtn, 0.3f, 0, RAYWHITE);
    DrawRectangleRoundedLines(backBtn, 0.3f, 6, BLACK);

    Vector2 backSize = MeasureTextEx(font, "Return to menu", 40, 4);
    DrawTextEx(font, "Return to menu", Vector2{ backBtn.x + backBtn.width / 2 - backSize.x / 2, backBtn.y + backBtn.height / 2 - backSize.y / 2 }, 40, 4, BLACK);

    if (CheckCollisionPointRec(mouse_pos, backBtn))
    {
        DrawRectangleRounded(backBtn, 0.3f, 0, GRAY);
        DrawRectangleRoundedLines(backBtn, 0.3f, 6, BLACK);
        DrawTextEx(font, "Return to menu", Vector2{ backBtn.x + backBtn.width / 2 - backSize.x / 2, backBtn.y + backBtn.height / 2 - backSize.y / 2 }, 40, 4, BLACK);

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            settings = false;
            suppress_menu_click_one_frame = true;
        }
    }
}

void Menu::check_button()
{
    if (suppress_menu_click_one_frame) {
        suppress_menu_click_one_frame = false;
        return;
    }

    if (!keybindings && !settings && !scores)
    {
        if (CheckCollisionPointRec(mouse_pos, start_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(start_button, 0.3f, 0, GRAY);
            Vector2 ts = MeasureTextEx(font, "Start game", 40, 4);
            DrawTextEx(font, "Start game", Vector2{ start_button.x + start_button.width / 2 - ts.x / 2, start_button.y + start_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) start_game = true;
        }
        else if (CheckCollisionPointRec(mouse_pos, scores_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(scores_button, 0.3f, 0, GRAY);
            Vector2 ts = MeasureTextEx(font, "Scores", 40, 4);
            DrawTextEx(font, "Scores", Vector2{ scores_button.x + scores_button.width / 2 - ts.x / 2, scores_button.y + scores_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                scores = true;
                scores_loaded = false;
            }
        }
        else if (CheckCollisionPointRec(mouse_pos, keybindings_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(keybindings_button, 0.3f, 0, GRAY);
            Vector2 ts = MeasureTextEx(font, "Keybindings", 40, 4);
            DrawTextEx(font, "Keybindings", Vector2{ keybindings_button.x + keybindings_button.width / 2 - ts.x / 2, keybindings_button.y + keybindings_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) keybindings = true;
        }
        else if (CheckCollisionPointRec(mouse_pos, settings_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(settings_button, 0.3f, 0, GRAY);
            Vector2 ts = MeasureTextEx(font, "Settings", 40, 4);
            DrawTextEx(font, "Settings", Vector2{ settings_button.x + settings_button.width / 2 - ts.x / 2, settings_button.y + settings_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) settings = true;
        }
        else if (CheckCollisionPointRec(mouse_pos, exit_button))
        {
            if (play_button_sound) PlaySound(button_sound);
            play_button_sound = false;
            DrawRectangleRounded(exit_button, 0.3f, 0, GRAY);
            Vector2 ts = MeasureTextEx(font, "Exit game", 40, 4);
            DrawTextEx(font, "Exit game", Vector2{ exit_button.x + exit_button.width / 2 - ts.x / 2, exit_button.y + exit_button.height / 2 - ts.y / 2 }, 40, 4, BLACK);
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
            Vector2 ts = MeasureTextEx(font, "Return to menu", 50, 5);
            DrawTextEx(font, "Return to menu", Vector2{ return_button.x + return_button.width / 2 - ts.x / 2, return_button.y + return_button.height / 2 - ts.y / 2 }, 50, 5, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) keybindings = false;
        }
        else play_button_sound = true;
    }
}

void Menu::pause() { if (IsKeyPressed(KEY_ESCAPE)) pause_ = !pause_; }

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

void Menu::ending(Player& player, Levels& level_1, Levels& level_2)
{
    UpdateMusicStream(ending_music);
    ClearBackground(RAYWHITE);
    DrawTextureRec(ending_background, ending_, Vector2{ 0, 0 }, color);

    if (!score_saved && current_user_id != -1) {
        DbContext db;
        if (db.Connect()) {
            db.InsertScore(current_user_id, player.score, 2, true);
            db.Disconnect();
        }
        score_saved = true;
    }

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
            score_saved = false;
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
#include"Menu.h"
#include"Enemies.h"

int main()
{
	InitWindow(0, 0, "Jaime Bross Game");
	ToggleFullscreen();
	InitAudioDevice();
	SetExitKey(KEY_NULL);
	SetTargetFPS(60);

	Image icon = LoadImage("resources/icon.png");
	SetWindowIcon(icon);

	Menu menu;
	Levels level_1(1);
	Levels level_2(2);
	Player player;
	Enemies enemies;

	PlayMusicStream(menu.menu_music);

	while (menu.init)
		menu.init_animation();

	while (!menu.exit_game)
	{
		// Music routing: only one context at a time
		if (!menu.start_game && !level_2.completed)
		{
			// Menu context
			if (!IsMusicStreamPlaying(menu.menu_music)) PlayMusicStream(menu.menu_music);
			StopMusicStream(level_1.level1_music);
			StopMusicStream(level_2.level2_music);
			StopMusicStream(menu.ending_music);
			UpdateMusicStream(menu.menu_music);
		}
		else if (menu.start_game && !level_2.completed)
		{
			StopMusicStream(menu.menu_music);
			StopMusicStream(menu.ending_music);
			if (!level_1.completed)
			{
				if (!IsMusicStreamPlaying(level_1.level1_music)) PlayMusicStream(level_1.level1_music);
				StopMusicStream(level_2.level2_music);
				UpdateMusicStream(level_1.level1_music);
			}
			else
			{
				if (!IsMusicStreamPlaying(level_2.level2_music)) PlayMusicStream(level_2.level2_music);
				StopMusicStream(level_1.level1_music);
				UpdateMusicStream(level_2.level2_music);
			}
		}
		else if (level_2.completed)
		{
			StopMusicStream(menu.menu_music);
			StopMusicStream(level_1.level1_music);
			StopMusicStream(level_2.level2_music);
			if (!IsMusicStreamPlaying(menu.ending_music)) PlayMusicStream(menu.ending_music);
			UpdateMusicStream(menu.ending_music);
		}

		BeginDrawing();

		if (!menu.start_game)
		{
			if (menu.settings)
				menu.draw_settings(player, level_1, level_2);
			else if (!menu.keybindings)
				menu.draw();
			else
				menu.draw_keybindings();
			menu.check_button();
			player.set_pos1();
			menu.reset_lvl(level_1, level_2);
			enemies.reset_enemies();
			menu.set_audio_volume();
			level_1.set_audio_volume();
			level_2.set_audio_volume();
			player.set_audio_volume();
		}
		if (menu.start_game && !level_2.completed)
		{
			BeginMode2D(player.camera);

			ClearBackground(SKYBLUE);
			if (!level_1.completed)
				level_1.draw(player);
			if (level_1.completed)
			{
				level_2.draw(player);
				level_2.draw_saw();
			}
			player.draw_ui();
			if (!menu.pause_)
			{
				player.check_input();
				player.manage_input();
				if (!level_1.completed)
				{
					enemies.manage_birds();
					enemies.manage_flames1();
					level_1.check_collisions(player, player.animations, enemies);
					if (player.death)
					{
						level_1.manage_death(player, enemies);
						if (player.lives <= 0)
						{
							menu.reset_lvl(level_1, level_2);
							level_1.completed = false;
							player.set_pos1();
							enemies.reset_enemies();
							level_1.reset_music();
						}
					}
				}
				if (level_1.completed)
				{
					enemies.manage_spiders();
					level_2.manage_saw();
					enemies.manage_flames2();
					level_2.check_collisions(player, player.animations, enemies);
					if (player.death)
					{
						level_2.manage_death(player, enemies);
						if (player.lives <= 0)
						{
							menu.reset_lvl(level_1, level_2);
							level_1.completed = false;
							player.set_pos1();
							level_2.reset_music();
							level_1.reset_music();
						}
					}
				}
			}
			menu.pause();
			menu.pause_draw(player, level_1, level_2);

			EndMode2D();
		}
		if (level_2.completed)
			menu.ending(level_1, level_2);
		EndDrawing();
	}

	CloseAudioDevice();
	CloseWindow();
	return 0;
}
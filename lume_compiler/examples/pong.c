/* Generated automatically by Lume Transpiler */
#include <raylib.h>
#include <stdio.h>

struct Ball {
    int x;
    int y;
    int dx;
    int dy;
    int radius;
};

struct Paddle {
    int x;
    int y;
    int width;
    int height;
    int speed;
    int score;
};

int check_collision(int bx, int by, int br, int px, int py, int pw, int ph);
int main(void);

int check_collision(int bx, int by, int br, int px, int py, int pw, int ph) {
    if ((((((bx + br) >= px) && ((bx - br) <= (px + pw))) && ((by + br) >= py)) && ((by - br) <= (py + ph)))) {
        return 1;
    }
    return 0;
}

int main(void) {
    int screen_width = 800;
    int screen_height = 500;
    InitWindow(screen_width, screen_height, "Lume Pong - Dogfooding Raylib");
    SetTargetFPS(60);
    int color_bg = 336860415;
    int color_white = (-1);
    int color_player = 1000000000;
    int color_ai = 15103743;
    int color_ball = (-65281);
    int color_gray = 1000000255;
    int KEY_W = 87;
    int KEY_S = 83;
    int KEY_UP = 265;
    int KEY_DOWN = 264;
    int KEY_R = 82;
    struct Ball ball;
    (ball.x = (screen_width / 2));
    (ball.y = (screen_height / 2));
    (ball.dx = 5);
    (ball.dy = 5);
    (ball.radius = 8);
    struct Paddle player;
    (player.x = 30);
    (player.y = ((screen_height / 2) - 45));
    (player.width = 15);
    (player.height = 90);
    (player.speed = 6);
    (player.score = 0);
    struct Paddle ai;
    (ai.x = (screen_width - 45));
    (ai.y = ((screen_height / 2) - 45));
    (ai.width = 15);
    (ai.height = 90);
    (ai.speed = 5);
    (ai.score = 0);
    char score_text[64];
    while ((!WindowShouldClose())) {
        if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))) {
            if ((player.y > 10)) {
                (player.y = (player.y - player.speed));
            }
        }
        if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))) {
            if (((player.y + player.height) < (screen_height - 10))) {
                (player.y = (player.y + player.speed));
            }
        }
        if (((ai.y + (ai.height / 2)) < (ball.y - 15))) {
            if (((ai.y + ai.height) < (screen_height - 10))) {
                (ai.y = (ai.y + ai.speed));
            }
        }
        if (((ai.y + (ai.height / 2)) > (ball.y + 15))) {
            if ((ai.y > 10)) {
                (ai.y = (ai.y - ai.speed));
            }
        }
        (ball.x = (ball.x + ball.dx));
        (ball.y = (ball.y + ball.dy));
        if ((((ball.y - ball.radius) <= 0) || ((ball.y + ball.radius) >= screen_height))) {
            (ball.dy = (-ball.dy));
        }
        if (check_collision(ball.x, ball.y, ball.radius, player.x, player.y, player.width, player.height)) {
            (ball.dx = 5);
            (ball.x = ((player.x + player.width) + ball.radius));
        }
        if (check_collision(ball.x, ball.y, ball.radius, ai.x, ai.y, ai.width, ai.height)) {
            (ball.dx = (-5));
            (ball.x = (ai.x - ball.radius));
        }
        if ((ball.x > screen_width)) {
            (player.score = (player.score + 1));
            (ball.x = (screen_width / 2));
            (ball.y = (screen_height / 2));
            (ball.dx = (-5));
        }
        if ((ball.x < 0)) {
            (ai.score = (ai.score + 1));
            (ball.x = (screen_width / 2));
            (ball.y = (screen_height / 2));
            (ball.dx = 5);
        }
        if (IsKeyDown(KEY_R)) {
            (player.score = 0);
            (ai.score = 0);
            (ball.x = (screen_width / 2));
            (ball.y = (screen_height / 2));
        }
        BeginDrawing();
        ClearBackground(GetColor(color_bg));
        int line_y = 0;
        while ((line_y < screen_height)) {
            DrawRectangle(((screen_width / 2) - 2), line_y, 4, 15, GetColor(color_gray));
            (line_y = (line_y + 30));
        }
        DrawRectangle(player.x, player.y, player.width, player.height, GetColor(color_player));
        DrawRectangle(ai.x, ai.y, ai.width, ai.height, GetColor(color_ai));
        DrawCircle(ball.x, ball.y, ball.radius, GetColor(color_ball));
        sprintf(score_text, "%d   :   %d", player.score, ai.score);
        DrawText(score_text, ((screen_width / 2) - 55), 25, 36, GetColor(color_white));
        DrawText("PLAYER (W/S)", 40, 20, 16, GetColor(color_white));
        DrawText("AI OPPONENT", (screen_width - 150), 20, 16, GetColor(color_white));
        DrawText("Press 'R' to Reset", ((screen_width / 2) - 60), (screen_height - 30), 14, GetColor(color_gray));
        EndDrawing();
    }
    CloseWindow();
    return 0;
}


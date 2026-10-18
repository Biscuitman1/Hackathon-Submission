#include "raylib.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define SCREEN_W 800
#define SCREEN_H 600
#define TILE 32
#define MAX_BULLETS 64
#define MAX_ENEMIES 8
#define MAX_COINS 8
#define MAX_PUMPKINS 3
#define MAX_BOSS_BULLETS 32
#define BOSS_HP_MAX 100
#define MAX_BOSSES 15
#define MAX_BOSS_GENERATION 3

typedef struct { Vector2 pos; Vector2 vel; bool active; } Bullet;
typedef struct { Vector2 pos; float speed; bool isGhost; bool active; } Enemy;
typedef struct { Vector2 pos; bool active; } Coin;
typedef struct { Vector2 pos; bool active; } Pumpkin;
typedef struct { Vector2 pos; Vector2 vel; int hp; int generation; float fireTimer; bool active; } Boss;

typedef enum { STATE_PLAYING, STATE_TREAT, STATE_TRICK, STATE_BOSS, STATE_WIN } GameState;

int hasHat = 0;
int speedBoost = 0;
int doubleGun = 0;

void DrawZombie(Vector2 pos) {
    DrawRectangle(pos.x - 8, pos.y - 4, 16, 16, (Color){80, 140, 60, 255});
    DrawCircle(pos.x, pos.y - 8, 7, (Color){110, 170, 80, 255});
    DrawCircle(pos.x - 3, pos.y - 9, 2, RED);
    DrawCircle(pos.x + 3, pos.y - 9, 2, RED);
    DrawRectangle(pos.x - 12, pos.y - 2, 4, 10, (Color){80, 140, 60, 255});
    DrawRectangle(pos.x + 8, pos.y - 2, 4, 10, (Color){80, 140, 60, 255});
}

void DrawGhost(Vector2 pos) {
    Color ghostCol = (Color){180, 200, 255, 160};
    DrawCircle(pos.x, pos.y - 2, 10, ghostCol);
    DrawRectangle(pos.x - 10, pos.y - 2, 20, 12, ghostCol);
    DrawCircle(pos.x - 7, pos.y + 10, 4, ghostCol);
    DrawCircle(pos.x,     pos.y + 10, 4, ghostCol);
    DrawCircle(pos.x + 7, pos.y + 10, 4, ghostCol);
    DrawCircle(pos.x - 4, pos.y - 4, 2, BLACK);
    DrawCircle(pos.x + 4, pos.y - 4, 2, BLACK);
}

void DrawCandy(Vector2 pos) {
    Color candyCol = (Color){255, 120, 180, 255};
    DrawTriangle(
        (Vector2){pos.x - 14, pos.y - 6},
        (Vector2){pos.x - 14, pos.y + 6},
        (Vector2){pos.x - 4,  pos.y},
        candyCol
    );
    DrawTriangle(
        (Vector2){pos.x + 14, pos.y - 6},
        (Vector2){pos.x + 14, pos.y + 6},
        (Vector2){pos.x + 4,  pos.y},
        candyCol
    );
    DrawCircle(pos.x, pos.y, 8, candyCol);
    DrawRectangle(pos.x - 8, pos.y - 2, 16, 4, WHITE);
}

void DrawPumpkin(Vector2 pos) {
    Color orange = (Color){240, 120, 30, 255};
    Color darkOrange = (Color){200, 90, 20, 255};
    DrawEllipse(pos.x, pos.y, 14, 12, orange);
    DrawLine(pos.x - 5, pos.y - 11, pos.x - 5, pos.y + 11, darkOrange);
    DrawLine(pos.x + 5, pos.y - 11, pos.x + 5, pos.y + 11, darkOrange);
    DrawRectangle(pos.x - 2, pos.y - 16, 4, 6, (Color){60, 120, 40, 255});
    DrawTriangle(
        (Vector2){pos.x - 8, pos.y - 6},
        (Vector2){pos.x - 3, pos.y - 6},
        (Vector2){pos.x - 5, pos.y - 1},
        BLACK
    );
    DrawTriangle(
        (Vector2){pos.x + 3, pos.y - 6},
        (Vector2){pos.x + 8, pos.y - 6},
        (Vector2){pos.x + 5, pos.y - 1},
        BLACK
    );
    DrawRectangle(pos.x - 6, pos.y + 3, 12, 3, BLACK);
    DrawRectangle(pos.x - 4, pos.y + 6, 2, 2, BLACK);
    DrawRectangle(pos.x,     pos.y + 6, 2, 2, BLACK);
    DrawRectangle(pos.x + 4, pos.y + 6, 2, 2, BLACK);
}

// Draw a pumpkin boss. Each generation is rendered smaller than its parent.
void DrawBoss(Vector2 pos, float scale) {
    Color orange = (Color){230, 90, 20, 255};
    Color darkOrange = (Color){170, 60, 10, 255};
    DrawEllipse(pos.x, pos.y, 60 * scale, 50 * scale, orange);
    DrawLine(pos.x - 20 * scale, pos.y - 48 * scale, pos.x - 20 * scale, pos.y + 48 * scale, darkOrange);
    DrawLine(pos.x + 20 * scale, pos.y - 48 * scale, pos.x + 20 * scale, pos.y + 48 * scale, darkOrange);
    DrawRectangle(pos.x - 6 * scale, pos.y - 62 * scale, 12 * scale, 14 * scale, (Color){60, 120, 40, 255});
    DrawPoly((Vector2){pos.x - 22 * scale, pos.y - 16 * scale}, 3, 14 * scale, 15, BLACK);
    DrawPoly((Vector2){pos.x + 22 * scale, pos.y - 16 * scale}, 3, 14 * scale, -15, BLACK);
    DrawCircle(pos.x - 20 * scale, pos.y - 16 * scale, 3 * scale, RED);
    DrawCircle(pos.x + 20 * scale, pos.y - 16 * scale, 3 * scale, RED);
    DrawRectangle(pos.x - 30 * scale, pos.y + 8 * scale, 60 * scale, 8 * scale, BLACK);
    for (int i = -30; i < 30; i += 12)
        DrawRectangle(pos.x + i * scale, pos.y + 16 * scale, 6 * scale, 6 * scale, BLACK);
    DrawCircle(pos.x - 50 * scale, pos.y - 10 * scale, 2 * scale, (Color){120, 255, 120, 200});
    DrawCircle(pos.x + 50 * scale, pos.y - 10 * scale, 2 * scale, (Color){120, 255, 120, 200});
}

// ============================================================
//                       PLAYER CHARACTER
// ============================================================
void DrawPlayer(Vector2 pos, int facing, bool throwing, int hasHat) {
    Color cloak      = (Color){ 60,  40, 120, 255};
    Color cloakShade = (Color){ 40,  25,  85, 255};
    Color face       = (Color){255, 220, 180, 255};
    Color bagCol     = (Color){200, 160,  60, 255};
    Color rockCol    = (Color){255, 220,  60, 255};

    // shadow
    DrawEllipse(pos.x, pos.y + 20, 14, 4, (Color){0, 0, 0, 110});

    // body / cloak
    DrawRectangle(pos.x - 10, pos.y - 4, 20, 22, cloak);
    DrawRectangle(pos.x - 12, pos.y + 14, 24, 4, cloakShade); // hem

    // hood
    DrawCircle(pos.x, pos.y - 10, 12, cloak);
    DrawCircle(pos.x - 3, pos.y - 13, 6, cloakShade);

    // face
    DrawCircle(pos.x, pos.y - 8, 7, face);

    // eyes track facing
    int ex = 0, ey = 0;
    if (facing == 2) ex = -2;
    if (facing == 3) ex =  2;
    if (facing == 0) ey = -2;
    if (facing == 1) ey =  2;
    DrawCircle(pos.x - 3 + ex, pos.y - 9 + ey, 1.5f, BLACK);
    DrawCircle(pos.x + 3 + ex, pos.y - 9 + ey, 1.5f, BLACK);

    // bag + throwing hand depend on facing
    Vector2 bagPos  = { pos.x, pos.y };
    Vector2 handPos = { pos.x, pos.y };
    switch (facing) {
        case 0: // up
            bagPos  = (Vector2){ pos.x + 10, pos.y - 2 };
            handPos = throwing ? (Vector2){ pos.x + 6,  pos.y - 24 }
                               : (Vector2){ pos.x + 8,  pos.y - 4 };
            break;
        case 1: // down
            bagPos  = (Vector2){ pos.x - 12, pos.y + 6 };
            handPos = throwing ? (Vector2){ pos.x + 4,  pos.y + 20 }
                               : (Vector2){ pos.x + 10, pos.y + 6 };
            break;
        case 2: // left
            bagPos  = (Vector2){ pos.x + 12, pos.y + 6 };
            handPos = throwing ? (Vector2){ pos.x - 22, pos.y - 6 }
                               : (Vector2){ pos.x - 10, pos.y + 2 };
            break;
        case 3: // right
            bagPos  = (Vector2){ pos.x - 12, pos.y + 6 };
            handPos = throwing ? (Vector2){ pos.x + 22, pos.y - 6 }
                               : (Vector2){ pos.x + 10, pos.y + 2 };
            break;
    }

    // bag of candy rocks
    DrawCircle(bagPos.x, bagPos.y, 7, bagCol);
    DrawRectangle(bagPos.x - 5, bagPos.y - 8, 10, 3, cloakShade);
    DrawCircle(bagPos.x - 2, bagPos.y - 8, 2, rockCol);
    DrawCircle(bagPos.x + 2, bagPos.y - 8, 2, rockCol);

    // throwing arm
    DrawLineEx((Vector2){ pos.x, pos.y - 2 }, handPos, 3, face);
    DrawCircle(handPos.x, handPos.y, 3, face);

    // rock + motion streak while throwing
    if (throwing) {
        DrawCircle(handPos.x, handPos.y, 4, rockCol);
        DrawLineEx(handPos,
                   (Vector2){ (handPos.x + pos.x) / 2, (handPos.y + pos.y) / 2 },
                   2, (Color){ 255, 220, 60, 120 });
    }

    // hat
    if (hasHat == 1) DrawRectangle(pos.x - 10, pos.y - 24, 20, 6, RED);
    if (hasHat == 2) DrawRectangle(pos.x - 10, pos.y - 24, 20, 6, BLUE);
    if (hasHat == 3) DrawRectangle(pos.x - 10, pos.y - 24, 20, 6, GREEN);
}

int dodgeCount[4] = {0,0,0,0};

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "ML 3RD PERSON Shooter");
    SetTargetFPS(60);

    Vector2 player = { SCREEN_W/2.0f, SCREEN_H/2.0f };
    float playerSize = TILE * 2;
    int hp = 10;
    int score = 0;
    int round = 1;
    int facing = 3;

    // Throw animation state
    float throwTimer = 0.0f;
    const float THROW_DURATION = 0.18f;

    Bullet bullets[MAX_BULLETS] = {0};
    Enemy enemies[MAX_ENEMIES] = {0};
    Coin coins[MAX_COINS] = {0};
    Pumpkin pumpkins[MAX_PUMPKINS] = {0};
    Bullet bossBullets[MAX_BOSS_BULLETS] = {0};

    GameState state = STATE_PLAYING;
    int selectedDoor = 1;

    Boss bosses[MAX_BOSSES] = {0};
    const float BOSS_FIRE_INTERVAL = 1.2f;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
        enemies[i].speed = 0.6f + (i % 3) * 0.3f;
        enemies[i].isGhost = (i % 2 == 0);
        enemies[i].active = true;
    }
    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
        coins[i].active = true;
    }
    for (int i = 0; i < MAX_PUMPKINS; i++) {
        pumpkins[i].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
        pumpkins[i].active = true;
    }

    while (!WindowShouldClose()) {

        // ================= INTERMISSION =================
        if (state == STATE_TREAT || state == STATE_TRICK) {
            if (IsKeyPressed(KEY_A)) selectedDoor = 0;
            if (IsKeyPressed(KEY_S)) selectedDoor = 1;
            if (IsKeyPressed(KEY_D)) selectedDoor = 2;
            if (IsKeyPressed(KEY_P)) hp = 100;

            if (IsKeyPressed(KEY_ENTER)) {
                if (state == STATE_TREAT) {
                    hasHat = selectedDoor + 1;
                } else {
                    if (selectedDoor == 0) speedBoost += 2;
                    if (selectedDoor == 1) doubleGun = 1;
                    if (selectedDoor == 2) { hp += 2; if (hp > 5) hp = 5; }
                }
                state = STATE_PLAYING;
            }

            BeginDrawing();
            ClearBackground((Color){20, 10, 30, 255});

            const char *title = (state == STATE_TREAT) ? "TREAT" : "TRICK";
            DrawText(title, SCREEN_W/2 - MeasureText(title, 50)/2, 60, 50, GOLD);

            const char *labels[3];
            if (state == STATE_TREAT) {
                labels[0] = "Red Hat"; labels[1] = "Blue Hat"; labels[2] = "Green Hat";
            } else {
                labels[0] = "Speed Boost"; labels[1] = "Double Gun"; labels[2] = "Extra HP";
            }

            for (int i = 0; i < 3; i++) {
                int dx = 150 + i * 200;
                int dy = 250;
                Color frame = (i == selectedDoor) ? YELLOW : GRAY;
                DrawRectangle(dx, dy, 120, 180, (Color){60, 40, 80, 255});
                DrawRectangleLinesEx((Rectangle){dx, dy, 120, 180}, 4, frame);
                DrawCircle(dx + 100, dy + 90, 6, GOLD);
                DrawText(labels[i], dx + 10, dy + 190, 18, WHITE);
                DrawText(i == 0 ? "A" : i == 1 ? "S" : "D", dx + 55, dy + 155, 24, frame);
            }

            DrawText("Press A/S/D to choose, ENTER to confirm", SCREEN_W/2 - 220, 480, 20, LIGHTGRAY);
            EndDrawing();
            continue;
        }

        // ================= WIN SCREEN =================
        if (state == STATE_WIN) {
            BeginDrawing();
            ClearBackground((Color){10, 20, 10, 255});
            const char *msg = "HAPPY HALLOWEEN, GO COUGS!!!";
            DrawText(msg, SCREEN_W/2 - MeasureText(msg, 30)/2, 250, 30, GOLD);
            DrawText(TextFormat("Final Score: %d", score), SCREEN_W/2 - 100, 300, 24, WHITE);
            DrawText("Press ESC to quit", SCREEN_W/2 - 100, 350, 20, GRAY);
            EndDrawing();
            continue;
        }

        // ================= PLAYING / BOSS =================
        bool bossActive = (state == STATE_BOSS);

        // Tick throw animation
        if (throwTimer > 0.0f) throwTimer -= GetFrameTime();

        Vector2 oldPos = player;
        float moveSpeed = 3.0f + speedBoost;
        if (IsKeyDown(KEY_W)) { player.y -= moveSpeed; facing = 0; }
        if (IsKeyDown(KEY_S)) { player.y += moveSpeed; facing = 1; }
        if (IsKeyDown(KEY_A)) { player.x -= moveSpeed; facing = 2; }
        if (IsKeyDown(KEY_D)) { player.x += moveSpeed; facing = 3; }

        if (player.x != oldPos.x || player.y != oldPos.y) {
            dodgeCount[facing]++;
        }

        if (player.x < 0) player.x = 0;
        if (player.y < 0) player.y = 0;
        if (player.x > SCREEN_W - playerSize) player.x = SCREEN_W - playerSize;
        if (player.y > SCREEN_H - playerSize) player.y = SCREEN_H - playerSize;

        // Shoot
        if (IsKeyPressed(KEY_SPACE)) {
            throwTimer = THROW_DURATION;
            int fired = 0;
            for (int i = 0; i < MAX_BULLETS && fired < (doubleGun ? 2 : 1); i++) {
                if (!bullets[i].active) {
                    bullets[i].pos = (Vector2){ player.x + playerSize/2, player.y + playerSize/2 };
                    if (doubleGun && fired == 1) {
                        if (facing == 0 || facing == 1) bullets[i].pos.x += 10;
                        else bullets[i].pos.y += 10;
                    }
                    float spd = 8.0f;
                    if (facing == 0) bullets[i].vel = (Vector2){0, -spd};
                    if (facing == 1) bullets[i].vel = (Vector2){0, spd};
                    if (facing == 2) bullets[i].vel = (Vector2){-spd, 0};
                    if (facing == 3) bullets[i].vel = (Vector2){spd, 0};
                    bullets[i].active = true;
                    fired++;
                }
            }
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                bullets[i].pos.x += bullets[i].vel.x;
                bullets[i].pos.y += bullets[i].vel.y;
                if (bullets[i].pos.x < 0 || bullets[i].pos.x > SCREEN_W ||
                    bullets[i].pos.y < 0 || bullets[i].pos.y > SCREEN_H)
                    bullets[i].active = false;
            }
        }

        // Learned dodge bias
        int mostCommon = 0;
        for (int i = 1; i < 4; i++) if (dodgeCount[i] > dodgeCount[mostCommon]) mostCommon = i;
        Vector2 bias = {0,0};
        if (mostCommon == 0) bias = (Vector2){0, -1};
        if (mostCommon == 1) bias = (Vector2){0, 1};
        if (mostCommon == 2) bias = (Vector2){-1, 0};
        if (mostCommon == 3) bias = (Vector2){1, 0};

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            Vector2 dir = { player.x - enemies[i].pos.x, player.y - enemies[i].pos.y };
            float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
            if (len > 1) { dir.x /= len; dir.y /= len; }
            dir.x += bias.x * 0.3f;
            dir.y += bias.y * 0.3f;
            enemies[i].pos.x += dir.x * enemies[i].speed;
            enemies[i].pos.y += dir.y * enemies[i].speed;

            Rectangle enemyRect = { enemies[i].pos.x-8, enemies[i].pos.y-8, 16, 16 };
            Rectangle playerRect = { player.x, player.y, playerSize, playerSize };
            if (CheckCollisionRecs(enemyRect, playerRect)) {
                hp--;
                enemies[i].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
            }
        }

        for (int b = 0; b < MAX_BULLETS; b++) {
            if (!bullets[b].active) continue;
            Rectangle bulletRect = { bullets[b].pos.x-3, bullets[b].pos.y-3, 6, 6 };
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (!enemies[e].active) continue;
                Rectangle enemyRect = { enemies[e].pos.x-8, enemies[e].pos.y-8, 16, 16 };
                if (CheckCollisionRecs(bulletRect, enemyRect)) {
                    bullets[b].active = false;
                    score += 10;
                    enemies[e].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
                    break;
                }
            }
            if (bossActive && bullets[b].active) {
                for (int k = 0; k < MAX_BOSSES; k++) {
                    if (!bosses[k].active) continue;
                    float scale = 1.0f / (1 << bosses[k].generation);
                    Rectangle bossRect = {bosses[k].pos.x - 60 * scale, bosses[k].pos.y - 50 * scale,
                                          120 * scale, 100 * scale};
                    if (CheckCollisionRecs(bulletRect, bossRect)) {
                        bullets[b].active = false;
                        bosses[k].hp -= 10;
                        if (bosses[k].hp <= 0) {
                            score += 100 * (MAX_BOSS_GENERATION - bosses[k].generation + 1);
                            Vector2 deadPos = bosses[k].pos;
                            int nextGeneration = bosses[k].generation + 1;
                            bosses[k].active = false;
                            if (nextGeneration <= MAX_BOSS_GENERATION) {
                                for (int child = 0; child < 2; child++) {
                                    for (int slot = 0; slot < MAX_BOSSES; slot++) {
                                        if (!bosses[slot].active) {
                                            float offset = (child == 0 ? -1.0f : 1.0f);
                                            bosses[slot].pos = (Vector2){
                                                deadPos.x + offset * 35.0f / (1 << nextGeneration),
                                                deadPos.y + offset * 20.0f / (1 << nextGeneration)};
                                            bosses[slot].vel = (Vector2){0, 0};
                                            bosses[slot].hp = BOSS_HP_MAX / (1 << nextGeneration);
                                            if (bosses[slot].hp < 5) bosses[slot].hp = 5;
                                            bosses[slot].generation = nextGeneration;
                                            bosses[slot].fireTimer = 0.0f;
                                            bosses[slot].active = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

        for (int c = 0; c < MAX_COINS; c++) {
            if (!coins[c].active) continue;
            Rectangle coinRect = { coins[c].pos.x-8, coins[c].pos.y-8, 16, 16 };
            Rectangle playerRect = { player.x, player.y, playerSize, playerSize };
            if (CheckCollisionRecs(coinRect, playerRect)) {
                coins[c].active = false;
                score += 5;
                coins[c].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
                coins[c].active = true;
            }
        }

        for (int p = 0; p < MAX_PUMPKINS; p++) {
            if (!pumpkins[p].active) continue;
            Rectangle pumpRect = { pumpkins[p].pos.x-14, pumpkins[p].pos.y-12, 28, 24 };
            Rectangle playerRect = { player.x, player.y, playerSize, playerSize };
            if (CheckCollisionRecs(pumpRect, playerRect)) {
                if (hp < 5) hp++;
                score += 5;
                pumpkins[p].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
                pumpkins[p].active = true;
            }
        }

        // Boss movement, adaptation, and shooting.
        if (bossActive) {
            int totalDodges = dodgeCount[0] + dodgeCount[1] + dodgeCount[2] + dodgeCount[3];
            for (int k = 0; k < MAX_BOSSES; k++) {
                if (!bosses[k].active) continue;
                float scale = 1.0f / (1 << bosses[k].generation);
                float moveSpeed = 0.75f + 0.25f * bosses[k].generation;
                Vector2 dir = {player.x + playerSize / 2 - bosses[k].pos.x,
                               player.y + playerSize / 2 - bosses[k].pos.y};
                float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
                if (len > 0.001f) { dir.x /= len; dir.y /= len; }

                if (totalDodges > 0) {
                    float learningWeight = totalDodges >= 20 ? 0.65f : 0.25f + totalDodges * 0.02f;
                    dir.x -= bias.x * learningWeight;
                    dir.y -= bias.y * learningWeight;
                    float adjustedLen = sqrtf(dir.x * dir.x + dir.y * dir.y);
                    if (adjustedLen > 0.001f) { dir.x /= adjustedLen; dir.y /= adjustedLen; }
                }
                bosses[k].vel = (Vector2){dir.x * moveSpeed, dir.y * moveSpeed};
                bosses[k].pos.x += bosses[k].vel.x;
                bosses[k].pos.y += bosses[k].vel.y;

                float marginX = 60 * scale, marginY = 50 * scale;
                if (bosses[k].pos.x < marginX) bosses[k].pos.x = marginX;
                if (bosses[k].pos.x > SCREEN_W - marginX) bosses[k].pos.x = SCREEN_W - marginX;
                if (bosses[k].pos.y < marginY) bosses[k].pos.y = marginY;
                if (bosses[k].pos.y > SCREEN_H - marginY) bosses[k].pos.y = SCREEN_H - marginY;

                bosses[k].fireTimer += GetFrameTime();
                if (bosses[k].fireTimer >= BOSS_FIRE_INTERVAL) {
                    bosses[k].fireTimer = 0.0f;
                    for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
                        if (!bossBullets[i].active) {
                            bossBullets[i].pos = bosses[k].pos;
                            Vector2 shotDir = {player.x + playerSize / 2 - bosses[k].pos.x,
                                               player.y + playerSize / 2 - bosses[k].pos.y};
                            float shotLen = sqrtf(shotDir.x * shotDir.x + shotDir.y * shotDir.y);
                            if (shotLen > 0) { shotDir.x /= shotLen; shotDir.y /= shotLen; }
                            bossBullets[i].vel = (Vector2){shotDir.x * 3.5f, shotDir.y * 3.5f};
                            bossBullets[i].active = true;
                            break;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
            if (!bossBullets[i].active) continue;
            bossBullets[i].pos.x += bossBullets[i].vel.x;
            bossBullets[i].pos.y += bossBullets[i].vel.y;
            if (bossBullets[i].pos.x < -20 || bossBullets[i].pos.x > SCREEN_W + 20 ||
                bossBullets[i].pos.y < -20 || bossBullets[i].pos.y > SCREEN_H + 20) {
                bossBullets[i].active = false;
                continue;
            }
            Rectangle bbRect = {bossBullets[i].pos.x - 5, bossBullets[i].pos.y - 5, 10, 10};
            Rectangle playerRect = {player.x, player.y, playerSize, playerSize};
            if (CheckCollisionRecs(bbRect, playerRect)) {
                bossBullets[i].active = false;
                hp--;
            }
        }

        // Win only after every split pumpkin has been defeated.
        if (bossActive) {
            bool anyBossAlive = false;
            for (int i = 0; i < MAX_BOSSES; i++)
                if (bosses[i].active) { anyBossAlive = true; break; }
            if (!anyBossAlive) state = STATE_WIN;
        }

        // Round advance (normal rounds only)
        if (state == STATE_PLAYING && score >= round * 100) {
            round++;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                enemies[i].active = true;
                enemies[i].speed += 0.1f;
            }
            if (round == 2)      { state = STATE_TREAT; selectedDoor = 1; }
            else if (round == 3) { state = STATE_TRICK; selectedDoor = 1; }
            else if (round >= 4) {
                state = STATE_BOSS;
                for (int i = 0; i < MAX_BOSSES; i++) bosses[i].active = false;
                bosses[0].pos = (Vector2){SCREEN_W / 2.0f, SCREEN_H / 2.0f};
                bosses[0].vel = (Vector2){0, 0};
                bosses[0].hp = BOSS_HP_MAX;
                bosses[0].generation = 0;
                bosses[0].fireTimer = 0.0f;
                bosses[0].active = true;
            }
        }

        if (hp <= 0) {
            hp = 5;
            score = 0;
            round = 1;
            state = STATE_PLAYING;
            hasHat = 0;
            speedBoost = 0;
            doubleGun = 0;
            throwTimer = 0.0f;
            for (int i = 0; i < MAX_BOSSES; i++) bosses[i].active = false;
            for (int i = 0; i < 3; i++) dodgeCount[i] = 0;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                enemies[i].pos = (Vector2){ rand() % SCREEN_W, rand() % SCREEN_H };
                enemies[i].speed = 0.6f + (i % 3) * 0.3f;
                enemies[i].active = true;
            }
            for (int i = 0; i < MAX_BOSS_BULLETS; i++) bossBullets[i].active = false;
        }

        // ================= DRAW =================
        BeginDrawing();
        ClearBackground(bossActive ? (Color){30, 5, 5, 255} : BLACK);

        for (int i = 0; i < MAX_COINS; i++) {
            if (coins[i].active) DrawCandy(coins[i].pos);
        }

        for (int i = 0; i < MAX_PUMPKINS; i++) {
            if (pumpkins[i].active) DrawPumpkin(pumpkins[i].pos);
        }

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            if (enemies[i].isGhost)
                DrawGhost(enemies[i].pos);
            else
                DrawZombie(enemies[i].pos);
        }

        if (bossActive) {
            for (int k = 0; k < MAX_BOSSES; k++) {
                if (!bosses[k].active) continue;
                float scale = 1.0f / (1 << bosses[k].generation);
                DrawBoss(bosses[k].pos, scale);
                float barWidth = 50 * scale;
                float maxHp = (float)(BOSS_HP_MAX / (1 << bosses[k].generation));
                float ratio = bosses[k].hp / maxHp;
                if (ratio > 1.0f) ratio = 1.0f;
                DrawRectangle(bosses[k].pos.x - barWidth / 2, bosses[k].pos.y - 75 * scale,
                              barWidth, 5 * scale, (Color){60, 20, 20, 255});
                DrawRectangle(bosses[k].pos.x - barWidth / 2, bosses[k].pos.y - 75 * scale,
                              barWidth * ratio, 5 * scale, RED);
            }
            DrawText("SUPER EVIL PUMPKIN GUY", SCREEN_W/2 - 135, 65, 20, RED);
        }

        // Player bullets (yellow)
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) DrawCircleV(bullets[i].pos, 3, YELLOW);
        }

        // Boss bullets (red)
        for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
            if (bossBullets[i].active) DrawCircleV(bossBullets[i].pos, 5, RED);
        }

        // ===== Player character (drawn from its center) =====
        DrawPlayer(
            (Vector2){ player.x + playerSize/2, player.y + playerSize/2 },
            facing,
            throwTimer > 0.0f,
            hasHat
        );

        // HUD
        DrawText(TextFormat("HP: %d  Score: %d  Round: %d", hp, score, round), 10, 10, 20, WHITE);
        if (!bossActive) DrawText(TextFormat("Next round at %d", round * 100), 10, 35, 18, GRAY);
        DrawText(TextFormat("Learned dodge: %s",
            mostCommon==0?"UP":mostCommon==1?"DOWN":mostCommon==2?"LEFT":"RIGHT"), 10, 58, 18, SKYBLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
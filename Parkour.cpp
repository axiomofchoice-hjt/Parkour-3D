// #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <list>
#include <queue>
#include <string>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <GL/glut.h>

using namespace std;

#include "imports/basic.h"
#include "imports/const.h"
#include "imports/audio.h"
#include "imports/UI.h"
#include "imports/texture.h"

class BlockProfile { // 方块种类的信息
public:
    string name; // 种类名称
    int texture; // 材质
    BlockProfile(const string& name)
        :name(name), texture(0) {}
};

BlockProfile blockprofile[] = {
    BlockProfile("Normal"), // 0
    BlockProfile("Green"),
    BlockProfile("Orange"), // 2
    BlockProfile("Yellow"),
    BlockProfile("Purple"), // 4
    BlockProfile("Leaves"),
    BlockProfile("Log"), // 6
    BlockProfile("Dirt"),
    BlockProfile("Thorn") // 8
};

const int GreenId = 1, OrangeId = 2, YellowId = 3, PurpleId = 4, ThornId = 8;

enum class GameMode { // 游戏模式
    Adventure,
    Creative
};

class Player { // 玩家
public:
    int eyedistance; // 第三人称摄像头与玩家的距离
    vec3 pos; // 位置
    vec3 look; // 视角方向向量（被经纬度决定）
    vec3 v; // 速度
    vec3 savepoint; // 存档点
    vec3 startpoint; // 起点
    lf lng, lat; // 视角经纬度
    GameMode gamemode; // 游戏模式（冒险 / 创造）
    int blockInHand; // 手中的方块类型
    bool touchfloor; // 是否接触地板
    bool fly; // 是否飞行状态
    bool killed; // 是否死亡
    Player() :pos(), look(), v(), lng(), lat(), touchfloor(), savepoint(),
        eyedistance(3), gamemode(GameMode::Adventure), fly(), blockInHand(),
        killed() {
    }
    void respawn() { // 回到存档点
        pos = savepoint;
        v = vec3();
        killed = false;
    }
    void restart() { // 回到起点
        pos = savepoint = startpoint;
        v = vec3();
        killed = false;
    }
    void draw() { // 绘制玩家模型
        if (eyedistance == 0) return;
        else {
            if (!killed) setMatColor(1, 1, 1, 0, 0, 0);
            else setMatColor(1, 0.3, 0.3, 0, 0, 0);

            vec3 s(block_horizon * 2 - 1, block_up + block_down - 1, block_horizon * 2 - 1);
            s -= vec3(0.1, 0.1, 0.1);
            glPushMatrix();
            glTranslated(pos.x, pos.y + (block_down - block_up) / 2, pos.z);
            glScaled(s.x, s.y, s.z);
            glLineWidth(1);
            glutWireCube(1);

            if ((pos - startpoint).Clen() < 2) { // 在起点附近，绘制方向
                setMatColor(1, 1, 1, 0, 0, 0);
                glPushMatrix();
                glTranslated(0, block_down - 0.1, 0);
                glRotated(-lng, 0, 1, 0);
                DrawLine(vec3(1, 0, 0), vec3(-0.7, 0, 0));
                DrawLine(vec3(0, 0, 0.7), vec3(0, 0, -0.7));
                DrawTriangle(vec3(1, 0, 0), vec3(0.6, 0, -0.18), vec3(0.6, 0, 0.18));
                glPopMatrix();
            }

            glPopMatrix();
        }
    }
};

class Thorn { // 刺，存于 Block::extra
public:
    vec3 top;
    lf r;
    Thorn(const vec3& top, lf r) : top(top), r(r) {
    }
    void trans(const vec3& pos) const {
        vec3 v = (top - pos).trunc(1);
        if (abs(v.x) + abs(v.z) < 0.000001) {
            if (v.y > 0) glRotated(270, 1, 0, 0);
            else glRotated(90, 1, 0, 0);
        }
        else {
            glRotated(atan2(v.x, v.z) * 180 / pi, 0, 1, 0);
            glRotated(-asin(v.y) * 180 / pi, 1, 0, 0);
        }
    }
};

class Block { // 方块
public:
    vec3 pos; // 位置
    int touched; // 被玩家触碰到哪个面
    int Category; // 类型
    void* extra; // 额外信息
    Block(const vec3& pos, int Category) :
        pos(pos),
        touched(-1),
        Category(Category),
        extra() {
    }
    Block(const vec3& pos, int Category, Thorn* extra) :
        pos(pos),
        touched(-1),
        Category(Category),
        extra(extra) {
    }
    ~Block() {
        if (extra != nullptr)
            delete extra;
    }
    const string& name() const {
        return blockprofile[Category].name;
    }

    bool iscoverplayer(const Player& player) const { // 是否遮住玩家
        if (player.eyedistance == 0) return false;
        const vec3 to = player.pos - player.eyedistance * player.look;
        if (Category == ThornId) {
            vec3 X(1, 2.718281828, pi);
            const Thorn& thorn = *((Thorn*)extra);
            vec3 dir = thorn.top - pos;
            X = (X - dir.trunc(dot(dir, X) / dir.len())).trunc(thorn.r);
            vec3 Y = cross(dir, X).trunc(thorn.r);
            for (lf angle = 0; angle < pi; angle += 1) {
                if (islookedtriangleconst(pos + X * cos(angle) + Y * sin(angle), pos - X * cos(angle) - Y * sin(angle), thorn.top, player.pos, to)
                    || islookedtriangleconst(pos + X * cos(angle) + Y * sin(angle), pos - X * cos(angle) - Y * sin(angle), thorn.top, vec3(player.pos.x, player.pos.y - (block_up - block_down), player.pos.z), to)
                    || islookedtriangleconst(pos + X * cos(angle) + Y * sin(angle), pos - X * cos(angle) - Y * sin(angle), thorn.top, vec3(player.pos.x, player.pos.y - (block_up - block_down) / 2, player.pos.z), to)) {
                    return true;
                }
            }
        }
        else {
            if (islookedcubeconst(pos - vec3(0.5, 0.5, 0.5), pos + vec3(0.5, 0.5, 0.5), player.pos, to)
                || islookedcubeconst(pos - vec3(0.5, 0.5, 0.5), pos + vec3(0.5, 0.5, 0.5), vec3(player.pos.x, player.pos.y - (block_up - block_down), player.pos.z), to)
                || islookedcubeconst(pos - vec3(0.5, 0.5, 0.5), pos + vec3(0.5, 0.5, 0.5), vec3(player.pos.x, player.pos.y - (block_up - block_down) / 2, player.pos.z), to)) {
                return true;
            }
        }
        return false;
    }
    void drawTexture() const { // 绘制贴图
        glEnable(GL_TEXTURE_2D);
        static const vec3 A(-0.5, -0.5, -0.5),
            B(-0.5, -0.5, 0.5),
            C(0.5, -0.5, 0.5),
            D(0.5, -0.5, -0.5),
            A1(-0.5, 0.5, -0.5),
            B1(-0.5, 0.5, 0.5),
            C1(0.5, 0.5, 0.5),
            D1(0.5, 0.5, -0.5);
        glBindTexture(GL_TEXTURE_2D, blockprofile[Category].texture);
        DrawTexture(A, B, C, D);
        DrawTexture(A1, B1, C1, D1);
        DrawTexture(A, B, B1, A1);
        DrawTexture(B, C, C1, B1);
        DrawTexture(C, D, D1, C1);
        DrawTexture(D, A, A1, D1);
        glDisable(GL_TEXTURE_2D);
    }
    void draw(const Player& player) const { // 绘制
        glPushMatrix();
        glTranslated(pos.x, pos.y, pos.z);
        if (!iscoverplayer(player)) {
            if (blockprofile[Category].texture != 0) {
                drawTexture();
            }
            else if (Category == YellowId) {
                if (touched == 0) setMatColor(0.5, 0.5, 0.1, 0.5, 0.5, 0.1);
                else setMatColor(0.35, 0.35, 0.0, 0.35, 0.35, 0.0);
                glutSolidCube(1);
            }
            else if (Category == ThornId) {
                glPushMatrix();
                ((const Thorn*)extra)->trans(pos);
                setMatColor(0.5, 0.15, 0.15, 0.5, 0.15, 0.15);
                glutSolidCone(((const Thorn*)extra)->r, (((const Thorn*)extra)->top - pos).len(), 24, 1);
                glPopMatrix();
            }
            else {
                ofstream log(path + "log.txt");
                log << "in Block::draw func" << endl;
                log << Category << " has no texture" << endl;
                log.close();
                exit(0);
            }
        }
        else {
            if (Category == ThornId) {
                glPushMatrix();
                ((const Thorn*)extra)->trans(pos);
                setMatColor(1.0, 1.0, 1.0, 0.0, 0.0, 0.0);
                glLineWidth(2);
                glutWireCone(((Thorn*)extra)->r, (((const Thorn*)extra)->top - pos).len(), 6, 1);
                glPopMatrix();
            }
            else {
                setMatColor(1, 1, 1, 1, 1, 1);
                glLineWidth(2);
                glutWireCube(1);
            }
        }
        glPopMatrix();
    }
};

vector<Block *> blocks;

void DelBlock(int i) { // 删除第 i 个方块
    delete blocks[i];
    blocks[i] = blocks.back();
    blocks.pop_back();
}

bool showinformation = true;
Player player;

class Arrow { // 箭矢（实则雪球）
public:
    vec3 pos, v;
    Arrow(const vec3& pos, const vec3& v) :pos(pos), v(v) {}
};

vector<Arrow> arrows;

void DrawSky() { // 绘制天空
    setMatColor(0.5, 0.65, 0.8, 0, 0, 0);
    const lf size = 500;
    glPushMatrix();
    const vec3 eyepos = player.pos - player.eyedistance * player.look;
    glTranslated(eyepos.x, eyepos.y, eyepos.z);
    DrawTriangle(vec3(0, size, 0), vec3(size, 0, 0), vec3(0, 0, size));
    DrawTriangle(vec3(0, size, 0), vec3(-size, 0, 0), vec3(0, 0, size));
    DrawTriangle(vec3(0, size, 0), vec3(size, 0, 0), vec3(0, 0, -size));
    DrawTriangle(vec3(0, size, 0), vec3(-size, 0, 0), vec3(0, 0, -size));
    glPopMatrix();
}

void DrawBlocks() { // 绘制方块
    for (const Block* b : blocks) {
        b->draw(player);
    }
}

void DrawLookAtBlock() { // 绘制正在看的方块的白色边框
    if (player.eyedistance != 0) return;
    setMatColor(1, 1, 1, 1, 1, 1);
    glLineWidth(2);
    vec3 to = player.pos + player.look * hand_length;
    const Block* bptr = nullptr;
    for (const Block* b : blocks) if (b->Category != ThornId) {
        if (islooked(b->pos - vec3(0.5, 0.5, 0.5), b->pos + vec3(0.5, 0.5, 0.5), player.pos, to) != -1)
            bptr = b;
    }
    if (bptr != nullptr) {
        glPushMatrix();
        glTranslated(bptr->pos.x, bptr->pos.y, bptr->pos.z);
        glutWireCube(1.003);
        glPopMatrix();

        /*
        glPushMatrix();
        glTranslated(to.x, to.y, to.z);
        glutWireCube(0.1);
        glPopMatrix();
        */
    }
}

void DrawArrows() { // 绘制箭矢（实则雪球）
    setMatColor(0.4, 0.4, 0.4, 0.7, 0.7, 0.7);
    for (const Arrow& i : arrows) {
        glPushMatrix();
        glTranslated(i.pos.x, i.pos.y, i.pos.z);
        glutSolidSphere(0.5, 10, 10);
        glPopMatrix();
    }
}

void DrawButtons() { // 绘制按钮指示器
    vec relative;
    auto V = [&](lf x, lf y) {
        return vec3(relative.x + x, relative.y + y, 0);
    };
    setMatColor(0.8, 0.5, 0.5, 0, 0, 0);
    glLineWidth(1);
    const lf size = 74.5, sx = size / WindowSizeX, sy = size / WindowSizeY;
    static map<char, vec> mp = { {'W', {2.1, 2.1} }, {'S', {2.1, 1} }, { 'A', {1, 1} }, { 'D', {3.2, 1} } };
    for (const pair<char, vec>& i : mp) {
        relative.assign(-1 + sx * i.second.x, -1 + sy * i.second.y);
        if (keys[i.first].state()) {
            DrawTriangle(V(0, 0), V(0, sy), V(sx, sy));
            DrawTriangle(V(0, 0), V(sx, 0), V(sx, sy));
        }
        else {
            vec3 A = V(0, 0), B = V(sx, sy);
            // glRectf(0, 0, sx, sy);
            DrawLines(V(0, 0), V(sx, 0), V(sx, sy), V(0, sy), V(0, 0));
        }
    }
    relative.assign(1 - sx * 1, -1 + sy * 1);
    lf Sx = sx * 2.5;
    if (keys[' '].state()) {
        DrawTriangle(V(0, 0), V(0, sy), V(-Sx, sy));
        DrawTriangle(V(0, 0), V(-Sx, 0), V(-Sx, sy));
    }
    else {
        DrawLines(V(0, 0), V(-Sx, 0), V(-Sx, sy), V(0, sy), V(0, 0));
    }
}

void DrawCursor() { // 绘制中心指示器
    if (player.eyedistance != 0) return;
    const lf cursor_size = 15, sx = cursor_size / WindowSizeX, sy = cursor_size / WindowSizeY;
    setMatColor(1, 1, 1, 0, 0, 0);
    glLineWidth(2);
    DrawLine(vec3(-sx, 0, 0), vec3(sx, 0, 0));
    DrawLine(vec3(0, -sy, 0), vec3(0, sy, 0));
}

class InfomationPrinter { // 用来打印信息，自动换行
public:
    vec pos;
    InfomationPrinter(lf x, lf y) :pos(x, y) {
        setMatColor(1, 1, 1, 0, 0, 0);
        glRasterPos2d(pos.x, pos.y);
    }
    void print(char c) {
        if (c != '\n') {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
        else {
            pos.y -= 40.0 / WindowSizeY;
            glRasterPos2d(pos.x, pos.y);
        }
    }
    void operator()(const char *s) {
        for (; *s != '\0'; s++) print(*s);
    }
    InfomationPrinter& operator<<(const string& str) {
        for (char i : str) print(i);
        return *this;
    }
    InfomationPrinter& operator<<(const char* s) {
        for (; *s != '\0'; s++) print(*s);
        return *this;
    }
};

void DrawInformation() { // 绘制信息
    if (!showinformation) return;
    InfomationPrinter output(-1 + 20.0 / WindowSizeX, 1 - 70.0 / WindowSizeX);
    static char str[100];
    output << "Press Esc to quit\n";
    sprintf_s(str, sizeof(str), "FPS: %.1f, %s mode\n",
        Timer::getFPS(),
        player.gamemode == GameMode::Creative ? "Creative" : "Adventure"
    );
    output(str);
    sprintf_s(str, sizeof(str), "XYZ: (%.1f, %.1f, %.1f)\n",
        player.pos.x, player.pos.y - block_up, player.pos.z
    );
    output(str);
    output << "Block in hand: " << blockprofile[player.blockInHand].name << "\n";
    if (player.killed) {
        output << "You dead\n";
    }
}

void DrawEvent() { // 绘制模型
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, WindowSizeX, WindowSizeY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0, (float)WindowSizeX / WindowSizeY, 0.01, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    const vec3 eyepos = player.pos - player.eyedistance * player.look;
    setLookAt(eyepos, eyepos + player.look, vec3(0, 1, 0));
    glPushMatrix();
    // glTranslated(-eyepos.x, -eyepos.y, -eyepos.z);

    DrawSky();
    DrawBlocks();
    DrawLookAtBlock();
    player.draw();
    DrawArrows();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    DrawButtons();
    DrawCursor();
    DrawInformation();
    glutSwapBuffers();
}

void MousePress() { // 处理鼠标事件
    if (keys[1].state() && (Timer::counter - keys[1].clickbegin) % clickcycle_t == 0) {
        if (player.gamemode == GameMode::Creative) {
            vec3 to = player.pos + player.look * hand_length;
            const Block* brec = nullptr;
            for (const Block* b : blocks) if (b->Category != ThornId) {
                if (islooked(b->pos - vec3(0.5, 0.5, 0.5), b->pos + vec3(0.5, 0.5, 0.5), player.pos, to) != -1)
                    brec = b;
            }
            for (uint i = 0; i < blocks.size(); i++) {
                if (blocks[i] == brec) {
                    DelBlock(i);
                    break;
                }
            }
        }
    }
    if (keys[2].state() && (Timer::counter - keys[2].clickbegin) % clickcycle_t == 0) {
        if (player.gamemode == GameMode::Creative) {
            vec3 to = player.pos + player.look * hand_length;
            const Block* brec = nullptr;
            int dir;
            for (const Block* b : blocks) if (b->Category != ThornId) {
                int t = islooked(b->pos - vec3(0.5, 0.5, 0.5), b->pos + vec3(0.5, 0.5, 0.5), player.pos, to);
                if (t != -1) {
                    dir = t;
                    brec = b;
                }
            }
            if (brec != nullptr) {
                vec3 pos = brec->pos + dir_to_vec3(dir);
                if (!player.pos.between(pos - vec3(block_horizon, block_down, block_horizon), pos + vec3(block_horizon, block_up, block_horizon))) {
                    blocks.push_back(new Block(pos, player.blockInHand));
                }
            }
        }
    }
    if (keys[4].state() && (Timer::counter - keys[4].clickbegin) % clickcycle_t == 0) {
        arrows.emplace_back(player.pos + player.look * 1.5, player.look * jump_v * 5);
    }
}

void KeyPress() { // 处理键盘事件
    // for (uint i = 0; i < 256; i++) if (keys[i].press()) cout << i << endl;
    if (keys[27].press()) // Esc 键
        exit(0);
    if (keys['R'].press()) {
        if (!keys.ctrl()) player.respawn();
        else player.restart();
    }
    if (keys['S'].press() && keys.ctrl()) { // Ctrl + S
        FILE* source = fopen((path + "save.txt").c_str(), "w");
        fprintf(source,
            "Start %.6f %.6f %.6f\n",
            player.startpoint.x, player.startpoint.y, player.startpoint.z
        );
        for (const Block* b : blocks) {
            if (b->Category == ThornId) {
                const Thorn& thorn = *(Thorn*)b->extra;
                fprintf(source,
                    "%s %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                    b->name().c_str(),
                    b->pos.x, b->pos.y, b->pos.z,
                    thorn.top.x, thorn.top.y, thorn.top.z,
                    thorn.r
                );
            }
            else {
                fprintf(source,
                    "%s %.6f %.6f %.6f\n",
                    b->name().c_str(),
                    b->pos.x, b->pos.y, b->pos.z
                );
            }
        }
    }
    if (keys[187].press() || keys[107].press()) { // + 键
        if (player.eyedistance == 0);
        else if (player.eyedistance == 3) player.eyedistance = 0;
        else player.eyedistance /= 2;
    }
    if (keys[189].press() || keys[109].press()) { // - 键
        if (player.eyedistance == 0) player.eyedistance = 3;
        else if (player.eyedistance < 128) player.eyedistance *= 2;
    }
    if (keys[9].press()) { // tab 键
        do {
            player.blockInHand = (player.blockInHand + 1) % (sizeof(blockprofile) / sizeof(*blockprofile));
        } while (player.blockInHand == ThornId);
    }
    if (keys[' '].doubleclick && player.gamemode == GameMode::Creative) { // 双击空格
        player.fly ^= true;
    }
}

void NextFrame() { // 周期的物理变化，被 timer 控制
    keys.update();
    KeyPress();
    MousePress();

    for (uint i = 0; i < arrows.size(); i++) {
        arrows[i].v.y -= gravity_a;
        arrows[i].pos = arrows[i].pos + arrows[i].v;
        if (arrows[i].pos.y < vacant_height) arrows[i] = arrows.back(), arrows.pop_back(), i--;
    }

    vec t;
    lf move_v2 = 1;
    if (keys['W'].state() - keys['S'].state() != 0 && keys['A'].state() - keys['d'].state() != 0) move_v2 = 1 / sqrt(2);
    t = t + ((lf)keys['W'].state() - keys['S'].state()) * move_v * move_v2 * vec(player.look.x, player.look.z).trunc(1);
    t = t + ((lf)keys['A'].state() - keys['D'].state()) * move_v * move_v2 * vec(player.look.z, -player.look.x).trunc(1);
    player.v.x = t.x;
    player.v.z = t.y;
    if (!player.fly) {
        player.v.y -= gravity_a;
        if (keys[' '].state() && player.touchfloor) {
            if (player.v.y < jump_v) player.v.y = jump_v;
        }
    }
    else {
        player.v.y = 0;
        if (keys[' '].state()) {
            player.v.y += move_v;
        }
        if (shiftstate()) {
            player.v.y -= move_v;
        }
    }
    if (player.v.y < -max_v)player.v.y = -max_v;
    player.touchfloor = false;

    vec3 to = player.pos + player.v;

    sort(blocks.begin(), blocks.end(), [](const Block* a, const Block* b) {
        if (abs(a->pos.y - b->pos.y) > 0.00001) return abs(player.pos.y - a->pos.y) < abs(player.pos.y - b->pos.y);
        else return abs(a->pos.x - player.pos.x) + abs(a->pos.z - player.pos.z) < abs(b->pos.x - player.pos.x) + abs(b->pos.z - player.pos.z);
    });
    for (Block* b : blocks) {
        if (b->Category == ThornId) {
            vec3 X(1, 2.718281828, pi);
            const Thorn& thorn = *((Thorn*)b->extra);
            vec3 dir = thorn.top - b->pos;
            X = (X - dir.trunc(dot(dir, X) / dir.len())).trunc(thorn.r);
            vec3 Y = cross(dir, X).trunc(thorn.r);
            for (lf angle = 0; angle < pi * 2; angle += 1) {
                if (islookedcubeconst(player.pos - vec3(block_horizon - 0.5, block_up - 0.5, block_horizon - 0.5), player.pos + vec3(block_horizon - 0.5, block_up - 0.5, block_horizon - 0.5), thorn.top, b->pos + X * cos(angle) + Y * sin(angle))) {
                    player.pos = to;
                    player.killed = true;
                    break;
                }
            }
        }
        else {
            int choose = islookedconst(b->pos - vec3(block_horizon, block_down, block_horizon), b->pos + vec3(block_horizon, block_up, block_horizon), player.pos * 2 - to, to);
            b->touched = choose;
            if (choose != -1) {
                if (choose == 0) {
                    player.touchfloor = true;
                    to.y = b->pos.y + block_up + 0.00001;
                    if (b->Category == GreenId && !keys.shift()) {
                        player.v.y = -player.v.y * 0.9;
                    }
                    else if (b->Category == PurpleId && !keys.shift()) {
                        player.v.y = purple_v;
                    }
                    else {
                        player.v.y = 0;
                    }
                    if (b->Category == YellowId) {
                        player.savepoint = b->pos + vec3(0, block_up, 0);
                    }
                    if (player.fly) player.fly = false;
                }
                else if (choose == 1) {
                    to.y = b->pos.y - block_down - 0.00001;
                    player.v.y = 0;
                }
                else if (choose == 2) {
                    to.x = b->pos.x + block_horizon + 0.000001;
                    player.v.x = 0;
                }
                else if (choose == 3) {
                    to.x = b->pos.x - block_horizon - 0.000001;
                    player.v.x = 0;
                }
                else if (choose == 4) {
                    to.z = b->pos.z + block_horizon + 0.000001;
                    player.v.z = 0;
                }
                else if (choose == 5) {
                    to.z = b->pos.z - block_horizon - 0.000001;
                    player.v.z = 0;
                }
                if (b->Category == OrangeId && choose >= 2 && choose <= 5) {
                    if (player.v.y > orange_v) player.v.y = orange_v;
                    if (player.v.y < -orange_v) player.v.y = -orange_v;
                }
            }
        }
        if (player.killed) break;
    }
    if (!player.killed) { // 死亡后不移动
        player.pos = to;
    }
    if (player.pos.y < vacant_height) player.killed = true;
    DrawEvent();
}

void SpecialKeyPressEvent(int key, int x, int y) { // 按下特殊按键
    // cout << key << endl;
    if (key == 5) { // F5
        if (player.eyedistance == 0) {
            player.eyedistance = 3;
        }
        else {
            player.eyedistance = 0;
        }
    }
    else if (key == 11) { // F11
        if (fullscreen) {
            glutReshapeWindow(1000, 600);
        }
        else {
            glutFullScreen();
        }
        fullscreen ^= 1;
    }
    else if (key == 9) { // F9
        if (MusicPlayer::playing) MusicPlayer::stop();
        else MusicPlayer::play();
    }
    else if (key == 1) { // F1
        if (player.gamemode == GameMode::Creative) {
            player.gamemode = GameMode::Adventure;
            player.fly = false;
        }
        else {
            player.gamemode = GameMode::Creative;
        }
    }
    else if (key == 3) { // F3
        showinformation ^= true;
    }
}

void MouseMoveEvent(int x, int y) { // 鼠标移动触发
    if (x == WindowSizeX / 2 && y == WindowSizeY / 2) return;
    player.lng += (x - lf(WindowSizeX / 2)) * head_move_v;
    player.lat -= (y - lf(WindowSizeY / 2)) * head_move_v;
    if (player.lat > 89.9) player.lat = 89.9;
    if (player.lat < -89.9) player.lat = -89.9;
    player.look.assign_from_lnglat(player.lng, player.lat);
    MouseToMiddle();
}

void ReshapeEvent(int x, int y) { // 窗口大小改变触发
    WindowSizeX = x;
    WindowSizeY = y;
    MouseToMiddle();
}

int name_to_blockid(const string& name) { // 通过名字求方块 id
    for (const BlockProfile& b : blockprofile) {
        if (name == b.name)
            return &b - blockprofile;
    }
    ofstream log(path + "log.txt");
    log << "in name_to_blockid func" << endl;
    log << "no block named " << name << endl;
    log.close();
    exit(0);
}

void initblocks() { // 从文件中输入 blocks 信息
    while (!blocks.empty()) DelBlock(0);
    ifstream source(path + "init.txt");
    player.startpoint = vec3(0, block_up, 0);
    string s;
    while (source >> s) {
        if (s == "Start") {
            vec3 pos;
            source >> pos.x >> pos.y >> pos.z;
            player.startpoint = pos;
        }
        else if (s == "Thorn") {
            vec3 pos, top; lf r;
            source >> pos.x >> pos.y >> pos.z;
            source >> top.x >> top.y >> top.z;
            source >> r;
            blocks.push_back(new Block(pos, name_to_blockid(s), new Thorn(top, r)));
        }
        else {
            vec3 pos;
            source >> pos.x >> pos.y >> pos.z;
            blocks.push_back(new Block(pos, name_to_blockid(s)));
        }
    }
    // sort(blocks.begin(), blocks.end(), [&](const Block* a, const Block* b) { return a->pos.y > b->pos.y; });
    source.close();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Parkour");
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glutSpecialFunc(SpecialKeyPressEvent); // 特殊按键按下
    glutDisplayFunc(DrawEvent); // 绘制函数
    glutMotionFunc(MouseMoveEvent); // 鼠标按下时的移动
    glutPassiveMotionFunc(MouseMoveEvent); // 鼠标未按下时的移动
    glutReshapeFunc(ReshapeEvent); // 窗口大小改变
    myglinit();

    initblocks();
    player.restart();
    player.lng = player.lat = 0;
    player.look.assign_from_lnglat(player.lng, player.lat);

    for (BlockProfile& i : blockprofile) {
        i.texture = load_texture((path + i.name + ".bmp").c_str());
    }

    // glutFullScreen();
    WindowSizeX = glutGet(GLUT_WINDOW_WIDTH);
    WindowSizeY = glutGet(GLUT_WINDOW_HEIGHT);
    
    glViewport(0, 0, WindowSizeX, WindowSizeY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0, (float)WindowSizeX / WindowSizeY, 1.0 * 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    MouseToMiddle();
    Timer::init(NextFrame);
    MusicPlayer::init();
    // MusicPlayer::play();
    // glEnable(GL_TEXTURE_2D);
    // glTexImage2D;
    glutMainLoop();
}
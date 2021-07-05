#ifndef Parkour_imports_UI
#define Parkour_imports_UI

#include "imports/basic.h"
#include "imports/const.h"

int WindowSizeX, WindowSizeY; // 窗口长宽
bool fullscreen = false; // 是否全屏

namespace Timer { // 计时器，每隔 cycle_t 执行一次函数 f
    int cycle_t;
    ll counter;
    ll preclock;
    int real_cycle_t;
    deque<ll> v(1, -100);
    void (*f)();
    lf getFPS() { // 计算帧率
        return (v.size() - 1) * 50000.0 / (v.back() - v.front());
    }
    void func(int id) {
        f();
        ll nowclock = clock();
        if (counter % 50 == 0) {
            if (v.size() == 6) v.pop_front();
            v.push_back(nowclock);
        }
        counter++;
        if (nowclock - preclock < cycle_t) real_cycle_t++;
        else if (nowclock - preclock >= cycle_t) real_cycle_t = max(real_cycle_t - 1, 0);
        glutTimerFunc(real_cycle_t, func, id);
        preclock = nowclock;
    }
    void init(void (*_f)()) {
        cycle_t = timer_t + timer_wait; f = _f; counter = 0;
        real_cycle_t = cycle_t;
        glutTimerFunc(cycle_t, func, 0);
    }
}

void glVertices() {} // 参数包递归退出
template <typename... Args>
void glVertices(const vec3& v, Args... x) { // 放置任意个点（参数包）
    glVertex3d(v.x, v.y, v.z);
    glVertices(x...);
}
void DrawTriangle(const vec3& A, const vec3& B, const vec3& C) { // 绘制三角形
    glBegin(GL_TRIANGLES);
    glVertices(A, B, C);
    glEnd();
}
void DrawLine(const vec3& A, const vec3& B) { // 绘制一条线
    glBegin(GL_LINES);
    glVertices(A, B);
    glEnd();
}
void DrawLines(const vec3& A) {} // 参数包递归退出
template <typename... Args>
void DrawLines(const vec3 &A, const vec3& B, Args... x) { // 绘制一个折线
    DrawLine(A, B);
    DrawLines(B, x...);
}

void DrawTexture(const vec3& A, const vec3& B, const vec3& C, vec3 D) {
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertices(A);
    glTexCoord2d(1.0, 0.0);
    glVertices(B);
    glTexCoord2d(1.0, 1.0);
    glVertices(C);
    glTexCoord2d(0.0, 1.0);
    glVertices(D);
    glEnd();
}

void setMatColor(lf R1, lf G1, lf B1, lf R2, lf G2, lf B2) { // 改变物体颜色，前三环境色，后三漫反射
    GLfloat color1[] = { (float)R1, (float)G1, (float)B1, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, color1);
    GLfloat color2[] = { (float)R2, (float)G2, (float)B2, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, color2);
}

void setLookAt(const vec3& from, const vec3& to, const vec3& up) {
    glLoadIdentity();
    gluLookAt(
        (float)from.x, (float)from.y, (float)from.z,
        (float)to.x, (float)to.y, (float)to.z,
        (float)up.x, (float)up.y, (float)up.z
    );
}

class KeyBoard { // 键盘输入
public:
    bool getkeystate(int c) { return GetKeyState(c) < 0; }
    class Key {
    public:
        bool activate, now, past, doubleclick;
        ll doubleclickbegin, clickbegin;
        Key() :activate(), now(), past(), doubleclickbegin(-doubleclick_t), doubleclick(), clickbegin() {}
        bool state() const { return now; }
        bool press() const { return now && !past; }
        bool release() const { return !now && past; }
    } a[256];
    KeyBoard() {
        for (int i = 0; i < 256; i++)
            a[i] = Key();
        a[16].activate = a[17].activate = a[18].activate = true;
    }
    const Key& operator[](int id) {
        a[id].activate = true;
        return a[id];
    }
    void update() {
        for (int i = 0; i < 256; i++) if (a[i].activate) {
            a[i].past = a[i].now;
            a[i].now = getkeystate(i);
            a[i].doubleclick = false;
            if (a[i].press()) {
                if (Timer::counter - a[i].doubleclickbegin < doubleclick_t) {
                    a[i].doubleclick = true;
                    a[i].doubleclickbegin = -doubleclick_t;
                }
                else {
                    a[i].doubleclickbegin = Timer::counter;
                }
                a[i].clickbegin = Timer::counter;
            }
        }
        // a[1].now = mouse.a.mkLButton;
        // a[2].now = mouse.a.mkRButton;
        // a[4].now = mouse.a.mkMButton;
    }
    bool none() const {
        if (a[16].state())return false; // shift
        if (a[17].state())return false; // ctrl
        if (a[18].state())return false; // alt
        return true;
    }
    bool shift() const {
        if (!a[16].state())return false; // shift
        if ( a[17].state())return false; // ctrl
        if ( a[18].state())return false; // alt
        return true;
    }
    bool ctrl() const {
        if ( a[16].state())return false; // shift
        if (!a[17].state())return false; // ctrl
        if ( a[18].state())return false; // alt
        return true;
    }
}keys;

bool shiftstate() {
    return GetKeyState(16) & (1 << (sizeof(SHORT) * 8 - 1));
}
bool ctrlstate() {
    return GetKeyState(17) & (1 << (sizeof(SHORT) * 8 - 1));
}

void MouseToMiddle() { // 光标移动至中心
    glutWarpPointer(WindowSizeX / 2, WindowSizeY / 2);
}

void myglinit() { // 处理一些 gl 的初始化
    {
        vec3 lightPosition(60.0, 80.0f, 100.0f);
        GLfloat light_position[] = { (GLfloat)lightPosition.x, (GLfloat)lightPosition.y, (GLfloat)lightPosition.z, 1.0f }; // 位置
        GLfloat light_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 环境光
        GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 漫反射光
        GLfloat light_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 镜面光
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 镜面光
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 30.0f;
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
    }

    glutSetCursor(GLUT_CURSOR_NONE);
    glutSetKeyRepeat(0);
}

namespace MusicPlayer { // 循环播放音乐
    AudioClip audio;
    int nowid = 1;
    bool playing;
    void func(int id) {
        if (id == nowid) {
            audio.play();
            glutTimerFunc(audio.milliseconds(), func, nowid);
        }
    }
    void stop() {
        playing = false;
        nowid++;
        audio.stop();
    }
    void play() {
        playing = true;
        nowid++;
        glutTimerFunc(0, func, nowid);
    }
    void init() {
        audio.load(path + "TwinShot.mp3");
    }
}

#endif // Parkour_imports_UI
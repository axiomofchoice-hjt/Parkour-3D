#ifndef Parkour_imports_basic
#define Parkour_imports_basic

namespace Definitions { // ����
    typedef uint32_t u32;
    typedef int32_t i32;
    typedef uint64_t u64;
    typedef int64_t i64;

    typedef uint32_t uint;
    typedef uint64_t ull;
    typedef int64_t ll;

    typedef double lf;
}
using namespace Definitions;

namespace ConstantValues { // ����
    const lf pi = acos(-1);
}
using namespace ConstantValues;

class vec { // ��ά����
public:
    lf x, y;
    vec() :x(), y() {}
    vec(lf x, lf y) :x(x), y(y) {}
    void assign(lf _x, lf _y) {
        x = _x; y = _y;
    }
    vec& operator+=(const vec& b) {
        x += b.x; y += b.y;
        return *this;
    }
    vec& operator-=(const vec& b) {
        x -= b.x; y -= b.y;
        return *this;
    }
    vec& operator*=(const lf& k) {
        x *= k; y *= k;
        return *this;
    }
    vec& operator/=(const lf& k) {
        x /= k; y /= k;
        return *this;
    }
    friend vec operator+(const vec& a, const vec& b) {
        return vec(a.x + b.x, a.y + b.y);
    }
    friend vec operator-(const vec& a, const vec& b) {
        return vec(a.x - b.x, a.y - b.y);
    }
    friend vec operator-(const vec& v) {
        return vec(-v.x, -v.y);
    }
    friend vec operator*(lf k, const vec& v) {
        return vec(k * v.x, k * v.y);
    }
    friend vec operator*(const vec& v, lf k) {
        return vec(k * v.x, k * v.y);
    }
    friend vec operator/(const vec& v, lf k) {
        return vec(v.x / k, v.y / k);
    }
    lf sqr() const { return x * x + y * y; } // ƽ��
    lf len() const { return hypot(x, y); } // ����
    vec trunc(lf k) const { // ������ͬ������Ϊ k ������
        return *this * (k / hypot(x, y));
    }
    bool between(const vec& l, const vec& r) const { // ��άƫ��� between
        return less_than(l, *this) && less_than(*this, r);
    }
    friend bool less_than(const vec& a, const vec& b) { // ��άƫ��
        return a.x < b.x&& a.y < b.y;
    }
    friend ostream& operator<<(ostream& o, const vec& v) { // ���
        return o << "(" << v.x << ", " << v.y << ")";
    }
};

struct vec3 { // ��ά����
    lf x, y, z;
    vec3() :x(0), y(0), z(0) {}
    vec3(lf x, lf y, lf z) :x(x), y(y), z(z) {}
    void assign(lf _x, lf _y, lf _z) {
        x = _x; y = _y; z = _z;
    }
    void assign_from_lnglat(lf lng, lf lat) { // lng ���ȣ�lat γ�ȣ�-90 < lat < 90
        lng *= pi / 180, lat *= pi / 180;
        y = sin(lat);
        lf m = cos(lat);
        x = cos(lng) * m, z = sin(lng) * m;
    }
    vec3 &operator+=(const vec3& b) {
        x += b.x; y += b.y; z += b.z;
        return *this;
    }
    vec3& operator-=(const vec3& b) {
        x -= b.x; y -= b.y; z -= b.z;
        return *this;
    }
    vec3& operator*=(const lf& k) {
        x *= k; y *= k; z *= k;
        return *this;
    }
    vec3& operator/=(const lf& k) {
        x /= k; y /= k; z /= k;
        return *this;
    }
    friend vec3 operator+(const vec3& a, const vec3& b) {
        return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
    }
    friend vec3 operator-(const vec3& a, const vec3& b) {
        return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
    }
    friend vec3 operator-(const vec3& v) {
        return vec3(-v.x, -v.y, -v.z);
    }
    friend vec3 operator*(const vec3& v, lf k) {
        return vec3(k * v.x, k * v.y, k * v.z);
    }
    friend vec3 operator*(lf k, const vec3& v) {
        return vec3(k * v.x, k * v.y, k * v.z);
    }
    friend vec3 operator/(const vec3& v, lf k) {
        return vec3(v.x / k, v.y / k, v.z / k);
    }
    lf sqr() const { return x * x + y * y + z * z; } // ƽ��
    lf len() const { return sqrt(x * x + y * y + z * z); } // ����
    lf Mlen() const { return abs(x) + abs(y); } // �����پ���
    lf Clen() const { return max(abs(x), abs(y)); } // �б�ѩ�����
    vec3 trunc(lf k = 1) const { return *this * (k / len()); } // ������ͬ������Ϊ k ������
    vec3 trunc(const vec3& k) const { return *this * (dot(*this, k) / sqr()); } // ������ͬ������Ϊ k �ڸ������ϵ�ͶӰ
    bool between(const vec3& l, const vec3& r) const { // ��άƫ��� between
        return less_than(l, *this) && less_than(*this, r);
    }
    friend vec3 cross(const vec3& a, const vec3& b) { // ���
        return vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
    friend lf dot(const vec3& a, const vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; } // ���
    friend vec3 rotate(const vec3& p, vec3 l, lf th) { // ���ᣨһ��Ϊԭ�㣩��ת
        struct four {
            lf r; vec3 v;
            four operator*(const four& b) const {
                return { r * b.r - dot(v,b.v),v * b.r + b.v * r + cross(v,b.v) };
            }
        };
        l = l.trunc();
        four P = { 0, p };
        four Q1 = { cos(th / 2), l * sin(th / 2) };
        four Q2 = { cos(th / 2), vec3() - l * sin(th / 2) };
        return ((Q1 * P) * Q2).v;
    }
    friend vec3 rotate(const vec3& p, const vec3& l0, const vec3& l1, lf th) { // ������ת
        return rotate(p - l0, l1 - l0, th) + l0;
    }
    friend bool less_than(const vec3& a, const vec3& b) { // ��άƫ��
        return a.x < b.x&& a.y < b.y&& a.z < b.z;
    }
    friend ostream& operator<<(ostream& o, const vec3& v) { // ���
        return o << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    }
};

int islooked(const vec3& b1, const vec3& b2, const vec3& from, vec3& to) { // b1, b2 ȷ�����������Ƿ��� from -> to �߶Σ�from ���������棩���������޸� to Ϊ��ӽ� from �ĵ�
#define f1(y, x, z) from.y > b2.y && to.y < b2.y && ((b2.y - to.y) * vec(from.x, from.z) + (from.y - b2.y) * vec(to.x, to.z)).between(vec(b1.x, b1.z) * (from.y - to.y), vec(b2.x, b2.z) * (from.y - to.y))
#define f2(y, x, z) from.y < b1.y && to.y > b1.y && ((to.y - b1.y) * vec(from.x, from.z) + (b1.y - from.y) * vec(to.x, to.z)).between(vec(b1.x, b1.z) * (to.y - from.y), vec(b2.x, b2.z) * (to.y - from.y))
#define op1(y) to = ((b2.y - to.y) * from + (from.y - b2.y) * to) / (from.y - to.y);
#define op2(y) to = ((to.y - b1.y) * from + (b1.y - from.y) * to) / (to.y - from.y);
    if (f1(y, x, z)) {
        op1(y);
        return 0;
    }
    else if (f2(y, x, z)) {
        op2(y);
        return 1;
    }
    else if (f1(x, y, z)) {
        op1(x);
        return 2;
    }
    else if (f2(x, y, z)) {
        op2(x);
        return 3;
    }
    else if (f1(z, x, y)) {
        op1(z);
        return 4;
    }
    else if (f2(z, x, y)) {
        op2(z)
        return 5;
    }
    return -1;
#undef f1
#undef f2
#undef op1
#undef op2
}

int islookedconst(const vec3& b1, const vec3& b2, const vec3& from, const vec3& to) { // b1, b2 ȷ�����������Ƿ��� from -> to �߶Σ�from ���������棩
#define f1(y, x, z) from.y > b2.y && to.y < b2.y && ((b2.y - to.y) * vec(from.x, from.z) + (from.y - b2.y) * vec(to.x, to.z)).between(vec(b1.x, b1.z) * (from.y - to.y), vec(b2.x, b2.z) * (from.y - to.y))
#define f2(y, x, z) from.y < b1.y && to.y > b1.y && ((to.y - b1.y) * vec(from.x, from.z) + (b1.y - from.y) * vec(to.x, to.z)).between(vec(b1.x, b1.z) * (to.y - from.y), vec(b2.x, b2.z) * (to.y - from.y))
    if (f1(y, x, z)) {
        return 0;
    }
    else if (f2(y, x, z)) {
        return 1;
    }
    else if (f1(x, y, z)) {
        return 2;
    }
    else if (f2(x, y, z)) {
        return 3;
    }
    else if (f1(z, x, y)) {
        return 4;
    }
    else if (f2(z, x, y)) {
        return 5;
    }
    return -1;
#undef f1
#undef f2
}

bool islookedcubeconst(const vec3& b1, const vec3& b2, const vec3& from, const vec3& to) { // ���������߶��Ƿ��н���
    if (from.between(b1, b2) || to.between(b1, b2)) return true;
    return islookedconst(b1, b2, from, to) != -1;
}

bool islookedplaneconst(const vec3& A, const vec3& B, const vec3& C, const vec3& from, const vec3& to) { // ƽ�����߶��Ƿ��н���
    return (dot(cross(B - A, C - A), from - A) > 0) ^ (dot(cross(B - A, C - A), to - A) > 0);
}
bool islookedtriangleconst(const vec3& A, const vec3& B, const vec3& C, const vec3& from, const vec3& to) { // ���������߶��Ƿ��н���
    return islookedplaneconst(A, B, C, from, to)
        && islookedplaneconst(from, to, A, B, C)
        && islookedplaneconst(from, to, B, C, A)
        && islookedplaneconst(from, to, C, A, B);
}

vec3 dir_to_vec3(int dir) { // ������ת��Ϊ�ռ�����
    static vec3 arr[] = {
        vec3(0, 1, 0), vec3(0, -1, 0),
        vec3(1, 0, 0), vec3(-1, 0, 0),
        vec3(0, 0, 1), vec3(0, 0, -1)
    };
    return arr[dir];
}

string to_string_p1(lf value) { // ���� 1 λС��
    static char s[20];
    sprintf(s, "%.1f", value);
    return s;
}

#endif // Parkour_imports_basic
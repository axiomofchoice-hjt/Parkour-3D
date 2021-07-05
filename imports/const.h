#ifndef Parkour_imports_const
#define Parkour_imports_const

const lf block_up = 2.19; // 方块碰撞箱的上表面高度
const lf block_down = 0.79; // 方块碰撞箱的下表面高度
const lf block_horizon = 0.7; // 方块碰撞箱的水平表面距离
const int timer_t = 10, timer_wait = 3; // 游戏时间和 timer 多等待的时间
const lf gravity_a = 0.00005 * timer_t * timer_t; // 重力加速度
const lf jump_v = sqrt(1.5 * 2 * gravity_a); // 跳跃速度
const lf move_v = 0.006 * timer_t; // 移动速度
const lf max_v = 5.0 * timer_t; // 最大下降速度
const lf orange_v = 0.0002 * timer_t; // 接触橙色方块的最大上升 / 下降速度
const lf purple_v = jump_v * 3; // 紫色方块弹跳速度
const lf head_move_v = 0.1; // 鼠标控制转头速度
const lf vacant_height = -50; // 虚空高度
const lf hand_length = 8; // 破坏、放置方块的距离限制
const int doubleclick_t = 40; // 双击最大时间间隔（帧）
const int clickcycle_t = 20; // 按住后触发单机事件周期（帧）

string path; // 数据（init.txt, res/ 等) 的位置
class Initpath {
public:
	Initpath() {
		ifstream source("res/init.txt");
		if (!source.good()) path = "../";
		source.close();
		path += "res/";
	}
}initpath;

#endif // Parkour_imports_const
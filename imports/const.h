#ifndef Parkour_imports_const
#define Parkour_imports_const

const lf block_up = 2.19; // ������ײ����ϱ���߶�
const lf block_down = 0.79; // ������ײ����±���߶�
const lf block_horizon = 0.7; // ������ײ���ˮƽ�������
const int timer_t = 10, timer_wait = 3; // ��Ϸʱ��� timer ��ȴ���ʱ��
const lf gravity_a = 0.00005 * timer_t * timer_t; // �������ٶ�
const lf jump_v = sqrt(1.5 * 2 * gravity_a); // ��Ծ�ٶ�
const lf move_v = 0.006 * timer_t; // �ƶ��ٶ�
const lf max_v = 5.0 * timer_t; // ����½��ٶ�
const lf orange_v = 0.0002 * timer_t; // �Ӵ���ɫ������������ / �½��ٶ�
const lf purple_v = jump_v * 3; // ��ɫ���鵯���ٶ�
const lf head_move_v = 0.1; // ������תͷ�ٶ�
const lf vacant_height = -50; // ��ո߶�
const lf hand_length = 8; // �ƻ������÷���ľ�������
const int doubleclick_t = 40; // ˫�����ʱ������֡��
const int clickcycle_t = 20; // ��ס�󴥷������¼����ڣ�֡��

string path; // ���ݣ�init.txt, res/ ��) ��λ��
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
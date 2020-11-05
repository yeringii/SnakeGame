#include <iostream>
#include <random> //random
#include "stage.cpp"
using namespace std;

const int ARR_X = 40;
const int ARR_Y = 21;

struct SnakeDir{
  int x;
  int y;
}; //x, y좌표

struct nextDir{
  int x;
  int y;
}; //다음좌표를 담는 구조체

class Snake{
  private:
    SnakeDir s[100];
    nextDir dir[4];
    int tail; //snake 길이
    int next_X, next_Y, d_op; //다음 좌표에 더할 값, tail을 추가할때에 필요한 방향
    int fail_dir, game_result;//가는 방향과 반대 방향일 시 game over
    int map[ARR_Y][ARR_X];
    int poison_Cnt;
    int growth_Cnt;
    int gate_Cnt; //74-76
	int g1_x, g1_y, g2_x, g2_y;
	int stage_chk = 1;
  public:
    Snake(int s = 1);
    void Map(); //snake를 표시하고 map각각을 정의함
    void Score();
    void Game(); //자판key를 받아 다음 방향을 정해줌
    int GameOver(int ch = -1);
    int itemTmp();
    void assign_I();
    int getXPoint();
    int getYPoint();
    void growth();
    void poison();
    void setZero(int, int);
		void is_Edge(int, int);
		void Exit_gate(int, int, int);
		void gate_xy();
		void check_gate();
};

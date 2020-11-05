#include <iostream>
#include <ncurses.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>//3~5 : kbhit
#include <random> //random
#include <ctime> //time
#include <queue>
#include <algorithm> //shuffle
#include <climits>
#include "map_fin.h"

static int peek_character = -1;
static struct termios initial_settings, new_settings;

int _kbhit()
{
	unsigned char ch;
	int nread;

	if (peek_character != -1) return 1;
	new_settings.c_cc[VMIN]=0;
	tcsetattr(0, TCSANOW, &new_settings);
	nread = read(0,&ch,1);
	new_settings.c_cc[VMIN]=1;
	tcsetattr(0, TCSANOW, &new_settings);
	if(nread == 1)
	{
		peek_character = ch;
		return 1;
	}
	return 0;
}

int _getch()
{
	char ch;

	if(peek_character != -1)
	{
		ch = peek_character;
		peek_character = -1;
		return ch;
	}
	read(0,&ch,1);
	return ch;
} //kbhit & getch 오픈소스 인용

int gateAssign = 0;
queue<pair<int, int>> q_xy;
deque<time_t> dq_t;
vector<pair<int, int>> wall; // 벽부분 좌표값을 저장해 줄 벡터 생성

int main() {
  random_device rd;
  mt19937 gen(rd());
  Snake snake; //snake 객체
  double rt = INT_MAX, rga = INT_MAX; //2-6초 사이에 랜덤으로 초를 받기위한 변수, 큰 수로 초기화 하지 않으면 111번째 줄에 시작하자마자 걸리기 때문에 큰 값으로 초기화
  int err; //에러 번호
  int x, y, ga_x1, ga_y1, ga_x2, ga_y2;; //아이템 개수와 좌표
  int t_ck = 0, tga_ck = 0;
  time_t start_t, start_tga; //아이템이 등장하기 위해 찍어놓는 시간점
  time_t stop_t, stop_tga; //아이템이 사라지게 하기 위해 찍어놓는 종료점
  time_t t; //dq_t에서 가져오기위해 필요한 변수

  while(1){
    try{
      snake.Map();
      snake.Score();
      snake.Game();
      usleep(50000);
      if (t_ck == 0 || tga_ck == 0){ //만약 시간이 측정되지 않은 상태인 경우
        if (t_ck == 0){ //item
          start_t = time(NULL);
          uniform_int_distribution<int> rc(1, 5); //랜덤한 초를 정하고
          rt = rc(gen);
          t_ck = 1; //시간이 측정됨을 알림
        }
        if (tga_ck == 0 && gateAssign == 0){ //(gate
          start_tga = time(NULL);
          uniform_int_distribution<int> rg(2, 6); //랜덤한 초를 정하고
          rga = rg(gen);
          tga_ck = 1; //시간이 측정됨을 알림
		    }
      }
      if (t_ck == 1 && time(NULL)-start_t >= rt) { //시간이 측정되어있는 상태이면서 랜덤한 초 이상이 지났을 경우
        if (snake.itemTmp() < 3) { //아이템이 3개이하 있을 때
          stop_t = time(NULL); //stop지점을 구하여 deque에 담아줌
          dq_t.push_back(stop_t);
          snake.assign_I(); //아이템 찍음
        }
        t_ck = 0;// 다시 시간 측정 X로 지정
      }
      if (tga_ck == 1 && time(NULL) - start_tga >= rga){
        stop_tga = time(NULL);
        snake.gate_xy();
        gateAssign = 1;
        tga_ck = 0;
      }

      if(!dq_t.empty()){ //deque가 비어있지 않은 경우
          auto it = dq_t.begin(); //iterator 초기화

        for (;it != dq_t.end(); it++){ //stop지점이 담긴 deque를 검사하여 5초이상인 부분의 아이템을 지우도록 함
          t = *it;
          if (time(NULL) - t >= 5){ //5초 이상인 경우
            x = q_xy.front().first; y = q_xy.front().second; //x, y값을 받아
            snake.setZero(x, y); //아이템 삭제
            q_xy.pop();
            dq_t.pop_front(); //아이템과, stop지점을 각 containor에서 제외
          }
        }
      }
    }
    catch (int e){ //error처리를 통해 game함수에서 벗어난 후 while문을 종료시킴
      err = e;
      break;
    }
  }

  switch (err){
    case 1:
      mvprintw(22, 12, "벽에 닿았습니다!!");
      break;
    case 2:
      mvprintw(22, 12, "진행방향의 반대방향으로는 갈 수 없습니다!!");
      break;
    case 3:
      mvprintw(22, 12, "자신의 몸을 통과할 수 없습니다!!");
      break;
    case 4:
      mvprintw(22, 12, "꼬리의 길이가 짧습니다!!");
      break;
    case 5:
      mvprintw(22, 12, "게임 클리어~ 축하합니다ㅜ");
      break;
  }
  getch();
	if(err < 5 && err > 0) mvprintw(24, 12, "GameOver!\n 종료하려면 아무키나 눌러주세요!\n");
  getch();
  endwin();
}

Snake::Snake(int stage)
{
  for (int i = 0; i < ARR_Y; i++){
    for (int j = 0; j < ARR_X; j++) map[i][j] = stage1[i][j];
  } //stage마다 정하기

  get_Cnt(stage);
  int poison_Cnt = 0;
  int growth_Cnt = 0;
  int gate_Cnt = 0;

  dir[0].x = 0; dir[0].y = -1;
  dir[1].x = 1; dir[1].y = 0;
  dir[2].x = 0; dir[2].y = 1;
  dir[3].x = -1; dir[3].y = 0;//90-93 방향을 구조체 배열로 지정

  s[0].x = 19; s[1].x = 20; s[2].x = 21;
  s[0].y = 10; s[1].y = 10; s[2].y = 10; //95-96 초기좌표 고정
  next_X = dir[3].x; next_Y = dir[3].y; d_op = 3; //다음방향 지정
  fail_dir = 67; game_result = -1; tail = 3; //꼬리길이(나중에 새로운 꼬리를 추가하거나 제거할 때 이용)
}

void Snake::Map(){
//벽 : 1 , 모서리 : 2, 머리 : 3, 꼬리 : 4, growth : 5, poison : 6, gate : 7 (배열기준)

  setlocale(LC_ALL, "");
  initscr();
  start_color();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK); //모서리
  init_pair(2, COLOR_BLUE, COLOR_BLACK); //머리
  init_pair(3, COLOR_CYAN, COLOR_BLACK); //꼬리들
  init_pair(4, COLOR_GREEN, COLOR_BLACK); //growth
  init_pair(5, COLOR_RED, COLOR_BLACK); //poison
  init_pair(6, COLOR_MAGENTA, COLOR_BLACK); //gate
  resize_term(100, 100); //(팔레트 번호)

  if (tail < 3) throw GameOver(4); //꼬리의 길이가 3미만인경우

  for (int i = 0; i < tail; i++){ //snake 특징 설정
    if (i == 0) {
      if (map[s[i].y][s[i].x] == 7) check_gate();
      else{
        if (map[s[i].y][s[i].x] == 1) throw GameOver(1);//벽에 닿을 경우
        if (map[s[i].y][s[i].x] == 4) throw GameOver(3); //자기 꼬리 닿을경우
        if (map[s[i].y][s[i].x] == 5) growth();
        if (map[s[i].y][s[i].x] == 6) poison();
        map[s[i].y][s[i].x] = 3; // 만약 머리일 경우 3으로 정해준다
      }
	  }
	  else map[s[i].y][s[i].x] = 4; //꼬리들은 4로 정해준다
  }
  for (int i = 0; i < 21; i++){
    for (int j = 0; j < 40; j++){ //map을 돌기 위한 2중
			if(map[i][j] == 1) {
				wall.push_back(pair<int, int>(i,j));
			}
		}
  }

  for(int i = 0; i < ARR_Y; i++){
    for(int j = 0; j < ARR_X; j++){
      if (map[i][j] == 2){
          attron(COLOR_PAIR(1));
          mvprintw(i, j, "\u2B1B");
          attroff(COLOR_PAIR(1));
      }
      else if (map[i][j] == 3){
          attron(COLOR_PAIR(2));
          mvprintw(i, j, "\u2B1B");
          attroff(COLOR_PAIR(2));
      }
      else if (map[i][j] == 4){
          attron(COLOR_PAIR(3));
          mvprintw(i, j, "\u2B1C");
          attroff(COLOR_PAIR(3));
      }
      else if (map[i][j] == 5){
          attron(COLOR_PAIR(4));
          mvprintw(i, j, "\u2B1B");
          attroff(COLOR_PAIR(4));
      }
      else if (map[i][j] == 6){
          attron(COLOR_PAIR(5));
          mvprintw(i, j, "\u2B1B");
          attroff(COLOR_PAIR(5));
      }
      else if (map[i][j] == 7){
          attron(COLOR_PAIR(6));
          mvprintw(i, j, "\u2B1B");
          attroff(COLOR_PAIR(6));
      }
      else if (map[i][j] == 0) mvprintw(i, j, " ");
      else if (map[i][j] == 1) mvprintw(i, j, "\u2B1B"); //map을 화면에 표시하여 준다.(각각 성질에 맞추어서)
    }
  }
  refresh();
}
void Snake::Score(){
  char ck_tail = ' ', ck_grow = ' ', ck_poi = ' ', ck_gate = ' ';
  WINDOW *win, *win2;
	int cnt = 0;

	cout << "<stage>  " << stage_chk << endl;

  if (tail >= TAIL) {ck_tail = 'v'; cnt += 1;}
  if (growth_Cnt >= GROWTH) {ck_grow = 'v'; cnt += 1;}
  if (poison_Cnt >= POISON) {ck_poi = 'v'; cnt += 1;}
  if (gate_Cnt >= GATE) {ck_gate = 'v'; cnt += 1;}


  win = newwin(10, 30, 0, 50);
  mvwprintw(win, 1, 5, "  ###ScoreBoard###");
  mvwprintw(win, 2, 2, "\u0042 : %d / %d", tail, TAIL);
  mvwprintw(win, 4, 2, "\u002B : %d", growth_Cnt );
  mvwprintw(win, 6, 2, "\u002D : %d", poison_Cnt);
  mvwprintw(win, 8, 2, "\u0047 : %d", gate_Cnt);

  win2 = newwin(10, 30, 11, 50);
  mvwprintw(win2, 1, 5, "     ###MISION###");
  mvwprintw(win2, 2, 2, "\u0042 : %d", TAIL);
  mvwprintw(win2, 2, 8, "\u0028%c\u0029", ck_tail);
  mvwprintw(win2, 4, 2, "\u002B : %d", GROWTH);
  mvwprintw(win2, 4, 8, "\u0028%c\u0029", ck_grow);
  mvwprintw(win2, 6, 2, "\u002D : %d", POISON);
  mvwprintw(win2, 6, 8, "\u0028%c\u0029", ck_poi);
  mvwprintw(win2, 8, 2, "\u0047 : %d", GATE);
  mvwprintw(win2, 8, 8, "\u0028%c\u0029", ck_gate);

  wborder(win, '-','-','-','-','+','+','+','+');
  wborder(win2, '-','-','-','-','+','+','+','+');

	if(cnt == 4) {
		stage_chk += 1;
		switch (stage_chk) {
			case 2:
				for (int i = 0; i < ARR_Y; i++){
					for (int j = 0; j < ARR_X; j++){
						map[i][j] = stage2[i][j];
					}
				}
				break;
			case 3:
				for (int i = 0; i < ARR_Y; i++){
					for (int j = 0; j < ARR_X; j++){
						map[i][j] = stage3[i][j];
					}
			}
			break;
			case 4:
				for (int i = 0; i < ARR_Y; i++){
					for (int j = 0; j < ARR_X; j++){
						map[i][j] = stage4[i][j];
					}
				}
				break;
		}
		get_Cnt(stage_chk);
		cnt = 0;
		if (stage_chk == 4){
			cout << "Game Clear" << endl;
			GameOver(5);
		}
	}

  wrefresh(win);wrefresh(win2);
}

void Snake::Game(){
  int tmp_x, tmp_y;
  int ch;

  keypad(stdscr, TRUE);
  if (_kbhit()) ch = _getch();
  else ch = -1;

  if (ch == fail_dir) throw GameOver(2);

  switch(ch){ //ch에 따라 next dir을 설정함
    case 65: //상 260
      next_X = dir[0].x; next_Y = dir[0].y; fail_dir = 66;
      d_op = 0;
      break;
    case 67: //오른쪽 258
      next_X = dir[1].x; next_Y = dir[1].y; fail_dir = 68;
      d_op = 1;
      break;
    case 66: //하 261
      next_X = dir[2].x; next_Y = dir[2].y; fail_dir = 65;
      d_op = 2;
      break;
    case 68: //왼쪽 259
      next_X = dir[3].x; next_Y = dir[3].y; fail_dir = 67;
      d_op = 3;
      break;
    case -1:
      break;
    default:
      break;
		}

  map[s[tail-1].y][s[tail-1].x] = 0;
  for(int i = tail; i >= 1; i--){
    tmp_x = s[i-1].x; tmp_y = s[i-1].y;
    s[i].x = tmp_x; s[i].y = tmp_y;
  } //snake꼬리들은 그 전의 좌표를 가지고감
  s[0].x += next_X; s[0].y += next_Y; //snake 머리만 다음 방향을 적용함
}

int Snake::GameOver(int ch){
//game over 1 : 벽에 닿을때, 2 : 반대방향으로 갈 때, 3 : 자기 몸 통과, 4 : 꼬리 2 이하일때 5: 성공
  game_result = ch;
  return game_result;
}

//random으로 시간, 아이템 몇개나올지 구하는 함수에서 리턴해줌, 리턴받은 시간이 지나면 몇개나오는지 구하는 만큼
//랜덤으로 좌표 랜덤개만큼, 랜덤한 좌표에 배정
//5초 지나면 사라지고 다시 반복

int Snake::itemTmp(){ //item개수 구하기
  int item_c = 0; //아이템 개수를 담는 변수

  for (int i = 0; i < ARR_Y; i++){
    for (int j = 0; j < ARR_X; j++){
      if (map[i][j] > 4 && map[i][j] != 7) item_c += 1; //4이상인 경우 아이템이므로 아이템 갯수를 추가해줌
    }
  }

  return item_c; //item개수 반환
}

void Snake::assign_I(){ //item의 종류 고르고 좌표설정
  int item_option; //item이 growth인지 poison인지 gate인지 할당하는 변수
  int point_x, point_y; //좌표
  pair<int, int> p; //x, y좌표를 pair로 묶어주는 변수
  random_device rd;
  mt19937 gen(rd());

  point_x = getXPoint();
  point_y = getYPoint(); ///209-10 : 좌표 설정
  p = make_pair(point_x, point_y);

  uniform_int_distribution<int> d_T(5, 6); //growth 인지 poison인지
  item_option = d_T(gen);

  if (map[point_y][point_x] == 0) {
    map[point_y][point_x] = item_option;  //좌표가 벽이나 snake 인 경우 다시 좌표 설정
    q_xy.push(p); }//x, y좌표를 pair로 묶어서 queue(q_xy)에 push
}

int Snake::getXPoint(){ //X좌표 랜덤설정
  int point_x;
  random_device rd;
  mt19937 gen(rd());

  //x좌표 1~39 -> 나중에 const int로 배열값 고정하ㄱㅣ
  uniform_int_distribution<int> d_X(1, 39);
  point_x = d_X(gen);

  return point_x;
}
int Snake::getYPoint(){ //Y좌표 랜덤설정
  int point_y;
  random_device rd;
  mt19937 gen(rd());

  //y좌표 1~20 -> 나중에 const int로 배열값 고정하ㄱㅣ
  uniform_int_distribution<int> d_Y(1, 20);
  point_y = d_Y(gen);

  return point_y;
}

void Snake::growth(){
  growth_Cnt += 1;
  //방향에 따라 지정해줘야함
  switch(d_op){//d_op = 0 : up, 1: right, 2: down, 3: left
    case 0:
      s[tail].x = s[tail - 1].x + dir[d_op].x;
      s[tail].y = s[tail - 1].y + dir[d_op].y;
      break;
    case 1:
      s[tail].x = s[tail - 1].x + dir[d_op].x;
      s[tail].y = s[tail - 1].y + dir[d_op].y;
      break;
    case 2:
      s[tail].x = s[tail - 1].x + dir[d_op].x;
      s[tail].y = s[tail - 1].y + dir[d_op].y;
      break;
    case 3:
      s[tail].x = s[tail - 1].x + dir[d_op].x;
      s[tail].y = s[tail - 1].y + dir[d_op].y;
      break;
  }
  tail += 1;
}

void Snake::poison(){ //poison 아이템 먹으면 배열에 0 할당후 꼬리길이 감소
  poison_Cnt += 1;
  map[s[tail - 1].y][s[tail-1].x] = 0;
  tail -= 1;
}

void Snake::setZero(int x, int y){ //들어온 좌표를 0으로 만들어줌
  map[y][x] = 0;
}
void Snake::gate_xy(){
		vector<pair<int, int>>::iterator iter;
		
		random_device rd;
	  	mt19937 gen(rd());
		shuffle(wall.begin(), wall.end(), gen); // vector에 모든 wall 좌표에 대한 pair을 만들어서 임의로 섞는다 
		iter = wall.begin();

		g1_y = iter->first;
		g1_x = iter->second;
		map[g1_y][g1_x] = 7; // map 1 -> 7

	  iter = wall.begin()+1; // gate2
		g2_y = iter->first;
		g2_x = iter->second;
		map[g2_y][g2_x] = 7;

}

/////////////////////////////////////////////////
void Snake::check_gate(){ //현재 게이트 & 머리 좌표를 확인한 후 다음 게이트의 좌표를 넘겨줌
	if(s[0].x == g1_x && s[0].y == g1_y){
		is_Edge(g2_y, g2_x);
	}else if(s[0].x == g2_x && s[0].y == g2_y){
		is_Edge(g1_y, g1_x);
	}
}
void Snake::is_Edge(int ny, int nx){ //가장 자리인지 확인하는 함수
	int chk = 0;
	if(ny == 0 && (1 <= nx && nx < ARR_X-1 )){
		Exit_gate(nx, ny, 1);
	}else if(ny == ARR_Y-1 && (1 <= nx && nx < ARR_X-1)){
		Exit_gate(nx, ny, 2);
	}else if((1 <= ny && ny < ARR_Y ) && nx == 0){
		Exit_gate(nx, ny, 3);
	}else if((1 <= ny && ny < ARR_Y) && nx == ARR_X-1){
		Exit_gate(nx, ny, 4);
	}else{ //가장 자리가 아닐 시에 처리하는 함수
			if(fail_dir == 66){ // 진입 방향 = 상
				if(map[ny+1][nx] == 1){Exit_gate(nx, ny, 3);} // 진출방향 = 우
				else if(map[ny+1][nx-1] != 1){Exit_gate(nx, ny, 2);} // (진출 방향이 막혀있지 않을 때) 방향 = 상(그대로)
			}
			else if(fail_dir == 65){ // 진입 방향 = 하
				// 진출 게이트 아래 좌표가 벽인 경우
				if(map[ny-1][nx] == 1) {
					Exit_gate(nx, ny, 4);} // 진출방향 = 좌
				else if(map[ny-1][nx] != 1){Exit_gate(nx, ny, 1);} // (진출 방향이 막혀있지 않을 때) 방향 = 하(그대로)
			}
			else if(next_X == dir[3].x){ // 진입 방향 = 좌
				// 진출 게이트 왼쪽 좌표가 벽인
				if(map[ny][nx-1] == 1) {Exit_gate(nx, ny, 2);} // 진출방향 = 상
				else if(map[ny][nx-1] != 1){Exit_gate(nx, ny, 4);}
			}
			else if(next_X == dir[1].x){ // 진입 방향 = 우
				// 진출 게이트 오른쪽 좌표가 벽인 경우
				if(map[ny][nx+1] == 1) {Exit_gate(nx, ny, 1);} // 진출방향 = 하
				else if(map[ny][nx+1] != 1){Exit_gate(nx, ny, 3);}
			}
	}
}

void Snake::Exit_gate(int x, int y, int d){
	int tmp_x,tmp_y;
	switch(d){
		case 1: //up -> down
		next_X = dir[2].x; next_Y = dir[2].y; fail_dir = 65;
		break;
		case 2: //down -> up
		next_X = dir[0].x; next_Y = dir[0].y; fail_dir = 66;
		break;
		case 3: //left -> right
		next_X = dir[1].x; next_Y = dir[1].y; fail_dir = 68;
		break;
		case 4: //right -> left
		next_X = dir[3].x; next_Y = dir[3].y; fail_dir = 67;
		break;
	}
	//////////////////이동//////////////////////////
	map[s[tail-1].y][s[tail-1].x] = 0;
	s[0].x = next_X + x; s[0].y = next_Y + y; //snake의 머리를 gate 안쪽 좌표로 설정
	for(int i = tail; i >= 1; i--){
    tmp_x = s[i-1].x; tmp_y = s[i-1].y;
    s[i].x = tmp_x; s[i].y = tmp_y;
  } //snake꼬리들은 그 전의 좌표를 가지고감
  gate_Cnt += 1;
	gateAssign = 0;
	map[g1_y][g1_x] = 1;
	map[g2_y][g2_x] = 1;
}

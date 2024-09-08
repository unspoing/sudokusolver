#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>            //for lines
#include <Fl/Fl_Multiline_Input.H> //fl_message
#include <Fl/fl_ask.H>
#include <cmath>   //floor divide
#include <fstream> //reading sudoku data
#include <iostream>
#include <set>     //checking for dupe numbers
#include <stdio.h> //for spintf
using namespace std;
using namespace std::chrono;

class Sudoku : public Fl_Window {
public:
  Sudoku(int x, int y, int w, int h, const char *l);
  Fl_Box *boxs[9][9];
  char grid[10][10];
  bool manual;
  bool invalidpuzzle;
  Fl_Button *solveagain;
  Fl_Return_Button *sol;
  Fl_Multiline_Input *manualenter;
  Fl_Box *instructiontext;
  Fl_Button *transfernums;

private:
  static void sol_cb(Fl_Widget *o, void *v);
  static void reopenselector(Fl_Widget *o, void *v);
  static void numstomainboard_cb(Fl_Widget *o, void *v);
  void numstomainboard(Fl_Multiline_Input *wid);
  void sol_cbi(Fl_Return_Button *wid);
  bool solve(); // recursive
  void ReadFile();
  bool checkvalid();
  bool checkfornonnums();
  void inputnums();
  void draw();                      // for black lines separating boxes
  void getcol(char *vcol, int col); // get values in column col which are not 0
  void getrow(char *vrow, int row); // get values in row which are not 0
  void getbox(char *vbox, int row,
              int col);             // get values in box which are not 0
  bool findEmpty(int &re, int &ce); // return true if found an empty cell
  void getpvals(char *pvals, char *vrow, char *vcol, char *vbox);
  int stack;
};
void Sudoku::draw() {
  // draws horiz and vertcal separator lines
  Fl_Window::draw();
  fl_color(FL_BLACK);
  fl_line_style(FL_SOLID, 5);
  fl_line((w() - 120) / 3, 0, (w() - 120) / 3,
          h() - 40); // vertical left (top-down)
  fl_line(2 * (w() - 120) / 3, 0, 2 * (w() - 120) / 3,
          h() - 40); // vertical right
  fl_line(0, (h() - 40) / 3, (w() - 120),
          (h() - 40) / 3); // horiz top(left - right)
  fl_line(0, 2 * (h() - 40) / 3, (w() - 120),
          2 * (h() - 40) / 3); // horiz bottom
}

Sudoku::Sudoku(int x, int y, int w, int h, const char *l = "Sudoku Solver")
    : Fl_Window(x, y, w, h, l) {
  begin();
  for (int r = 0; r < 9; r++) {
    for (int c = 0; c < 9; c++) {
      boxs[r][c] = new Fl_Box(c * 40, r * 40, 40, 40);
      boxs[r][c]->box(FL_UP_BOX);
    }
  }
  transfernums = new Fl_Button(360, 340, 120, 60);
  transfernums->label("Transfer Numbers\nAbove to\nMain Board");
  transfernums->callback(numstomainboard_cb, this);
  instructiontext = new Fl_Box(360, 60, 120, 90);
  instructiontext->label(
      "Manually Enter\na Puzzle Below\n(Use 0 for Empty\nSpaces)");
  manualenter = new Fl_Multiline_Input(360, 150, 120, 190);
  manualenter->textsize(20);
  manualenter->textfont(FL_COURIER_BOLD);
  solveagain = new Fl_Button(360, 0, 120, 60);
  solveagain->label("Pick a Puzzle\nto Solve");
  solveagain->callback(reopenselector, this);
  sol = new Fl_Return_Button(0, 360, 360, 40);
  sol->label("Solve");
  sol->callback(sol_cb, this);
  // ReadFile();
  end();
  // resizable(this);
  show();
}

void Sudoku::sol_cb(Fl_Widget *o, void *v) {
  // static functions don't have "this" pointer. But we sent it through with v
  Sudoku *win = (Sudoku *)v;
  Fl_Return_Button *b = (Fl_Return_Button *)o;
  win->sol_cbi(b);
}

void Sudoku::reopenselector(Fl_Widget *o, void *v) {
  Sudoku *repeat = (Sudoku *)v;
  Fl_Button *b = (Fl_Button *)o;
  repeat->ReadFile();
}

void Sudoku::sol_cbi(Fl_Return_Button *wid) {
  cout << manualenter->value() << endl;
  if (manual != true) {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        grid[i][j] = '0';
      }
      manual = false;
    }
  }

  invalidpuzzle = false;
  if (checkvalid() != true) {
    fl_message("Invalid Puzzle, Duplicate Numbers in a Row/Column/Box");
    invalidpuzzle = true;

  } else if (checkfornonnums() != true) {
    fl_message("Invalid Puzzle, Ensure Board Contains Numbers Only");
    invalidpuzzle = true;
  }

  else {
    if (solve() == false) {
      fl_message(
          "Invalid Puzzle, Check if All Numbers Are in the Correct Space");
    }
  }
}

bool Sudoku::findEmpty(int &re, int &ce) {
  for (re = re; re < 9; re++) {
    for (ce = 0; ce < 9; ce++) {
      if (grid[re][ce] == '0') {
        return true;
      }
    }
  }
  return false;
}
void Sudoku::getbox(char *vbox, int re, int ce) {
  int newr = floor(re / 3) * 3;
  int newc = floor(ce / 3) * 3;
  int c = 0;
  for (int i = newr; i < (newr + 3); i++) {
    for (int j = newc; j < (newc + 3); j++) {
      vbox[c] = grid[i][j];
      c++;
    }
  }
}
void Sudoku::getcol(char *vcol, int ce) {
  for (int i = 0; i < 9; i++) {
    vcol[i] = grid[i][ce];
  }
}

void Sudoku::getrow(char *vrow, int re) {
  for (int i = 0; i < 9; i++) {
    vrow[i] = grid[re][i];
  }
}

void delChar(char *s, char c) { // c is char to del from s
  int write = 0;
  int read = 0;

  while (s[read]) {
    if (s[read] != c) {
      s[write] = s[read]; // if not char to del then overwrite char from read
                          // index to write index
      write++;            // only inc write index if not char to del
               //  So after char to del, write index is < read index. Hence,
               //  char to del will be overwritten
    }
    read++; // always inc read char
  }
  s[write] = '\0';
}

void Sudoku::getpvals(char *pvals, char *vrow, char *vcol, char *vbox) {
  // find possible values for cell given row, col and box values
  // start with values 1-9 and remove values in box, row and col
  strcpy(pvals, "0123456789");

  for (int i = 0; i < sizeof(vrow) + 1; i++) {
    delChar(pvals, vrow[i]);
  }
  for (int i = 0; i < sizeof(vcol + 1); i++) {
    delChar(pvals, vcol[i]);
  }
  for (int i = 0; i < sizeof(vbox) + 1; i++) {
    delChar(pvals, vbox[i]);
  }
}

bool Sudoku::checkvalid() {
  for (int i = 0; i < 9; i++) {
    set<char> checkrow;
    for (int j = 0; j < 9; j++) {
      if (checkrow.find(grid[i][j]) != checkrow.end()) {
        return false;
      } else {
        if (grid[i][j] != '0') {
          checkrow.insert(grid[i][j]);
        }
      }
    }
  }

  for (int i = 0; i < 9; i++) {
    set<char> checkcol;
    for (int j = 0; j < 9; j++) {
      if (checkcol.find(grid[j][i]) != checkcol.end()) {
        return false;
      } else {
        if (grid[j][i] != '0') {
          checkcol.insert(grid[j][i]);
        }
      }
    }
  }
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      set<char> checkbox;
      int x = i * 3;
      int y = j * 3;
      for (int k = 0; k < 3; k++) {
        for (int l = 0; l < 3; l++) {
          if (checkbox.find(grid[x + k][y + l]) != checkbox.end()) {
            return false;
          } else {
            if (grid[x + k][y + l] != '0') {
              checkbox.insert(grid[x + k][y + l]);
            }
          }
        }
      }
    }
  }

  return true;
}

bool Sudoku::checkfornonnums() {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (isdigit(grid[i][j]) == false) {
        return false;
      }
    }
  }
  return true;
}

void Sudoku::numstomainboard_cb(Fl_Widget *o, void *v) {
  // static functions don't have "this" pointer. But we sent it through with v
  Sudoku *transfer = (Sudoku *)v;
  Fl_Multiline_Input *i = (Fl_Multiline_Input *)o;
  transfer->numstomainboard(i);
}

void Sudoku::numstomainboard(Fl_Multiline_Input *i) {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      grid[i][j] = '0';
      boxs[i][j]->label("");
      boxs[i][j]->color(FL_WHITE);
    }
  }

  manual = true;
  string vals = manualenter->value();
  if (vals.length() == 89) {
    int stringindex = 0;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 10; j++) {
        if (j == 9) {
          stringindex = stringindex + 1;
          continue;
        }
        grid[i][j] = vals[stringindex];
        if (vals[stringindex] != '0') {
          char label[2];
          label[0] = vals[stringindex];
          label[1] = '\0';
          boxs[i][j]->copy_label(label);
          boxs[i][j]->color(FL_GRAY);
        }
        cout << grid[i][j] << endl;
        stringindex = stringindex + 1;
      }
    }
  } else {
    fl_message("Invalid Puzzle - Input a Full 9x9 Grid of Numbers\n(Ensure "
               "that there are 9 rows of 9 numbers only)");
  }
}
bool Sudoku::solve() {
  if (invalidpuzzle == true) {
    return true;
  }
  int re(0);
  int ce(0);
  if (findEmpty(re, ce) == false) {
    cout << "You and me 내 맘이 보이지?" << endl
         << "한참을 쳐다봐, 가까이 다가가 you see (ey-yeah)" << endl
         << "You see, ey, ey, ey, ey" << endl
         << "One, two, three 용기가 생겼지 " << endl
         << "이미 아는 니 눈치" << endl
         << "고개를 돌려 천천히, 여기" << endl
         << "You see 여기 보이니?" << endl
         << "Looking for attention 너야겠어" << endl
         << "확실하게 나로 만들겠어 stop" << endl
         << "Ey, drop the question" << endl
         << "Drop the, drop the question" << endl
         << "Want attention" << endl
         << "Wanna want attention" << endl
         << "You give me butterflies, you know" << endl
         << "내 맘은 온통 paradise" << endl
         << "꿈에서 깨워주지 마" << endl
         << "You got me looking for attention" << endl
         << "You got me looking for attention" << endl
         << "가끔은 정말 헷갈리지만 분명한 건" << endl
         << "Got me looking for attention" << endl
         << "널 우연히 마주친 척할래" << endl
         << "못 본 척 지나갈래" << endl
         << "You're so fine (ey)" << endl
         << "Gotta, gotta get to know ya" << endl
         << "나와, 나와 걸어가 줘" << endl
         << "지금 돌아서면 I need ya, need ya, need ya" << endl
         << "To look at me back" << endl
         << "Hey 다 들켰었나?" << endl
         << "널 보면 하트가 튀어나와" << endl
         << "난 사탕을 찾는 baby (baby)" << endl
         << "내 맘은 설레이지" << endl
         << "Ey, drop the question" << endl
         << "Drop the, drop the question" << endl
         << "Want attention" << endl
         << "Wanna want attention" << endl
         << "You give me butterflies, you know" << endl
         << "내 맘은 온통 paradise" << endl
         << "꿈에서 깨워주지 마 (one, two, three ey)" << endl
         << "You got me looking for attention" << endl
         << "You got me looking for attention" << endl
         << "가끔은 정말 헷갈리지만 분명한 건" << endl
         << "Got me looking for attention" << endl
         << "You got me looking for attention" << endl
         << "You got me looking for attention" << endl
         << "가끔은 정말 헷갈리지만 분명한 건" << endl
         << "Got me looking for attention" << endl
         << "A-T-T-E-N-T-I-ON" << endl
         << "Attention is what I want" << endl
         << "A-T-T-E-N-T-I-ON" << endl
         << "Attention is what I want" << endl
         << "A-T-T-E-N-T-I-ON" << endl
         << "Attention is what I want" << endl
         << "A-T-T-E-N-T-I-ON" << endl
         << "You got me looking for attention" << endl;
    return true;
  }
  char r[9] = "\0";
  char c[9] = "\0";
  char b[9] = "\0";
  char nums[10] = "\0";
  getcol(c, ce);
  getrow(r, re);
  getbox(b, re, ce);
  getpvals(nums, r, c, b);
  char num[2];
  for (int i = 0; i < strlen(nums); i++) {
    num[0] = nums[i];
    num[1] = '\0';
    boxs[re][ce]->labeltype(FL_NORMAL_LABEL);
    boxs[re][ce]->copy_label(num);
    boxs[re][ce]->redraw();
    grid[re][ce] = *num;
    if (solve() == true) {
      return true;
    } else {
      boxs[re][ce]->label("");
      grid[re][ce] = '0';
    }
  }

  return false;
}

void Sudoku::ReadFile() {

  const char *fname = fl_file_chooser("Pick a File", "*.txt", "");
  char n;
  char c[2];
  ifstream f(fname);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      grid[i][j] = 0;
      boxs[i][j]->labeltype(FL_NORMAL_LABEL);
      boxs[i][j]->color(FL_GRAY);
    }
  }
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      f.get(n);
      c[0] = n;
      c[1] = '\0';
      boxs[i][j]->copy_label(c);
      grid[i][j] = n;
      if (n == '0') {
        boxs[i][j]->labeltype(FL_NO_LABEL);
        boxs[i][j]->color(FL_WHITE);
      }
    }
    f.get(n);
  }
  f.close();
}

int main() {
  Sudoku s(0, 0, 480, 400);
  Fl::run();
}

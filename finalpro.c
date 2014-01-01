#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <stdio.h>
#include <rowshift.h>

//--------Find GCD function --------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
   unsigned long int c;
   while(1){
       c = a%b;
       if(c==0){return b;}
       a = b;

b = c;
   }
   return 0;
}
//--------End find GCD function ----------------------------------------------

// Overhead
int count = 0;
unsigned char *LCD_Data = &PORTD;    // LCD 8-bit data bus
unsigned char *LCD_Ctrl = &PORTB;    // LCD needs 2-bits for control, use port B
const unsigned char LCD_RS = 3;        // LCD Reset pin is PB3
const unsigned char LCD_E = 4;        // LCD Enable pin is PB4

unsigned char LCD_rdy_g = 0; // Set by LCD interface synchSM, ready to display new string
unsigned char LCD_go_g = 0; // Set by user synchSM wishing to display string in LCD_string_g
unsigned char LCD_string_g[64]; // Filled by user synchSM, 16 chars plus end-of-string char

unsigned char MAP_SIZE = 30;

typedef struct
{
  unsigned char attack;
  unsigned char defense;
  unsigned char speed;
  unsigned char life;
  signed char pos_x;
  signed char pos_y;
} player;

typedef struct
{
   unsigned char x;
   unsigned char y;
}door;

unsigned char map[30][30] =  { {1,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
                               {0,0,0,0,0,1,0,0,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1},
                               {0,1,0,0,0,1,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
                               {0,1,1,1,1,1,0,0,0,0,1,0,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1},
                               {0,0,0,0,0,1,0,0,1,1,1,3,1,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,1},
                               {1,1,1,1,0,1,1,1,1,0,0,0,0,0,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1,1},
                               {0,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,1,1,1,1,0,1,0,0,0,1,0,0,0,1},
                               {0,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1},
                               {0,0,0,0,1,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
                               {1,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                               {0,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1},
                               {0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,0,0,1,0,0,0,1,1,1,0,1,0,0,0,1},
                               {0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,1},
                               {1,1,0,1,3,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,0,1,0,1},
                               {0,0,0,1,0,0,0,0,1,0,3,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0},
                               {0,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
                               {0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
                               {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1},
                               {0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
                               {0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,1},
                               {0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,3,0,1,0,0,0,0,1},
                               {1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,1},
                               {0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,0,1},
                               {0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1},
                               {0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1},
                               {0,1,0,1,0,0,0,1,0,1,1,1,1,0,1,0,1,0,1,0,0,0,1,1,1,1,1,1,0,1},
                               {0,1,0,1,0,0,0,1,0,1,0,0,1,0,1,0,1,0,1,1,1,1,1,0,0,0,0,0,0,1},
                               {0,1,0,1,0,0,0,1,0,1,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
                               {0,1,3,1,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,1,1,0,1,0,0,0,0,0,0,1},
                               {0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1} };

unsigned char my_view[8][8];

player user, enemy1, enemy2, enemy3, enemy4, enemy5, enemy6;
door d1, d2, d3, d4, d5;

player enemies[6];
door doors[5];

unsigned char enemy_index = 0;
unsigned char counter = 0;
unsigned char final_battle_flag = 0;
unsigned char my_defend_used = 0;
unsigned char enemy_defend_used = 0;
unsigned char is_my_turn = 0;
player* enemy;

unsigned char GetKeypadKey()
{
  PORTA = 0xEF; // Enable col 4 with 0, disable others with 1?s
  asm("nop"); // add a delay to allow PORTC to stabilize before checking
  if (GetBit(PINA,0)==0) return('1');
  if (GetBit(PINA,1)==0) return('4');
  if (GetBit(PINA,2)==0) return('7');
  if (GetBit(PINA,3)==0) return('*');

  // Check keys in col 2
  PORTA = 0xDF; // Enable col 5 with 0, disable others with 1?s
  asm("nop"); // add a delay to allow PORTC to stabilize before checking
  if (GetBit(PINA,0)==0) return('2');
  if (GetBit(PINA,1)==0) return('5');
  if (GetBit(PINA,2)==0) return('8');
  if (GetBit(PINA,3)==0) return('0');

  // Check keys in col 3
  PORTA = 0xBF; // Enable col 6 with 0, disable others with 1?s
  asm("nop"); // add a delay to allow PORTC to stabilize before checking
  if (GetBit(PINA,0)==0) return('3');
  if (GetBit(PINA,1)==0) return('6');
  if (GetBit(PINA,2)==0) return('9');
  if (GetBit(PINA,3)==0) return('#');

  // Check keys in col 4
  PORTA = 0x7F; // Enable col 6 with 0, disable others with 1?s
  asm("nop"); // add a delay to allow PORTC to stabilize before checking
  if (GetBit(PINA,0)==0) return('A');
  if (GetBit(PINA,1)==0) return('B');
  if (GetBit(PINA,2)==0) return('C');
  if (GetBit(PINA,3)==0) return('D');

  return('\0'); // default value
}

void Write7Seg(unsigned char x)
{
  // Define this function. It should set PORTC (bits 6..0)
  // Use a switch to set set PORTC to the appropriate hex value.
  unsigned char numout = 0x00;

  if(x == 0) numout = 0x7E;
  else if(x == 1) numout = 0x48;
  else if(x == 2) numout = 0x3D;
  else if(x == 3) numout = 0x6D;
  else if(x == 4) numout = 0x4B;
  else if(x == 5) numout = 0x67;
  else if(x == 6) numout = 0x77;
  else if(x == 7) numout = 0x4C;
  else if(x == 8) numout = 0x7F;
  else if(x == 9) numout = 0x6F;
  else numout = 0x80;
  seg7Write(numout);
}

void show_flags()
{
  Write7Seg(enemy_index);
}

void initialize_game()
{
  enemy_index = 0;
  final_battle_flag = 0;
  counter = 0;
  my_defend_used = 0;
  enemy_defend_used = 0;
  is_my_turn = 0;
  enemy = &enemies[enemy_index];

  user.attack = 3;
  user.defense = 3;
  user.speed = 3;
  user.life = 6;
  user.pos_x = 28;
  user.pos_y = 24;

  enemy1.attack = 2;
  enemy1.defense = 2;
  enemy1.speed = 1;
  enemy1.life = 6;
  enemy1.pos_x = 21;
  enemy1.pos_y = 27;

  enemy2.attack = 5;
  enemy2.defense = 2;
  enemy2.speed = 3;
  enemy2.life = 6;
  enemy2.pos_x = 17;
  enemy2.pos_y = 9;

  enemy3.attack = 6;
  enemy3.defense = 4;
  enemy3.speed = 4;
  enemy3.life = 6;
  enemy3.pos_x = 26;
  enemy3.pos_y = 5;

  enemy4.attack = 7;
  enemy4.defense = 4;
  enemy4.speed = 5;
  enemy4.life = 6;
  enemy4.pos_x = 15;
  enemy4.pos_y = 14;

  enemy5.attack = 8;
  enemy5.defense = 6;
  enemy5.speed = 6;
  enemy5.life = 6;
  enemy5.pos_x = 1;
  enemy5.pos_y = 3;

  enemy6.attack = 10;
  enemy6.defense = 6;
  enemy6.speed = 7;
  enemy6.life = 6;
  enemy6.pos_x = 2;
  enemy6.pos_y = 7;

  enemies[0] = enemy1;
  enemies[1] = enemy2;
  enemies[2] = enemy3;
  enemies[3] = enemy4;
  enemies[4] = enemy5;
  enemies[5] = enemy6;

  d1.x = 20;
  d1.y = 22;

  d2.x = 28;
  d2.y = 2;

  d3.x = 14;
  d3.y = 10;

  d4.x = 13;
  d4.y = 4;

  d5.x = 4;
  d5.y = 11;

  doors[0] = d1;
  doors[1] = d2;
  doors[2] = d3;
  doors[3] = d4;
  doors[4] = d5;

  seg7Write(0x00);
  columnWrite(0xFF);
  rowWrite(0x00);
}

void light_up(unsigned char r, unsigned char c)
{
  unsigned char x, y;
  x = 1 << (c - 1);
  y = 1 << (r - 1);
  columnWrite (0xFF);
  rowWrite (0x00);
  columnWrite(~x);
  rowWrite(y);
}

void show_map()
{
  for(signed char  j = 0; j < 8; j++)
    for(signed char  i = 0; i < 8; i++)
    {
      if(user.pos_x - 4 + i < 0 || user.pos_y - 3 + j < 0 || user.pos_x - 4 + i > MAP_SIZE - 1 || user.pos_y - 3 + j > MAP_SIZE - 1)
        my_view[i][j] = 1;
      else if(map[user.pos_x - 4 + i][user.pos_y - 3 + j] == 1)
        my_view[i][j] = 1;
      else
        my_view[i][j] = 0;
      for(unsigned char k = enemy_index; k < 6; k++)
        if(((user.pos_x - 4 + i) == enemies[k].pos_x) && ((user.pos_y - 3 + j) == enemies[k].pos_y))
          my_view[i][j] = 1;
      for(unsigned char k = 0; k < 5; k++)
      {
        if(k < enemy_index) map[doors[k].x][doors[k].y] = 0;
        else
        {
          if(((user.pos_x - 4 + i) == doors[k].x) && ((user.pos_y - 3 + j) == doors[k].y))
            my_view[i][j] = 1;
        }
      }
    }
    for(unsigned char  j = 0; j < 8; j++)
      for(unsigned char  i = 0; i < 8; i++)
        if(my_view[i][j] > 0)
          light_up (i+1,j+1);
  light_up(5, 4);
}

void move_hero()
{
  static unsigned char temp;
  temp = GetKeypadKey ();
  if(temp == '4' && user.pos_y > 0 && map[user.pos_x][user.pos_y-1] != 1 && map[user.pos_x][user.pos_y-1] != 3) user.pos_y--;
  else if(temp == '6' && user.pos_y < MAP_SIZE - 1 && map[user.pos_x][user.pos_y+1] != 1 && map[user.pos_x][user.pos_y+1] != 3) user.pos_y++;
  else if(temp == '2' && user.pos_x > 0 && map[user.pos_x-1][user.pos_y] != 1 && map[user.pos_x-1][user.pos_y] != 3) user.pos_x--;
  else if(temp == '5' && user.pos_x < MAP_SIZE - 1 && map[user.pos_x+1][user.pos_y] != 1 && map[user.pos_x+1][user.pos_y] != 3) user.pos_x++;
}

void draw_life_bars()
{
  for(int i = 0; i < enemy->life; i++)
    light_up(1,8-i);
  for(int i = 0; i < user.life; i++)
    light_up(8,i+1);
}

void draw_hero()
{
  light_up(4, 2);
  light_up(5, 2);
  light_up(4, 1);
  light_up(4, 3);
  light_up(6, 3);
  light_up(6, 1);
  light_up(3, 2);
  columnWrite (0xFF);
  rowWrite (0x00);
}

void draw_enemy()
{
  if(enemy_index == 5)
  {
    light_up(3, 5);
    light_up(3, 6);
    light_up(4, 6);
    light_up(5, 6);
    light_up(5, 7);
    light_up(5, 8);
    light_up(6, 6);
    light_up(6, 7);
    light_up(6, 8);
  }
  else
  {
    light_up(3, 7);
    light_up(4, 7);
    light_up(5, 7);
    light_up(4, 6);
    light_up(4, 8);
    light_up(6, 8);
    light_up(6, 6);
  }
}

void show_battle()
{
  draw_life_bars();
  draw_enemy();
  draw_hero();
}

void enemys_turn()
{
  if(enemy_defend_used)
  {
    enemy->defense -= 2;
    enemy_defend_used--;
  }
  if(rand() % 2 == 0)
  {
    if(enemy->attack >= user.defense) user.life -= (enemy->attack - user.defense);
    if(user.life < 0) user.life = 0;
  }
  else
  {
    enemy->defense += 2;
    enemy_defend_used = 1;
  }
}

void LCD_WriteCmdStart(unsigned char cmd) {
   *LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_RS, 0);
   *LCD_Data = cmd;
   *LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 1);
}
void LCD_WriteCmdEnd() {
   *LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 0);
}
void LCD_WriteDataStart(unsigned char Data) {
   *LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_RS,1);
   *LCD_Data = Data;
   *LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 1);
}
void LCD_WriteDataEnd() {
   *LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 0);
}
void LCD_Cursor(unsigned char column ) {
   if ( column < 16) LCD_WriteCmdStart(0x80+column);
   else if(column >= 32) LCD_WriteCmdStart(0x80+column-32);
   else LCD_WriteCmdStart(0xB0+column); // IEEE change this value to 0xBF+column
}

enum LI_States { LI_Init1, LI_Init2, LI_Init3, LI_Init4, LI_Init5, LI_Init6,
   LI_WaitDisplayString, LI_Clr, LI_PositionCursor, LI_DisplayChar, LI_WaitGo0 } LI_State;

void LI_Tick() {
   static unsigned char i;
   switch(LI_State) { // Transitions
       case -1:
           LI_State = LI_Init1;
           break;
       case LI_Init1:
           LI_State = LI_Init2;
           i=0;
           break;
       case LI_Init2:
           if (i<10)  // Wait 100 ms after power up
               LI_State = LI_Init2;
           else
               LI_State = LI_Init3;
           break;
       case LI_Init3:
           LI_State = LI_Init4;
           LCD_WriteCmdEnd();
           break;
       case LI_Init4:
           LI_State = LI_Init5;
           LCD_WriteCmdEnd();
           break;
       case LI_Init5:
           LI_State = LI_Init6;
           LCD_WriteCmdEnd();
           break;
       case LI_Init6:
           LI_State = LI_WaitDisplayString;
           LCD_WriteCmdEnd();
           break;
       //////////////////////////////////////////////
       case LI_WaitDisplayString:
           if (!LCD_go_g) {
               LI_State = LI_WaitDisplayString;
           }
           else if (LCD_go_g) {
            LCD_rdy_g = 0;
               LI_State = LI_Clr;
           }
           break;
       case LI_Clr:
           LI_State = LI_PositionCursor;
           LCD_WriteCmdEnd();
           i=0;
           break;
       case LI_PositionCursor:
           LI_State = LI_DisplayChar;
           LCD_WriteCmdEnd();
           break;
       case LI_DisplayChar:
           if (i<31) {
               LI_State = LI_PositionCursor;
               LCD_WriteDataEnd();
           i++;
           }
           else {
               LI_State = LI_WaitGo0;
               LCD_WriteDataEnd();
           }
           break;
       case LI_WaitGo0:
           if (!LCD_go_g) {
               LI_State = LI_WaitDisplayString;
           }
           else if (LCD_go_g) {
               LI_State = LI_WaitGo0;
           }
           break;
       default:
           LI_State = LI_Init1;
       } // Transitions

   switch(LI_State) { // State actions
       case LI_Init1:
        LCD_rdy_g = 0;
           break;
       case LI_Init2:
           i++; // Waiting after power up
           break;
       case LI_Init3:
           LCD_WriteCmdStart(0x38);
           break;
       case LI_Init4:
           LCD_WriteCmdStart(0x06);
           break;
       case LI_Init5:
           LCD_WriteCmdStart(0x0F);
           break;
       case LI_Init6:
           LCD_WriteCmdStart(0x01); // Clear
           break;
       //////////////////////////////////////////////
       case LI_WaitDisplayString:
           LCD_rdy_g = 1;
           break;
       case LI_Clr:
           LCD_WriteCmdStart(0x01);
           break;
       case LI_PositionCursor:
           LCD_Cursor(i);
           break;
       case LI_DisplayChar:
           LCD_WriteDataStart(LCD_string_g[i]);
           break;
       case LI_WaitGo0:
           break;
       default:
           break;
   } // State actions
}
//--------END LCD interface synchSM------------------------------------------------


// SynchSM for testing the LCD interface -- waits for button press, fills LCD with repeated random num

enum LT_States { LT_s0, LT_WaitLcdRdy, LT_Write1, LT_WaitForBattle,
                LT_WriteAttack, LT_WaitAttack, LT_AttackRelease,
                LT_WriteDefend, LT_WaitDefend, LT_DefendRelease,
                LT_WriteRun, LT_WaitRun, LT_RunRelease, pre_attack,
                LT_WriteFinal, LT_WaitFinal } LT_State;

void LT_Tick() {
 static unsigned short j;
 static unsigned char i, x, c;
 static unsigned char temp;
 temp = GetKeypadKey();
 switch(LT_State) { // Transitions
   case -1:
     LT_State = LT_s0;
     break;
   case LT_s0:
     LT_State = LT_WaitLcdRdy;
     break;
   case LT_WaitLcdRdy:
     if (!LCD_rdy_g) LT_State = LT_WaitLcdRdy;
     else if (LCD_rdy_g) LT_State = LT_Write1;
     break;
   case LT_Write1:
     LT_State = LT_WaitForBattle;
     break;
   case LT_WaitForBattle:
     LCD_go_g = 0;
     break;
   case pre_attack:
     LT_State = LT_WriteAttack;
     break;
   case LT_WriteAttack:
     LT_State = LT_WaitAttack;
     break;
   case LT_WaitAttack:
     LCD_go_g = 0;
     if (temp == '5') LT_State = LT_AttackRelease;
     break;
   case LT_AttackRelease:
     if (temp != '5') LT_State = LT_WriteDefend;
     break;
   case LT_WriteDefend:
     LT_State = LT_WaitDefend;
     break;
   case LT_WaitDefend:
     LCD_go_g = 0;
     if (temp == '5') LT_State = LT_DefendRelease;
     break;
   case LT_DefendRelease:
     if (temp != '5') LT_State = LT_WriteRun;
     break;
   case LT_WriteRun:
     LT_State = LT_WaitRun;
     break;
   case LT_WaitRun:
     LCD_go_g = 0;
     if (temp == '5') LT_State = LT_RunRelease;
     break;
   case LT_RunRelease:
     if (temp != '5') LT_State = LT_WriteAttack;
     break;
   case LT_WriteFinal:
     if(final_battle_flag)
     {
        LCD_go_g = 0;
        LT_State = LT_WaitForBattle;
     }
     break;
   default:
     LT_State = LT_s0;
 } // Transitions

 switch(LT_State) { // State actions
   case LT_s0:
     LCD_go_g=0;
     strcpy(LCD_string_g, " "); // Init, but never seen, shows use of strcpy though
     break;
   case LT_Write1:
     strcpy(LCD_string_g, "   Dungeon              Duels   ");
     LCD_go_g = 1; // Display string
     break;
   case LT_WriteAttack:
     show_flags();
     strcpy(LCD_string_g, "What to do?     ->Attack       _");
     LCD_go_g = 1; // Display string
     break;
   case LT_WriteDefend:
     strcpy(LCD_string_g, "What to do?     ->Defend       _");
     LCD_go_g = 1; // Display string
     break;
   case LT_WriteRun:
     strcpy(LCD_string_g, "What to do?     ->Run          _");
     LCD_go_g = 1; // Display string
     break;
   case LT_WriteFinal:
     strcpy(LCD_string_g, "FIRE BREATHING  BRISK!!!        ");
     LCD_go_g = 1; // Display string
     final_battle_flag = 1;
     break;
   default:
     break;
 } // State actions
}

enum matrix_states{matrix_wait, matrix_clear, matrix_show, matrix_battle} m_state;
void matrix()
{
  static unsigned char flag = 0;
  switch(m_state)
  {
    case -1:
      m_state = matrix_wait;
      break;
    case matrix_clear:
      m_state = matrix_show;
      break;
    case matrix_show:
      m_state = matrix_clear;
      break;
    default:
      break;
  }
  switch(m_state)
  {
    case matrix_clear:
      rowWrite (0x00);
      columnWrite (0xFF);
      break;
    case matrix_show:
      show_flags();
      show_map();
      break;
    case matrix_battle:
      enemy = &enemies[enemy_index];
      show_battle();
      break;
    default:
      break;
  }
}

enum game_states{init, load_screen, move, move_wait, battle_wait,update_battle, enemy_turn,
                my_turn, att, def, run, exit_battle, win, lose, my_turn_release, my_turn_release_done, final_battle} g_state;

void game()
{
  static unsigned char temp;
  temp = GetKeypadKey();

  switch(g_state)
  {
     case -1:
        g_state = init;
        break;
     case init:
        if(GetKeypadKey() == 'A') g_state = load_screen;
        break;
     case load_screen:
        if(user.pos_x == enemy->pos_x && user.pos_y == enemy->pos_y && enemy_index == 5) g_state = final_battle;
        else if(user.pos_x == enemy->pos_x && user.pos_y == enemy->pos_y) g_state = battle_wait;
        else if(temp == '2' || temp == '4' || temp == '5' || temp == '6') g_state = move;
        break;
     case final_battle:
        if(counter >= 30)
        {
           g_state = battle_wait;
           counter = 0;
        }
        break;
     case move:
        g_state = move_wait;
        break;
     case move_wait:
        if(temp != '2' && temp != '4' && temp != '5' && temp != '6') g_state = load_screen;
        break;
     case battle_wait:
        if(user.life == 0) g_state = lose;
        else if(enemy->life == 0) g_state = win;
        else if(is_my_turn == 1) g_state = my_turn;
        else if(is_my_turn == 2) g_state = enemy_turn;
        else if(user.speed > enemy->speed) g_state = my_turn;
        else if(user.speed < enemy->speed) g_state = enemy_turn;
        else if(user.speed == enemy->speed) g_state = (rand() % 2) ? my_turn : enemy_turn;
        break;
     case my_turn:
        if(temp == '5') g_state = my_turn_release;
        if(is_my_turn==2 && LT_State == LT_WaitAttack) g_state = att;
        else if(is_my_turn==2 && LT_State == LT_WaitDefend) g_state = def;
        else if(is_my_turn==2 && LT_State == LT_WaitRun && user.speed >= enemy->speed) g_state = run;
        break;
     case my_turn_release:
        if(temp != '5') g_state = my_turn_release_done;
        break;
     case att:
        g_state = battle_wait;
        break;
     case def:
        g_state = battle_wait;
        break;
     case run:
        g_state = exit_battle;
        break;
     case exit_battle:
        LT_State = LT_WaitLcdRdy;
        g_state = load_screen;
        break;
     case enemy_turn:
        g_state = battle_wait;
        break;
     case win:
        g_state = exit_battle;
        break;
     case lose:
        g_state = exit_battle;
        break;
     default:
        break;
  }
  switch(g_state)
  {
     case load_screen:
        m_state = matrix_show;
        break;
     case move:
        move_hero();
        break;
     case final_battle:
       LT_State = LT_WriteFinal;
       counter++;
       break;
     case battle_wait:
        LT_State = pre_attack;
        m_state = matrix_battle;
        break;
     case my_turn:
        is_my_turn = 1;
        if(my_defend_used)
        {
           user.defense -= 2;
           my_defend_used--;
        }
        else if(temp == 'A')
        {
           is_my_turn = 2;
           while(temp =='A') temp = GetKeypadKey();
        }
        break;
     case my_turn_release_done:
        g_state = my_turn;
        break;
     case att:
        if(user.attack >= enemy->defense) enemy->life -= (user.attack - enemy->defense);
        if(enemy->life < 0) enemy->life = 0;
        break;
     case def:
        user.defense += 2;
        my_defend_used = 1;
        break;
     case run:
        user.pos_x++;
        break;
     case exit_battle:
        break;
     case enemy_turn:
        is_my_turn = 2;
        enemys_turn();
        is_my_turn--;
        break;
     case win:
        enemy_index++;
        user.attack++;
        user.defense++;
        user.speed++;
        user.life = 6;
        break;
     case lose:
        user.pos_x++;
        enemy->life = 6;
        user.life = 6;
        break;
     default:
        break;
  }
}

enum reset_states{on, off} r_state;
void reset()
{
  switch(r_state)
  {
     case -1:
        r_state = on;
        break;
     case on:
        r_state = off;
        break;
     case off:
        if(GetBit(PINC,6) == 0)
        {
           while(GetBit(PINC,6) == 0);
           seg7Write(0x00);
           rowWrite (0x00);
           columnWrite (0xFF);
           m_state = matrix_wait;
           r_state = on;
        }
        break;
     default:
        break;
  }
  switch(r_state)
  {
     case on:
        initialize_game();
        g_state = init;
        LT_State = -1;
        LI_State = -1;
        break;
     default:
        break;
  }
}

// End Overhead

//--------Task scheduler data structure---------------------------------------
// Struct for Tasks represent a running process in our simple real-time operating system.
typedef struct _task {
   /*Tasks should have members that include: state, period,
       a measurement of elapsed time, and a function pointer.*/
   signed char state; //Task's current state
   unsigned long int period; //Task period
   unsigned long int elapsedTime; //Time elapsed since last task tick
   int (*TickFct)(int); //Task tick function
} task;

//--------End Task scheduler data structure-----------------------------------

//--------User defined FSMs---------------------------------------------------
//Enumeration of states.


// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
// Set Data Direction Registers
// Buttons PORTA[0-7], set AVR PORTA to pull down logic
   DDRB = 0xFF; PORTB = 0x00; // Set port B to output
   DDRD = 0xFF; PORTD = 0x00; // Set port D to output
   DDRA = 0xF0; PORTA = 0x0F;
   DDRC = 0x3F; PORTC = 0xC0;
// . . . etc

// Period for the tasks
unsigned long int SMTick1_calc = 10;
unsigned long int SMTick2_calc = 50;
unsigned long int SMTick3_calc = 50;
unsigned long int SMTick4_calc = 50;
unsigned long int SMTick5_calc = 3;

//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
tmpGCD = findGCD(tmpGCD, SMTick3_calc);
tmpGCD = findGCD(tmpGCD, SMTick4_calc);
tmpGCD = findGCD(tmpGCD, SMTick5_calc);

//Greatest common divisor for all tasks or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;

//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;
unsigned long int SMTick2_period = SMTick2_calc/GCD;
unsigned long int SMTick3_period = SMTick3_calc/GCD;
unsigned long int SMTick4_period = SMTick4_calc/GCD;
unsigned long int SMTick5_period = SMTick5_calc/GCD;

//Declare an array of tasks
static task task1, task2, task3, task4, task5;
task *tasks[] = { &task1, &task2, &task3, &task4, &task5};
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

// Task 1
task1.state = -1;//Task initial state.
task1.period = SMTick1_period;//Task Period.
task1.elapsedTime = SMTick1_period;//Task current elapsed time.
task1.TickFct = &LI_Tick;//Function pointer for the tick.

// Task 2
task2.state = -1;//Task initial state.
task2.period = SMTick2_period;//Task Period.
task2.elapsedTime = SMTick2_period;//Task current elapsed time.
task2.TickFct = &LT_Tick;//Function pointer for the tick.

// Task 3
task3.state = -1;//Task initial state.
task3.period = SMTick3_period;//Task Period.
task3.elapsedTime = SMTick3_period;//Task current elapsed time.
task3.TickFct = &reset;//Function pointer for the tick.

// Task 4
task4.state = -1;//Task initial state.
task4.period = SMTick4_period;//Task Period.
task4.elapsedTime = SMTick4_period;//Task current elapsed time.
task4.TickFct = &game;//Function pointer for the tick.

// Task 5
task5.state = -1;//Task initial state.
task5.period = SMTick5_period;//Task Period.
task5.elapsedTime = SMTick5_period;//Task current elapsed time.
task5.TickFct = &matrix;//Function pointer for the tick.

initialize_game ();

// Set the timer and turn it on
TimerSet(GCD);
TimerOn();

unsigned short i; // Scheduler for-loop iterator
while(1) {
   // Scheduler code
   for ( i = 0; i < numTasks; i++ ) {
       // Task is ready to tick
       if ( tasks[i]->elapsedTime == tasks[i]->period ) {
           // Setting next state for task
           tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
           // Reset the elapsed time for next tick.
           tasks[i]->elapsedTime = 0;
       }
       tasks[i]->elapsedTime += 1;
   }
   while(!TimerFlag);
   TimerFlag = 0;
}

// Error: Program should not exit!
return 0;
}

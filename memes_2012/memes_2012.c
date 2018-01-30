/*
 * memes_2012.c : 2012�N�xMEMEs�������K
 *
 */

#include <stdlib.h>
#include "iodefine.h"
#include "typedefine.h"

#define	printf	((int (*)(const char *,...))0x00007c7c)

#define	SW6 	(PD.DR.BIT.B18)
#define	SW5 	(PD.DR.BIT.B17)
#define	SW4 	(PD.DR.BIT.B16)

#define	LED6	(PE.DR.BIT.B11)
#define	LED_ON	(0)
#define	LED_OFF	(1)

#define	DIG1	(PE.DR.BIT.B3)
#define	DIG2	(PE.DR.BIT.B2)
#define	DIG3	(PE.DR.BIT.B1)

#define	SPK	(PE.DR.BIT.B0)

#define LCD_RS		(PA.DR.BIT.B22)
#define LCD_E		(PA.DR.BIT.B23)
#define LCD_RW		(PD.DR.BIT.B23)
#define LCD_DATA	(PD.DR.BYTE.HH)

#define	NMROF_ROCKS	6

struct position {
  int x;
  int y;
  int active;
};

void wait_us(_UINT);
void LCD_inst(_SBYTE);
void LCD_data(_SBYTE);
void LCD_cursor(_UINT, _UINT);
void LCD_putch(_SBYTE);
void LCD_putstr(_SBYTE *);
void LCD_cls(void);
void LCD_init(void);


// --------------------
// -- �g�p����֐��Q --
// --------------------
void wait_us(_UINT us) {
	_UINT val;

	val = us * 10 / 16;
	if (val >= 0xffff)
		val = 0xffff;

	CMT0.CMCOR = val;
	CMT0.CMCSR.BIT.CMF &= 0;
	CMT.CMSTR.BIT.STR0 = 1;
	while (!CMT0.CMCSR.BIT.CMF);
	CMT0.CMCSR.BIT.CMF = 0;
	CMT.CMSTR.BIT.STR0 = 0;
}

void LCD_inst(_SBYTE inst) {
	LCD_E = 0;
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_E = 1;
	LCD_DATA = inst;
	wait_us(1);
	LCD_E = 0;
	wait_us(40);
}

void LCD_data(_SBYTE data) {
	LCD_E = 0;
	LCD_RS = 1;
	LCD_RW = 0;
	LCD_E = 1;
	LCD_DATA = data;
	wait_us(1);
	LCD_E = 0;
	wait_us(40);
}

void LCD_cursor(_UINT x, _UINT y) {
	if (x > 15)
		x = 15;
	if (y > 1)
		y = 1;
	LCD_inst(0x80 | x | y << 6);
}

void LCD_putch(_SBYTE ch) {
	LCD_data(ch);
}

void LCD_putstr(_SBYTE *str) {
	_SBYTE ch;

	while (ch = *str++)
		LCD_putch(ch);
}

void LCD_cls(void) {
	LCD_inst(0x01);
	wait_us(1640);
}

void LCD_init(void) {
	wait_us(45000);
	LCD_inst(0x30);
	wait_us(4100);
	LCD_inst(0x30);
	wait_us(100);
	LCD_inst(0x30);
	
	LCD_inst(0x38);
	LCD_inst(0x08);
	LCD_inst(0x01);
	wait_us(1640);
	LCD_inst(0x06);
	LCD_inst(0x0c);
}

// --------------------------------------------
// -- �Q�[���p�̊֐��Q --

// -- �������ړ� --
void move_me(struct position *me)
{
	struct position old_position;

	old_position.x = me->x;
	old_position.y = me->y;

	if (AD0.ADDR0 < 0x4000) {
		// -- �W���C�X�e�B�b�N�� --
		me->y = 0;
	} else if (AD0.ADDR0 > 0xc000) {
		// -- �W���C�X�e�B�b�N�� --
		me->y = 1;
	}
	
	if (AD0.ADDR1 < 0x4000) {
		 //-- �W���C�X�e�B�b�N�E --
		if(me->x >= 15){
			me->x = 15;
		} else {
			me->x += 1;
		}
	} else if (AD0.ADDR1 > 0xc000) {
		 //-- �W���C�X�e�B�b�N�� --
		if(me->x <= 0){
			me->x = 0;
		} else {
			me->x -= 1;
		}
	}

	if (old_position.y != me->y || old_position.x != me->x) {
		// -- �ړ������Ƃ� .. �Â��\�������� --
		LCD_cursor(old_position.x, old_position.y);
		LCD_putch(' ');
	}
	LCD_cursor(me->x, me->y);
	LCD_putch('>');
}


// -- ����ړ� --
void move_rock(struct position rock[])
{
	int i;

	for (i = 0; i < NMROF_ROCKS; i++) {
		if (rock[i].active) {
			// ��ʏ�Ɋ₪���݂���
			LCD_cursor(rock[i].x, rock[i].y);
			LCD_putch(' ');
			if (rock[i].x == 0) {
				// ����
				rock[i].active = 0;
			} else {
				rock[i].x--;
				LCD_cursor(rock[i].x, rock[i].y);
				LCD_putch('*');
			}
		}
	}
}


// -- �V���������� --
void new_rock(struct position rock[])
{
	int i;

	for (i = 0; i < NMROF_ROCKS; i++) {
		if (rock[i].active == 0) {
			// -- �V������ --
			rock[i].active = 1;
			rock[i].x = 15;
			rock[i].y = rand() % 2;
			LCD_cursor(rock[i].x, rock[i].y);
			LCD_putch('*');
			break;
		}
	}
}


// --------------------------------------------
// -- ���C���֐� --
void main()
{
	struct position me;					// �����̎Ԃ̍��W
	struct position rock[NMROF_ROCKS];	// ��̍��W
	int move_timing, new_timing;
	int ad, i;

	STB.CR4.BIT._AD0 = 0;
	STB.CR4.BIT._CMT = 0;
	STB.CR4.BIT._MTU2 = 0;

	CMT0.CMCSR.BIT.CKS = 1;

	// MTU2 ch0
	MTU20.TCR.BIT.TPSC = 3;			// 1/64�I��
	MTU20.TCR.BIT.CCLR = 1;			// TGRA�̃R���y�A�}�b�`�ŃN���A
	MTU20.TGRA = 31250 - 1;			// 100ms
	MTU20.TIER.BIT.TTGE = 1;		// A/D�ϊ��J�n�v��������

	// AD0
	AD0.ADCSR.BIT.ADM = 3;			// 2�`�����l���X�L�������[�h
	AD0.ADCSR.BIT.CH = 1;			// AN0
	AD0.ADCSR.BIT.TRGE = 1;			// MTU2����̃g���K�L��
	AD0.ADTSR.BIT.TRG0S = 1;		// TGRA�R���y�A�}�b�`�Ńg���K

	// MTU2 ch1
	MTU21.TCR.BIT.TPSC = 3;			// 1/64�I��
	MTU21.TCR.BIT.CCLR = 1;			// TGRA�̃R���y�A�}�b�`�ŃN���A
	MTU21.TGRA = 31250 - 1;			// 100ms

	LCD_init();

	MTU2.TSTR.BIT.CST0 = 1;			// MTU2 CH0�X�^�[�g
	MTU2.TSTR.BIT.CST1 = 1;			// MTU2 CH1�X�^�[�g

	me.x = me.y = 0;
	for (i = 0; i < NMROF_ROCKS; i++)
		rock[i].active = 0;

	move_timing = new_timing = 0;
	while(1){
		//SW6�ŃQ�[���J�n
		if (SW6){
			while (1) {
				//SW5���������...
				if(SW5){
					while(1){
						//SW5�������ꂽ��
						if(!SW5){
							//�ꎞ��~��
							while(1){
								//������x�����ꂽ��ĊJ
								if(SW6){
									break;
								}
							}
						}
						if(SW6){
							break;
						}
					}
				}
				if (MTU21.TSR.BIT.TGFA) {
					// MTU2 ch1 �R���y�A�}�b�`����(100ms��)
					MTU21.TSR.BIT.TGFA = 0;	// �t���O�N���A

					move_me(&me);			// �����ړ�
					if (move_timing++ >= 2) {
						move_timing = 0;
						move_rock(rock);	// ����ړ�
						if (new_timing-- <= 0) {
							new_timing = rand() * 5 / (RAND_MAX + 1);
							new_rock(rock);		// �V�����₪�o��
						}
					}
				}
			}
		}
	}
}
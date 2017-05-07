#include "neslib.h"
#include "b-rush.h"

#define DIR_LEFT 1
#define DIR_RIGHT 2

const unsigned char metasprite[]={
	0,	8,	0x10,	0,
	0,	0,	0x00,	0,
	128
};

const unsigned char palette[16]={ 0x0f,0x02,0x16,0x30,0x0f,0x01,0x21,0x31,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };

static unsigned char update_list[32*3+1];

static unsigned char i, h;
static unsigned char pad,spr;
static unsigned char frame;

static unsigned char pen_x[2];
static unsigned char pen_y[2];
static unsigned char pen_v[2];
static unsigned char dir_sprite[2];
static unsigned char state_sprite[2];

// player
void player_sprite_0(const int j)
{
	pad=pad_poll(j);
	if(pad&PAD_LEFT ) 
	{
		dir_sprite[j] = DIR_LEFT;
		state_sprite[j] = 1;
	}
	if(pad&PAD_RIGHT)
	{
		dir_sprite[j] = DIR_RIGHT;
		state_sprite[j] = 1;
	}
	if(pad&PAD_A)
	{
		state_sprite[j] = 2;
	}
	
}

void player_sprite_1(const int j)
{
	pad=pad_poll(j);
	state_sprite[j] = 0;
	if(pad&PAD_LEFT ) 
	{
		dir_sprite[j] = DIR_LEFT;
		state_sprite[j] = 1;
	}
	if(pad&PAD_RIGHT)
	{
		dir_sprite[j] = DIR_RIGHT;
		state_sprite[j] = 1;
	}
	if(pad&PAD_A)
	{
		state_sprite[j] = 2;
	}
}

void player_sprite_2(const int j)
{
	pen_v[j] = 10;
}

void player_sprite_3(const int j)
{
	state_sprite[j] = 0;
}

void player_sprite_4(const int j)
{
	pad=pad_poll(j);
	if(pad&PAD_LEFT ) 
	{
		dir_sprite[j] = DIR_LEFT;
	}
	if(pad&PAD_RIGHT)
	{
		dir_sprite[j] = DIR_RIGHT;
	}
}

void player_machine(const int k)
{
/*
	0 - Repos
	1 - Marche
	2 - Saut
	3 - Reception 
	4 - en l'air
*/

	if ( state_sprite[k] == 0) player_sprite_0(k);
	if ( state_sprite[k] == 1) player_sprite_1(k);
	if ( state_sprite[k] == 2) player_sprite_2(k);
	if ( state_sprite[k] == 3) player_sprite_3(k);
	if ( state_sprite[k] == 4) player_sprite_4(k);
	
}

unsigned char get_metatile(unsigned int x,unsigned int y)
{
	static unsigned char v_x,v_y;
	v_x = x/8;
	v_y = y/8;
	return nametable[v_y*32+v_x];//level_y/16 is pixels to row, then *16 is row to offset, then x/16 is pixels to column
}

void physique(const int k)
{
	static unsigned char tile, bap;
	static unsigned char t_x,t_y;
	
	if(dir_sprite[k] == DIR_LEFT  && state_sprite[k] == 1 && pen_x[k]>  0) pen_x[k]-=2;
	if(dir_sprite[k] == DIR_RIGHT && state_sprite[k] == 1 && pen_x[k]<248) pen_x[k]+=2;
	if(dir_sprite[k] == DIR_LEFT  && state_sprite[k] == 4 && pen_x[k]>  0) pen_x[k]-=2;
	if(dir_sprite[k] == DIR_RIGHT && state_sprite[k] == 4 && pen_x[k]<248) pen_x[k]+=2;
	tile = get_metatile(pen_x[k]+4,pen_y[k]+16);
	if(tile != 0x06 && state_sprite[k] == 4) 
	{
		state_sprite[k] = 0;
		pen_v[k]=0;
	}
	if(tile == 0x06 && state_sprite[k] == 1) 
	{
		state_sprite[k] = 4;
	}
	if(state_sprite[k] == 2) 
	{
		state_sprite[k] = 4;
	}
	if(tile ==0x03)
	{	
		t_x = (pen_x[k]+4)/8;
		t_y = (pen_y[k]+16)/8;
		update_list[3*k] = MSB(NTADR_A(t_x,t_y));
		update_list[3*k+1] = LSB(NTADR_A(t_x,t_y));
		update_list[3*k+2] = tile+k+1;
	}
	if(state_sprite[k] == 4)
	{
		pen_y[k] = pen_y[k] - pen_v[k];
		bap = pen_v[k]+7;
		if (bap>0) pen_v[k]-=1; 
	}

	update_list[3*2] = MSB(NTADR_A(1,1));
	update_list[3*2+1] = LSB(NTADR_A(1,1));
	update_list[3*2+2] = tile;
	update_list[9]=NT_UPD_EOF;
	set_vram_update(update_list);
}

void main(void)
{	
	pal_spr(palette);
	
	pal_bg(palette);//set background palette from an array

	update_list[96]=NT_UPD_EOF;

	//now the main loop
	
	for(i=0;i<30;++i)
	{
		for(h=0;h<32;++h)
		{
			update_list[h*3] = MSB(NTADR_A(h,i));
			update_list[h*3+1] = LSB(NTADR_A(h,i));
			update_list[h*3+2] = nametable[i*32+h];
		}

		flush_vram_update(update_list);
		//set_vram_update(update_list);
	}
	
	
	ppu_on_all();//enable rendering

	//set initial coords
	
	pen_x[0]=52;
	pen_y[0]=104;
	pen_v[0]=0;
	pen_x[1]=196;
	pen_y[1]=104;
	pen_v[1]=0;
	
	frame=0;//frame counter

	//init other vars


	while(1)
	{
		ppu_wait_frame();//wait for next TV frame


		//process players
		
		spr=0;

		for(i=0;i<2;++i)
		{
			//display metasprite
			
			spr=oam_meta_spr(pen_x[i],pen_y[i],spr,metasprite);

			player_machine(i);
			physique(i);
			//poll pad and change coordinates
			
		}

		frame++;
	}
}

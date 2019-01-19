
#include <bios.h>
#include <dos.h>
#include <memory.h>
#include <math.h>
#include <time.h>

#include <keyb.h>

int keys_active = 0;
int keyboard_state[128];
int keyboard_processed[128];

void Sleep_Key()
{
	int hit = 0;
	int current_keys_active = keys_active;
	while(hit == 0)
	{
		if(keys_active <= current_keys_active)
		{
			current_keys_active = keys_active;
		}
		else
		{
			hit = 1;
		}
	}
}

void Delay_Key(int msecs)
{
	int hit = 0;
	int current_keys_active = keys_active;

	clock_t time = clock();
	int time_elapsed = 0;

	while(hit == 0 && time_elapsed < msecs*CLOCKS_PER_SEC/1000)
	{
		time_elapsed = clock()-time;
			
		if(keys_active <= current_keys_active)
		{
			current_keys_active = keys_active;
		}
		else
		{
			hit = 1;
		}
	}
}

int Get_Any_Key()
{
	return keys_active;
}

int Get_Key(int make_code)
{
	return keyboard_state[make_code];
}

int Get_Key_Once(int make_code)
{
	if(keyboard_state[make_code] == KEY_DOWN && keyboard_processed[make_code] == 0)
	{
		keyboard_processed[make_code] = 1;
		return 1;
	}
	else
	{
		return 0;
	}
}

void (interrupt far *Old_Keyboard_ISR)();

void interrupt far Keyboard_Driver()
{
	//New keyboard driver, continually updates the keyboard_state tbale with the state of the keys.
	int raw_scan_code = 0;

	_asm	{
			sti 					// re enables interrupts
			in al, KEY_BUFFER		// get the key that was pressed
			xor ah,ah 				// ah = 0 
			mov raw_scan_code, ax 	// store the key in variable
			in al, KEY_CONTROL		// set the control register to refelct read key
			or al, 82h				// set the proper bits to rst keyboard flipflop
			out KEY_CONTROL, al		// send new data to the control register
			and al, 7fh				// mask off high bit
			out KEY_CONTROL, al		// complete the reset
			mov al,	 20h			// puts the reset command in the al register to send
			out PIC_PORT, al		// tells PIC to reenable interrupts
			}

	if(raw_scan_code < 128)
	{
		if(keyboard_state[raw_scan_code] == KEY_UP)
		{
			keys_active++;
			keyboard_state[raw_scan_code] = KEY_DOWN;
		}
		
	}
	else 
	{
		raw_scan_code -= 128;
		if(keyboard_state[raw_scan_code] == KEY_DOWN)
		{
			keys_active--;	
			keyboard_state[raw_scan_code] = KEY_UP;
			keyboard_processed[raw_scan_code] = 0;
		}
	}
}

void Keyboard_Install_Driver()
{
	// Places the new keyboard driver on the interrupt vector table and saves the original for later restoration
	int i;

	for (i = 0; i<128; i++)
	{
		keyboard_state[i] = 0;
		keyboard_processed[i] = 0;
	}



	Old_Keyboard_ISR = _dos_getvect(KEYBOARD_INTERRUPT);

	_dos_setvect(KEYBOARD_INTERRUPT, Keyboard_Driver);
}

void Keyboard_Restore_Driver()
{
	//removes the new driver from the int vector table, restoring the previously saved DOS version
	_dos_setvect(KEYBOARD_INTERRUPT, Old_Keyboard_ISR);

}



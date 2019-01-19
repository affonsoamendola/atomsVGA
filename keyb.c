
#include <keyb.h>

int keyboard_state[128];
int keys_active = 0;

unsigned char Get_Key()
{
	//test if a key has been buffered by the system, if so, return ASCII of the key, otherwise, return 0

	if(bios_keybrd(KEYBRD_READY))
	{
		return((unsigned char)bios_keybrd(KEYBRD_READY));
	}
	else
	{
		return 0;
	}
}

unsigned char Get_Scan_Code()
{
	//Use BIOS funcions to retrieve the scan code of the last pressed key.
	//Returns 0 if no key pressed.

	_asm
	{
			mov ah, 01h			; function #1, KEY_READY
			int 16h				; call to the bios keyb interrupt
			jz buffer_empty 	; if no key, exit
			mov ah, 00h			; function #0
			int 16h				; call to the bios keyb interrupt
			mov al, ah			; result is placed in ah, copy it to al
			xor ah, ah			; ah = 0
			jmp done 			; jump to end to skyp buffer_empty:

		buffer_empty:
			xor ax, ax			; ax = 0

		done:					;doesnt really need to do anything, 8, or 16 bit data ir returned in AX. 
	}
}

unsigned int Get_Shift_State(unsigned int mask)
{
	// returns the shift state of the keyboard masked by the sent mask (logical AND)

	return (mask & bios_keybrd(KEYBRD_SHIFTSTATUS));

}

void (interrut far *Old_Keyboard_ISR)();

void interrupt far Keyboard_Driver()
{
	//New keyboard driver, continually updates the keyboard_state tbale with the state of the keys.
	int raw_scan_code = 0;

	_asm
		{
				sti 					;re enables interrupts
				in al, KEY_BUFFER		;get the key that was pressed
				xor ah,ah 				;ah = 0 
				mov raw_scan_code, ax 	;store the key in variable
				in al, KEY_CONTROL		;set the control register to refelct read key
				or al, 82h				;set the proper bits to rst keyboard flipflop
				out KEY_CONTROL, al		;send new data to the control register
				and al, 7fh				;mask off high bit
				out KEY_CONTROL, al		;complete the reset
				mov al,	 20h			;puts the reset command in the al register to send
				out PIC_PORT, al		;tells PIC to reenable interrupts
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
	}

	Old_Keyboard_ISR = dos_getvect(KEYBOARD_INTERRUPT);

	dos_setvect(KEYBOARD_INTERRUPT, Keyboard_Driver);
}

void Keyboard_Restore_Driver()
{
	//removes the new driver from the int vector table, restoring the previously saved DOS version
	dos_setvect(KEYBOARD_INTERRUPT, Old_Keyboard_ISR);

}



// implementation of video memory functions

#include "vidmem.h"

#include "../port_io.h"
#include "../util/algorithm.h"

#define _VIDMEM_ROW_CNT 25
#define _VIDMEM_COL_CNT 80

#define _VIDMEM_ADDRESS ((uint_8*) 0xb8000)

#define _VIDMEM_REGISTER_SCREEN_CTRL 0x3d4
#define _VIDMEM_REGISTER_SCREEN_DATA 0x3d5

typedef uint_16 _vm_pos;

// current cleared color
uint_8 _VIDMEM_CURRENT_CLEARED_COLOR = 0x0f;

// begin utility (used only in vidmem.c)

// get _vm_pos of row and col
_vm_pos _vidmem_position_of(uint_8 row, uint_8 col) {
	return ((_vm_pos) row * _VIDMEM_COL_CNT + (_vm_pos) col) << 1;
}

// get row of _vm_pos
uint_8 _vidmem_row_of(_vm_pos position) {
	return position / _VIDMEM_COL_CNT;
}

// get col of _vm_pos
uint_8 _vidmem_col_of(_vm_pos position) {
	return position % _VIDMEM_COL_CNT;
}

// get cursor position (_vm_pos format)
// does not correct for color bytes
_vm_pos _vidmem_get_cursor() {
	port_byte_out(_VIDMEM_REGISTER_SCREEN_CTRL, 14);
	_vm_pos result = port_byte_in(_VIDMEM_REGISTER_SCREEN_DATA) << 8;
	port_byte_out(_VIDMEM_REGISTER_SCREEN_CTRL, 15);
	result += port_byte_in(_VIDMEM_REGISTER_SCREEN_DATA);
	return result;
}

// scroll down a given number of lines
// does not update cursor position
void _vidmem_scroll_down(uint_64 count) {
	if (!count) {
		return;
	}
	
	for (uint_8 row = 0; row < _VIDMEM_ROW_CNT - 1; row++) {
		for (uint_8 col = 0; col < _VIDMEM_COL_CNT; col++) {
			_VIDMEM_ADDRESS[_vidmem_position_of(row, col)    ] =
			_VIDMEM_ADDRESS[_vidmem_position_of(row + 1, col)];
			_VIDMEM_ADDRESS[_vidmem_position_of(row, col)     | 1] =
			_VIDMEM_ADDRESS[_vidmem_position_of(row + 1, col) | 1];
		}
	}
	
	for (uint_8 col = 0; col < _VIDMEM_COL_CNT; col++) {
		_VIDMEM_ADDRESS[_vidmem_position_of(_VIDMEM_ROW_CNT - 1, col)    ] = 0x00;
		_VIDMEM_ADDRESS[_vidmem_position_of(_VIDMEM_ROW_CNT - 1, col) | 1] = _VIDMEM_CURRENT_CLEARED_COLOR;
	}
	
	if (--count) {
		_vidmem_scroll_down(count);
	}
}

// advance cursor by count
// handle end of line
// handle scrolling
// updates cursor position at the end
void _vidmem_advance_cursor(uint_64 count) {
	if (!count) {
		return;
	}
	
	uint_8 row = _vidmem_row_of(_vidmem_get_cursor());
	uint_8 col = _vidmem_col_of(_vidmem_get_cursor());
	
	col++;
	
	// end of line?
	if (col >= _VIDMEM_COL_CNT) {
		col = 0;
		row++;
	}
	
	// handle scrolling
	while (row >= _VIDMEM_ROW_CNT) {
		_vidmem_scroll_down(1);
		row--;
	}
	
	vidmem_set_cursor(row, col);
	
	if (--count) {
		_vidmem_advance_cursor(count);
	}
}

// put a number on the screen
const uint_8* _VIDMEM_DIGIT_TO_CHAR = "0123456789abcdef";
void _vidmem_put_int(int_64 value, int_8 base, uint_8 color, bool_t write_color) {
	bool_t is_unsigned = base < 0;
	if (is_unsigned) {
		base = -base;
	}
	
	bool_t is_neg = !is_unsigned && value < (int_64) 0;
	if (is_neg) {
		value = -value;
		write_color ? vidmem_putchar_color('-', color) : vidmem_putchar('-');
	}
	
	size_t digit_cnt = 0;
	uint_64 temp = (uint_64) value;
	uint_64 mult = 1;
	do {
		digit_cnt++;
		temp /= base;
		if (temp) {
			mult *= base;
		}
	} while (temp);
	
	for (size_t digit_index = 0; digit_index < digit_cnt; digit_index++) {
		temp = (uint_64) value;
		temp /= mult;
		mult /= base;
		temp %= base;
		write_color ?
		vidmem_putchar_color(_VIDMEM_DIGIT_TO_CHAR[temp], color) :
		vidmem_putchar      (_VIDMEM_DIGIT_TO_CHAR[temp]       );
	}
}

// end utility (used only in vidmem.c)

// set cursor position
// caps row at _VIDMEM_ROW_CNT - 1
// caps col at _VIDMEM_COL_CNT - 1
void vidmem_set_cursor(uint_8 row, uint_8 col) {
	row = min(row, _VIDMEM_ROW_CNT - 1);
	col = min(col, _VIDMEM_COL_CNT - 1);
	_vm_pos position = _vidmem_position_of(row, col) >> 1;
	port_byte_out(_VIDMEM_REGISTER_SCREEN_CTRL, 0x0f);
	port_byte_out(_VIDMEM_REGISTER_SCREEN_DATA, (uint_8) position);
	port_byte_out(_VIDMEM_REGISTER_SCREEN_CTRL, 0x0e);
	port_byte_out(_VIDMEM_REGISTER_SCREEN_DATA, (uint_8) (position >> 8));
}

// get cursor position row
uint_8 vidmem_cursor_row() {
	return _vidmem_row_of(_vidmem_get_cursor());
}

// get cursor position col
uint_8 vidmem_cursor_col() {
	return _vidmem_col_of(_vidmem_get_cursor());
}

// clear the screen
void vidmem_clear_screen() {
	for (uint_8 row = 0; row < _VIDMEM_ROW_CNT; row++) {
		for (uint_8 col = 0; col < _VIDMEM_COL_CNT; col++) {
			_VIDMEM_ADDRESS[_vidmem_position_of(row, col)    ] = 0x00;
			_VIDMEM_ADDRESS[_vidmem_position_of(row, col) | 1] = _VIDMEM_CURRENT_CLEARED_COLOR;
		}
	}
}

// clear screen to a specific color
void vidmem_clear_screen_color(uint_8 color) {
	_VIDMEM_CURRENT_CLEARED_COLOR = color;
	for (uint_8 row = 0; row < _VIDMEM_ROW_CNT; row++) {
		for (uint_8 col = 0; col < _VIDMEM_COL_CNT; col++) {
			_VIDMEM_ADDRESS[_vidmem_position_of(row, col)    ] =  0x00;
			_VIDMEM_ADDRESS[_vidmem_position_of(row, col) | 1] = _VIDMEM_CURRENT_CLEARED_COLOR;
		}
	}
}

// put a char then advance cursor
void vidmem_putchar(uint_8 data) {
	// new line
	if (data == '\n') {
		uint_8 col = 0;
		uint_8 row = vidmem_cursor_row() + 1;
		while (row >= _VIDMEM_ROW_CNT) {
			_vidmem_scroll_down(1);
			row--;
		}
		vidmem_set_cursor(row, col);
		return;
	}
	// tab
	if (data == '\t') {
		for (uint_8 i = 0; i < 4; i++) {
			vidmem_putchar(' ');
		}
		return;
	}
	
	_VIDMEM_ADDRESS[_vidmem_get_cursor() << 1] = data;
	
	_vidmem_advance_cursor(1);
}

// put a char with color then advance cursor
void vidmem_putchar_color(uint_8 data, uint_8 color) {
	// new line
	if (data == '\n') {
		uint_8 col = 0;
		uint_8 row = vidmem_cursor_row() + 1;
		while (row >= _VIDMEM_ROW_CNT) {
			_vidmem_scroll_down(1);
			row--;
		}
		vidmem_set_cursor(row, col);
		return;
	}
	// tab
	if (data == '\t') {
		for (uint_8 i = 0; i < 4; i++) {
			vidmem_putchar(' ');
		}
		return;
	}
	
	_VIDMEM_ADDRESS[(_vidmem_get_cursor() << 1)    ] =  data;
	_VIDMEM_ADDRESS[(_vidmem_get_cursor() << 1) | 1] = color;
	
	_vidmem_advance_cursor(1);
}

// put chars and advance cursor
void vidmem_puts(const uint_8* data) {
	for (size_t i = 0; data[i] != '\0'; i++) {
		vidmem_putchar(data[i]);
	}
}

// put chars with color and advance cursor
void vidmem_puts_color(const uint_8* data, uint_8 color) {
	for (size_t i = 0; data[i] != '\0'; i++) {
		vidmem_putchar_color(data[i], color);
	}
}

// put a number on the screen
void vidmem_put_int(int_64 value, int_8 base) {
	_vidmem_put_int(value, base, 0, false);
}

// put a number with color on the screen
void vidmem_put_int_color(int_64 value, int_8 base, uint_8 color) {
	_vidmem_put_int(value, base, color, true);
}

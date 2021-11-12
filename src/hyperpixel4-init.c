
#include <gpiod.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define CLK 27
#define MOSI 26
#define CS 18
#define DELAY 100 // clock pulse time in microseconds
#define WAIT 120000 // wait time in microseconds

struct gpiod_chip *chip;

struct gpiod_line *line_clk;
struct gpiod_line *line_mosi;
struct gpiod_line *line_cs;

const char *consumer = "HyperPixel Init";


int32_t commands[] = {
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x101, // Select Page 1
    0x008, 0x110, // Interface mode
    0x021, 0x10f, // Display control
    0x030, 0x102, // Resolution control
    0x031, 0x100, // Display inversion contrl
    0x040, 0x110, // Power control
    0x041, 0x155, // Power Control
    0x042, 0x102, // Power Control
    0x043, 0x184, // Power Control VGH_CLPEN VGH_CLP
    0x044, 0x184, // Power Control VGL_CLPEN VGL_CLP
    0x050, 0x178, // Power Control VREG1OU
    0x051, 0x178, // Power Control VREG2OUT
    0x052, 0x100, // Power Control VCOM
    0x053, 0x177, // Does not match sample code
    0x057, 0x160, // Added
    0x060, 0x107, // Source timing adjust
    0x061, 0x100,
    0x062, 0x108,
    0x063, 0x100,
    // Positive gamma cntrol
    0x0a0, 0x100, 0x0a1, 0x107, 0x0a2, 0x10c, 0x0a3, 0x10b,
    0x0a4, 0x103, 0x0a5, 0x107, 0x0a6, 0x106, 0x0a7, 0x104,
    0x0a8, 0x108, 0x0a9, 0x10c, 0x0aa, 0x113, 0x0ab, 0x106,
    0x0ac, 0x10d, 0x0ad, 0x119, 0x0ae, 0x110, 0x0af, 0x100,
    0x0c0, 0x100, 0x0c1, 0x107, 0x0c2, 0x10c, 0x0c3, 0x10b,
    0x0c4, 0x103, 0x0c5, 0x107, 0x0c6, 0x107, 0x0c7, 0x104,
    0x0c8, 0x108, 0x0c9, 0x10c, 0x0ca, 0x113, 0x0cb, 0x106,
    0x0cc, 0x10d, 0x0cd, 0x118, 0x0ce, 0x110, 0x0cf, 0x100,
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x106, 0x000, 0x120,
    0x001, 0x10a, 0x002, 0x100, 0x003, 0x100, 0x004, 0x101,
    0x005, 0x101, 0x006, 0x198, 0x007, 0x106, 0x008, 0x101,
    0x009, 0x180, 0x00a, 0x100, 0x00b, 0x100, 0x00c, 0x101,
    0x00d, 0x101, 0x00e, 0x100, 0x00f, 0x100, 0x010, 0x1f0,
    0x011, 0x1f4, 0x012, 0x101, 0x013, 0x100, 0x014, 0x100,
    0x015, 0x1c0, 0x016, 0x108, 0x017, 0x100, 0x018, 0x100,
    0x019, 0x100, 0x01a, 0x100, 0x01b, 0x100, 0x01c, 0x100,
    0x01d, 0x100, 0x020, 0x101, 0x021, 0x123, 0x022, 0x145,
    0x023, 0x167, 0x024, 0x101, 0x025, 0x123, 0x026, 0x145,
    0x027, 0x167, 0x030, 0x111, 0x031, 0x111, 0x032, 0x100,
    0x033, 0x1ee, 0x034, 0x1ff, 0x035, 0x1bb, 0x036, 0x1aa,
    0x037, 0x1dd, 0x038, 0x1cc, 0x039, 0x166, 0x03a, 0x177,
    0x03b, 0x122, 0x03c, 0x122, 0x03d, 0x122, 0x03e, 0x122,
    0x03f, 0x122, 0x040, 0x122,
    0x052, 0x110,
    0x053, 0x110,
    0x054, 0x113,
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x107, // Select page 7
    0x018, 0x11d, // enable VREG
    0x017, 0x122,
    0x002, 0x177,
    0x026, 0x1b2,
    0x0e1, 0x179,
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x100, // Select page 0
    0x03a, 0x160, // DPI 18BPP
    0x035, 0x100, // Tearing effect
    0x011, 0x100, // Turn off sleep mode
    -1, // Sleep
    0x029, 0x100, // Display on
    -1  // Sleep
};

int setup_pins(void)
{
    chip = gpiod_chip_open("/dev/gpiochip0");

    line_clk = gpiod_chip_get_line(chip, CLK);
    if(gpiod_line_request_output(line_clk, consumer, 0)) {
	printf("Unable to request output: BCM %d", CLK);
        return -1;
    }

    line_mosi = gpiod_chip_get_line(chip, MOSI);
    if(gpiod_line_request_output(line_mosi, consumer, 0)) {
	printf("Unable to request output: BCM %d", MOSI);
        return -1;
    }

    line_cs = gpiod_chip_get_line(chip, CS);
    if(gpiod_line_request_output(line_cs, consumer, 1)) {
	printf("Unable to request output: BCM %d", CS);
        return -1;
    }
}


void cleanup_pins(void)
{
    // Return the touch interrupt pin to a usable state
    gpiod_line_release(line_clk);
    gpiod_line_release(line_mosi);
    gpiod_line_release(line_cs);
    gpiod_chip_close(chip);
}

void send_bits(uint16_t data, uint16_t count){
    int x;
    int mask = 1 << (count-1);
    for(x = 0; x < count; x++){
        gpiod_line_set_value(line_mosi, (data & mask) > 0);
        data <<= 1;

	gpiod_line_set_value(line_clk, 0);
        usleep(DELAY);
	gpiod_line_set_value(line_clk, 1);
        usleep(DELAY);
    }
    gpiod_line_set_value(line_mosi, 0);
}

void write_9bit(uint16_t command){
    gpiod_line_set_value(line_cs, 0);
    send_bits(command, 9);
    gpiod_line_set_value(line_cs, 1);
}

void setup_lcd(void){
    int count = sizeof(commands) / sizeof(int32_t);
    int x;
    for(x = 0; x < count; x++){
        int32_t command = commands[x];
        if(command == -1){
            usleep(WAIT);
            continue;
        }
        write_9bit((uint16_t)command);
    }
}

int main(int argc, char **argv)
{
    setup_pins();
    setup_lcd();

    cleanup_pins();
    return 0;
}


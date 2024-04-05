#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Bit test macros */
#define IS_MODE_GPIO(reg, x)		!!((reg) & (1 << (x)))
#define IS_DIR_INPUT(reg, x)		!!((reg) & (1 << (x)))
#define IS_LVL_HIGH(reg, x)		!!((reg) & (1 << (x)))
#define IS_BLINKING(reg, x)		!!((reg) & (1 << (x)))
#define IS_INVERTED(reg, x)		!!((reg) & (1 << (x)))

/* Their opposites */
#define IS_MODE_NATIVE(reg, x)		!IS_MODE_GPIO(reg, x)
#define IS_DIR_OUTPUT(reg, x)		!IS_DIR_INPUT(reg, x)
#define IS_LVL_LOW(reg, x)		!IS_LVL_HIGH(reg, x)

const char *gpiofmt = "gpiobase+0x%*x: 0x%8x %63[^\n]";


uint32_t GPIO_USE_SEL;
uint32_t GP_IO_SEL;
uint32_t GP_LVL;
uint32_t GPO_BLINK;
uint16_t GPI_INV;

uint32_t GPIO_USE_SEL2;
uint32_t GP_IO_SEL2;
uint32_t GP_LVL2;

/* Set 3 only exists on PCH */
uint32_t GPIO_USE_SEL3;
uint32_t GP_IO_SEL3;
uint32_t GP_LVL3;


void parse_gpio(uint32_t value, char* buffer) {
	if (! strcmp(buffer, "(GPIO_USE_SEL)")) {
		GPIO_USE_SEL = value;

	} else if (! strcmp(buffer, "(GP_IO_SEL)")) {
		GP_IO_SEL = value;

	} else if (! strcmp(buffer, "(GP_LVL)")) {
		GP_LVL = value;

	} else if (! strcmp(buffer, "(GPO_BLINK)")) {
		GPO_BLINK = value;

	} else if (! strcmp(buffer, "(GPI_INV)")) {
		GPI_INV = value;

	} else if (! strcmp(buffer, "(GPIO_USE_SEL2)")) {
		GPIO_USE_SEL2 = value;

	} else if (! strcmp(buffer, "(GP_IO_SEL2)")) {
		GP_IO_SEL2 = value;

	} else if (! strcmp(buffer, "(GP_LVL2)")) {
		GP_LVL2 = value;

	} else if (! strcmp(buffer, "(GPIO_USE_SEL3)")) {
		GPIO_USE_SEL3 = value;
	
	} else if (! strcmp(buffer, "(GP_IO_SEL3)")) {
		GP_IO_SEL3 = value;

	} else if (! strcmp(buffer, "(GP_LVL3)")) {
		GP_LVL3 = value;
	}
}

static inline void print_gpio(const uint32_t grp, const uint32_t i) {
	const uint32_t num = (32 * (grp - 1)) + i;

	/* If the GPIO number is only one digit, add an extra space for alignment purposes */
	printf("	.gpio%d %s= ", num, num < 10 ? " " : "");
}

static void print_gpio_set(const uint32_t grp, const uint32_t use_sel, const uint32_t io_sel, const uint32_t lvl) {
	uint32_t i;

	/* Always print out whether the mode is native or GPIO */
	printf("static const struct pch_gpio_set%u pch_gpio_set%u_mode = {\n", grp, grp);
	for (i = 0; i < 32; i++) {

		print_gpio(grp, i);
		printf("GPIO_MODE_%s,\n", IS_MODE_GPIO(use_sel, i) ? "GPIO" : "NATIVE");
	}
	printf("};\n\n");

	/* Then, print out the GPIO direction */
	printf("static const struct pch_gpio_set%u pch_gpio_set%u_direction = {\n", grp, grp);
	for (i = 0; i < 32; i++) {

		/* Only matters for GPIOs used as GPIOs */
		if (IS_MODE_NATIVE(use_sel, i))
			continue;

		print_gpio(grp, i);
		printf("GPIO_DIR_%s,\n", IS_DIR_INPUT(io_sel, i) ? "INPUT" : "OUTPUT");
	}
	printf("};\n\n");

	/* Next, print the GPIO output level */
	printf("static const struct pch_gpio_set%u pch_gpio_set%u_level = {\n", grp, grp);
	for (i = 0; i < 32; i++) {

		/* Only matters for GPIOs used as GPIOs */
		if (IS_MODE_NATIVE(use_sel, i))
			continue;

		/* Only applies to GPIOs used as outputs */
		if (IS_DIR_INPUT(io_sel, i))
			continue;

		print_gpio(grp, i);
		printf("GPIO_LEVEL_%s,\n", IS_LVL_HIGH(lvl, i) ? "HIGH" : "LOW");
	}
	printf("};\n\n");
}


int main() {
	uint32_t value = 0;
	char buffer[64];
	char c;

	/* Parse stdin. Expects inteltool log,
	 * extracts GPIO register values. */
	while (1) {
		int n = scanf(gpiofmt, &value, buffer);

		if (n == 2) {
			parse_gpio(value, buffer);
		} else if (n == EOF) {
			break;
		}

		while ((c = getchar()) != '\n') {
			if (c == EOF) break;
		}
	}


	uint32_t i;

	/* Print out some license boilerplate and header inclusion */
	printf("/* SPDX-License-Identifier: GPL-2.0-only */\n");
	printf("\n");
	printf("#include <southbridge/intel/common/gpio.h>\n");
	printf("\n");

	/* Start off with the first set */
	print_gpio_set(1, GPIO_USE_SEL, GP_IO_SEL, GP_LVL);

	/* The first GPIO set has some extra registers */
	printf("static const struct pch_gpio_set1 pch_gpio_set1_blink = {\n");
	for (i = 0; i < 16; i++) {

		/* Only matters for GPIOs used as GPIOs */
		if (IS_MODE_NATIVE(GPIO_USE_SEL, i))
			continue;

		/* Only applies to GPIOs used as outputs */
		if (IS_DIR_INPUT(GP_IO_SEL, i))
			continue;

		/* If inverted, write it down */
		if (IS_BLINKING(GPO_BLINK, i)) {
			print_gpio(1, i);
			printf("GPIO_BLINK,\n");
		}
	}
	printf("};\n\n");

	/* Only the first 16 bits are meaningful */
	printf("static const struct pch_gpio_set1 pch_gpio_set1_invert = {\n");
	for (i = 0; i < 16; i++) {

		/* Only matters for GPIOs used as GPIOs */
		if (IS_MODE_NATIVE(GPIO_USE_SEL, i))
			continue;

		/* Only applies to GPIOs used as inputs */
		if (IS_DIR_OUTPUT(GP_IO_SEL, i))
			continue;

		/* If inverted, write it down */
		if (IS_INVERTED(GPI_INV, i)) {
			print_gpio(1, i);
			printf("GPIO_INVERT,\n");
		}
	}
	printf("};\n\n");

	/* Now do the second set */
	print_gpio_set(2, GPIO_USE_SEL2, GP_IO_SEL2, GP_LVL2);

	/* And then do the third set, only exists on PCHs it seems */
	print_gpio_set(3, GPIO_USE_SEL3, GP_IO_SEL3, GP_LVL3);

	/* Finally, write the full GPIO map */
	printf(
		"const struct pch_gpio_map mainboard_gpio_map = {\n"
		"	.set1 = {\n"
		"		.mode      = &pch_gpio_set1_mode,\n"
		"		.direction = &pch_gpio_set1_direction,\n"
		"		.level     = &pch_gpio_set1_level,\n"
		"		.blink     = &pch_gpio_set1_blink,\n"
		"		.invert    = &pch_gpio_set1_invert,\n"
		"	},\n"
		"	.set2 = {\n"
		"		.mode      = &pch_gpio_set2_mode,\n"
		"		.direction = &pch_gpio_set2_direction,\n"
		"		.level     = &pch_gpio_set2_level,\n"
		"	},\n"
		"	.set3 = {\n"
		"		.mode      = &pch_gpio_set3_mode,\n"
		"		.direction = &pch_gpio_set3_direction,\n"
		"		.level     = &pch_gpio_set3_level,\n"
		"	},\n"
		"};\n"
	);

	return 0;
}


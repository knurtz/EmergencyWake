#ifndef PT6312_H
#define PT6312_H

#define PT6312_COMMAND_1            (0b00 << 6)
#define PT6312_COMMAND_2            (0b01 << 6)
#define PT6312_COMMAND_3            (0b11 << 6)
#define PT6312_COMMAND_4            (0b10 << 6)

// options for command 1
#define PT6312_MODE_4DIGITS         0U

// options for command 2
#define PT6312_WRITE_DISPLAY_DATA   0U
#define PT6312_AUTO_INCREMENT       0U

// options for command 3
#define PT6312_ADDRESS_DIGIT(n)     (n*2U)

// options for command 4
#define PT6312_PULSEWIDTH_MIN       0U
#define PT6312_PULSEWIDTH_1         0U
#define PT6312_PULSEWIDTH_2         1U
#define PT6312_PULSEWIDTH_4         2U
#define PT6312_PULSEWIDTH_10        3U
#define PT6312_PULSEWIDTH_11        4U
#define PT6312_PULSEWIDTH_12        5U
#define PT6312_PULSEWIDTH_13        6U
#define PT6312_PULSEWIDTH_14        7U
#define PT6312_PULSEWIDTH_MAX       7U
#define PT6312_DISPLAY_ON           (1 << 3)
#define PT6312_DISPLAY_OFF          0U

#endif /* PT6312_H */

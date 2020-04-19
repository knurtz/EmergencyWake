#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include "ch.h"
#include "hal.h"
#include <string.h>
#include "shell.h"
#include "chprintf.h"
#include "ff.h"

// SD test command
void cmd_sdc(BaseSequentialStream *chp, int argc, char *argv[]);

// FatFS tree command
void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]);

// FatFS create command
void cmd_create(BaseSequentialStream *chp, int argc, char *argv[]);

// Hello world command
void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]);

// I2C test command
void cmd_i2c(BaseSequentialStream *chp, int argc, char *argv[]);


#endif /* SHELL_COMMANDS_H */

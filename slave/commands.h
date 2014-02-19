/*
 * File: commands.h
 * Author: Anthony Cargile
 * Description: defines op codes for PIC (both slave daughter board and 
 *     control program at once)
 * Copyright: 2014 Embark Industries, all rights reserved. 
 */
#ifndef _COMMANDS_H_
#define _COMMANDS_H_

// misc. setup stuff
#define SLAVE_BUS_ID 0x04
#define MASTER_BUS_ID 0x03

/*
 * ***** COMMANDS *****
 */
// sent when first initializing
#define OP_HELLO 1

// stops the slave's processing
#define OP_HALT 2

// undo's the OP_HALT command
#define OP_UNHALT 3

// triggers a hard-reset
#define OP_RESET 4

#endif /* _COMMANDS_H_ */

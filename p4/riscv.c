#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "linkedlist.h"
#include "hashtable.h"
#include "riscv.h"

/************** BEGIN HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ***************/
const int R_TYPE = 0;
const int I_TYPE = 1;
const int MEM_TYPE = 2;
const int U_TYPE = 3;
const int UNKNOWN_TYPE = 4;

/**
 * Return the type of instruction for the given operation
 * Available options are R_TYPE, I_TYPE, MEM_TYPE, UNKNOWN_TYPE
 */
static int get_op_type(char *op)
{
    const char *r_type_op[] = {"add", "sub", "and", "or", "xor", "slt", "sll", "sra"};
    const char *i_type_op[] = {"addi", "andi", "ori", "xori", "slti"};
    const char *mem_type_op[] = {"lw", "lb", "sw", "sb"};
    const char *u_type_op[] = {"lui"};
    for (int i = 0; i < (int)(sizeof(r_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(r_type_op[i], op) == 0)
        {
            return R_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(i_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(i_type_op[i], op) == 0)
        {
            return I_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(mem_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(mem_type_op[i], op) == 0)
        {
            return MEM_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(u_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(u_type_op[i], op) == 0)
        {
            return U_TYPE;
        }
    }
    return UNKNOWN_TYPE;
}
/*************** END HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ****************/

registers_t *registers;
hashtable_t *memory;

void init(registers_t *starting_registers)
{
    registers = starting_registers;
    registers->r[0] = 0;
    memory = ht_init(1000);
}

void end()
{
    ht_free(memory);
    free(registers);
}

/**
 * Trim leading whitespace from a string.
 * Credit: https://codeforwin.org/2016/04/c-program-to-trim-leading-white-spaces-in-string.html
 */
void trimLeading(char *str)
{
    int index, i;
    index = 0;
    while (str[index] == ' ' || str[index] == '\t' || str[index] == '\n')
    {
        index++;
    }

    if (index != 0)
    {
        i = 0;
        while (str[i + index] != '\0')
        {
            str[i] = str[i + index];
            i++;
        }
        str[i] = '\0';
    }
}

/**
 * Trim trailing whitespace from a string.
 * Credit: https://codeforwin.org/2016/04/c-program-to-trim-trailing-white-space-characters-in-string.html
 */
void trimTrailing(char *str)
{
    int index, i;
    index = -1;
    i = 0;
    while (str[i] != '\0')
    {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            index = i;
        }
        i++;
    }
    str[index + 1] = '\0';
}

/**
 * Evaluates a given R-Type instruction.
 */
void step_r(char *op, char *insn)
{
    // Parse rd: Seperate by comma, trim trailing whitespace,
    // convert str after 1st char into int (1st char is 'x')
    char *rd_str = strsep(&insn, ",");
    trimTrailing(rd_str);
    int rd = atoi(rd_str + 1);

    // Parse rs1: Same process as rd except initially trim any leading whitespace
    trimLeading(insn);
    char *r1_str = strsep(&insn, ",");
    trimTrailing(r1_str);
    int r1 = atoi(r1_str + 1);

    // Parse rs2: Leftover string should only be x# and any trailing/leading whitespace.
    // Trim and parse.
    trimLeading(insn);
    int r2 = atoi(insn + 1);

    // Perform operation
    if (strcmp(op, "add") == 0)
    {
        registers->r[rd] = registers->r[r1] + registers->r[r2];
    }
    else if (strcmp(op, "sub") == 0)
    {
        registers->r[rd] = registers->r[r1] - registers->r[r2];
    }
    else if (strcmp(op, "and") == 0)
    {
        registers->r[rd] = registers->r[r1] & registers->r[r2];
    }
    else if (strcmp(op, "or") == 0)
    {
        registers->r[rd] = registers->r[r1] | registers->r[r2];
    }
    else if (strcmp(op, "xor") == 0)
    {
        registers->r[rd] = registers->r[r1] ^ registers->r[r2];
    }
    else if (strcmp(op, "slt") == 0)
    {
        registers->r[rd] = registers->r[r1] < registers->r[r2] ? 1 : 0;
    }
    else if (strcmp(op, "sll") == 0)
    {
        registers->r[rd] = registers->r[r1] << registers->r[r2];
    }
    else if (strcmp(op, "sra") == 0)
    {
        registers->r[rd] = registers->r[r1] >> registers->r[r2];
    }
    else
    {
        return;
    }
}

/**
 * Evaluates a given U-Type instruction.
 */
void step_u(char *op, char *insn)
{
    // Parse rd
    char *rd_str = strsep(&insn, ",");
    trimTrailing(rd_str);
    int rd = atoi(rd_str + 1);

    // Parse imm (hex or decimal)
    trimLeading(insn);
    int imm;
    // Is hex;
    if (insn[1] == 'x')
    {
        long hex_num = strtol(insn + 2, NULL, 16);
        imm = hex_num;
    }
    else
    {
        imm = atoi(insn);
    }

    // Perform operation, should always be lui
    if (strcmp(op, "lui") == 0)
    {
        registers->r[rd] = imm << 12;
    }
    else
    {
        return;
    }
}

/**
 * Evaluates a given I-Type instruction.
 */
void step_i(char *op, char *insn)
{
    // Parse rd
    char *rd_str = strsep(&insn, ",");
    trimTrailing(rd_str);
    int rd = atoi(rd_str + 1);

    // Parse rs1
    trimLeading(insn);
    char *r1_str = strsep(&insn, ",");
    trimTrailing(r1_str);
    int r1 = atoi(r1_str + 1);

    // Parse imm (hex or decimal)
    trimLeading(insn);
    int imm;
    // Is hex;
    if (insn[1] == 'x')
    {

        long hex_num = strtol(insn + 2, NULL, 16);
        imm = hex_num;
    }
    else
    {
        imm = atoi(insn);
    }

    // check to sign extend:
    int mask = 0x800;
    if ((mask & imm) == mask)
    {
        int extendMask = 0xfffff000;
        imm |= extendMask;
    }

    // Perform Op
    if (strcmp(op, "addi") == 0)
    {
        registers->r[rd] = registers->r[r1] + imm;
    }
    else if (strcmp(op, "andi") == 0)
    {
        registers->r[rd] = registers->r[r1] & imm;
    }
    else
    {
        return;
    }
}

/**
 * Gets 1st byte of a 4-byte (32-bit) word
 */
int get_byte1(int word)
{
    int mask = 0x000000ff;
    int byte = word & mask;
    // Check if negative, sign extend (if MSB == 1)
    int bit8mask = 0x80;
    if ((bit8mask & byte) == 0x80)
    {
        int extendMask = 0xffffff00;
        byte |= extendMask;
    }
    return byte;
}

/**
 * Gets 2nd byte of a 4-byte (32-bit) word
 */
int get_byte2(int word)
{
    int mask = 0x0000ff00;
    int byte = (word & mask) >> 8;
    // Check if negative, sign extend (if MSB == 1)
    int bit8mask = 0x80;
    if ((bit8mask & byte) == 0x80)
    {
        int extendMask = 0xffffff00;
        byte |= extendMask;
    }
    return byte;
}

/**
 * Gets 3rd byte of a 4-byte (32-bit) word
 */
int get_byte3(int word)
{
    int mask = 0x00ff0000;
    int byte = (word & mask) >> 16;
    // Check if negative, sign extend (if MSB == 1)
    int bit8mask = 0x80;
    if ((bit8mask & byte) == 0x80)
    {
        int extendMask = 0xffffff00;
        byte |= extendMask;
    }
    return byte;
}

/**
 * Gets 4th byte of a 4-byte (32-bit) word
 */
int get_byte4(int word)
{
    int mask = 0xff000000;
    return (word & mask) >> 24;
}

/**
 * Creates 4-byte word given 4 individual bytes
 */
int get_word(int b3, int b2, int b1, int b0)
{
    int msbMask = 0x800000;

    int byte3 = (b3 << 24);

    int byte2 = (b2 << 16);
    // Remove trailing 1s (check msb==1)
    if ((msbMask & byte2) == msbMask)
    {
        int zerosMask = 0xff000000;
        byte2 ^= zerosMask;
    }

    int byte1 = (b1 << 8);
    // Remove trailing 1s (check msb==1)
    if ((msbMask & byte1) == msbMask)
    {
        int zerosMask = 0xffff0000;
        byte1 ^= zerosMask;
    }

    int byte0 = b0;
    // Remove trailing 1s (check msb==1)
    if ((msbMask & byte0) == msbMask)
    {
        int zerosMask = 0xffffff00;
        byte0 ^= zerosMask;
    }

    return byte3 + byte2 + byte1 + byte0;
}

/**
 * Evaluates a given "Mem"-Type instruction.
 */
void step_mem(char *op, char *insn)
{
    // Parse for rd/rs2 (depending on if load/store)
    char *rd_r2_str = strsep(&insn, ",");
    trimTrailing(rd_r2_str);
    int rd_r2 = atoi(rd_r2_str + 1);

    // Parse imm (hex or decimal), seperate on "("
    trimLeading(insn);
    char *imm_str = strsep(&insn, "(");
    trimTrailing(imm_str);
    int imm;
    if (imm_str[1] == 'x')
    {
        long hex_num = strtol(imm_str + 2, NULL, 16);
        imm = hex_num;
    }
    else
    {
        imm = atoi(imm_str);
    }

    int mask = 0x800;
    if ((mask & imm) == mask)
    {
        int extendMask = 0xfffff000;
        imm |= extendMask;
    }

    // parse rs1
    trimLeading(insn);
    char *r1_str = strsep(&insn, ")");
    trimTrailing(r1_str);
    int r1 = atoi(r1_str + 1);

    // Perform Op
    if (strcmp(op, "lw") == 0)
    {
        int addr = (registers->r[r1]) + imm;
        int byte0 = ht_get(memory, addr);
        int byte1 = ht_get(memory, addr + 1);
        int byte2 = ht_get(memory, addr + 2);
        int byte3 = ht_get(memory, addr + 3);
        registers->r[rd_r2] = get_word(byte3, byte2, byte1, byte0);
    }
    else if (strcmp(op, "lb") == 0)
    {
        int addr = (registers->r[r1]) + imm;
        registers->r[rd_r2] = ht_get(memory, addr);
    }
    else if (strcmp(op, "sw") == 0)
    {
        // Split 4-byte register value into each of its 4 bytes
        int word = registers->r[rd_r2];
        int byte1 = get_byte1(word);
        int byte2 = get_byte2(word);
        int byte3 = get_byte3(word);
        int byte4 = get_byte4(word);
        // Add each byte to memory offset by 0-3 depending on byte
        int addr = (registers->r[r1]) + imm;
        ht_add(memory, addr, byte1);
        ht_add(memory, addr + 1, byte2);
        ht_add(memory, addr + 2, byte3);
        ht_add(memory, addr + 3, byte4);
    }
    else if (strcmp(op, "sb") == 0)
    {
        int byte = get_byte1(registers->r[rd_r2]);
        int addr = (registers->r[r1]) + imm;
        ht_add(memory, addr, byte);
    }
    else
    {
        return;
    }
}

void step(char *instruction)
{
    // Extracts and returns the substring before the first space character,
    // by replacing the space character with a null-terminator.
    // `instruction` now points to the next character after the space
    // See `man strsep` for how this library function works
    char *op = strsep(&instruction, " ");
    // Uses the provided helper function to determine the type of instruction
    int op_type = get_op_type(op);
    // Skip this instruction if it is not in our supported set of instructions
    if (op_type == UNKNOWN_TYPE)
    {
        return;
    }

    trimLeading(instruction);
    trimTrailing(instruction);
    if (op_type == R_TYPE)
    {
        step_r(op, instruction);
    }
    else if (op_type == MEM_TYPE)
    {
        step_mem(op, instruction);
    }
    else if (op_type == I_TYPE)
    {
        step_i(op, instruction);
    }
    else if (op_type == U_TYPE)
    {
        step_u(op, instruction);
    }
    registers->r[0] = 0;
}

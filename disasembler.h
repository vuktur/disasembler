#ifndef DISASEMBLER_H
#define DISASEMBLER_H

#include "header.h"
#include "instruction.h"
#include <stdio.h>

/**
 * @brief 
 * 
 * @param source 
 * @param target 
 * @param newStr 
 * @return char* 
 */
char *replacedStr(char *source, char *target, char *newStr);

/**
 * @brief 
 * 
 * @param source 
 * @param field 
 * @param newStr 
 * @return char* 
 */
char *replacedStrField(char *source, char field, char *newStr);

/**
 * @brief 0001 0010 0011 0100 -> 1234 -> "1234"
 * 
 * @param bytes 
 * @param len 
 * @return char* 
 */
char *byteFormatHex(char *bytes, int len);

/**
 * @brief 0001 0010 0011 0100 -> 1234 -> "3412"
 * 
 * @param bytes 
 * @param len 
 * @return char* 
 */
char *byteFormatRevHex(char *bytes, int len);

/**
 * @brief strip the leading zeros of a string
 * 
 * @param str 
 * @return char* 
 */
char *stripLeadingZeros(char *str);

/**
 * @brief 
 * 
 * @param instrType 
 * @param field 
 * @return int 
 */
int fieldExists(const InstructionType *instrType, char field);

/**
 * @brief Get the designated field, 
 * does not apply for big fields with braces
 * 
 * @param instr Instruction*
 * @param field char
 * @return int the value of the field
 */
int getField(Instruction *instr, char field);

/**
 * @brief 
 * 
 * @param byte 
 * @param mod 
 * @param reg 
 * @param rxm 
 * @return int 
 */
int decomposeRegMem(char byte, char *mod, char *reg, char *rxm);

/**
 * @brief 
 * 
 * @param byte char
 * @param str const char*
 * @return 1 if match, 0 if not match, 2 if end of str, -1 if error
 */
int byteMatch(char byte, const char *str);

/**
 * @brief gets pointers to the different parts of the instruction
 *
 * @param str const char*
 * @return const char** the parts (allocated)
 */
const char** splitCodeFormat(const InstructionType* instrType);

/**
 * @brief gets pointers to the different parts of the instruction
 *
 * @param str const char*
 * @param parts const char** the destination, allocated before call
 * @return const char** the parts
 */
const char** splitCodeFormatTo(const InstructionType* instrType, const char **parts);

/**
 * @brief Get the length of the instruction
 * 
 * @param instr 
 * @return int the length
 */
int calcInstrLength(Instruction *instr);

/**
 * @brief Get the length of the instruction
 * 
 * @param instr 
 * @param codeFormatParts 
 * @return int the length 
 */
int calcInstrLengthFrom(Instruction *instr, const char **codeFormatParts);

/**
 * @brief 
 * 
 * @param text 
 * @param textLen 
 * @param pos 
 * @return Instruction 
 */
Instruction readInstruction(char *text, int textLen, unsigned int pos);

/**
 * @brief Get the Disp object
 * 
 * @param mod 
 * @param rxm 
 * @param pos 
 * @param instr 
 * @return char* 
 */
char *getDisp(char mod, char rxm, int pos, Instruction *instr);

/**
 * @brief 
 * 
 * @param pos 
 * @param instr 
 * @return int 
 */
int printInstruction(unsigned int pos, Instruction* instr);

/**
 * @brief 
 * 
 * @param file 
 * @param hdr 
 * @return int 
 */
int readText(FILE *file, Header *hdr);

#endif
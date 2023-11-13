/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"
int sim=1, sig;
int lsp = 0;
int flag = 1;

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STOREP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_LOAD:
        case OPCODE_LOADP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        /*case OPCODE_LOADP:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_STOREP:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs3, stage->rs1,
                   stage->rs2);
            break;
        }*/

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BP:
        case OPCODE_BNP:
        case OPCODE_BN:
        case OPCODE_BNN:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

		case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }
        case OPCODE_CML:
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }
        
        case OPCODE_NOP:
		{
			//printf("NOP");
		}

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    if(!sim){
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
   }

  /* if(sig ==1){
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
   }*/
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */

static void
print_reg_file(const APEX_CPU *cpu)
{
    int i,j;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0, j =  (REG_FILE_SIZE / 2); i < REG_FILE_SIZE / 2, j<REG_FILE_SIZE; ++i,++j)
    {
        printf("R%-3d[%-3d]  R%-3d[%-3d]  \n", i, cpu->regs[i],j,cpu->regs[j]);
    }

    printf("\n");

    printf("\nZero_flag = %d\n",cpu->zero_flag);
    printf("\nPositive flag = %d\n",cpu->p_flag);
    printf("\nNegative flag = %d\n",cpu->n_flag);

    printf("\n\n========== STATE OF DATA MEMORY ==========\n\n");
    for(int i = 0; i < 4096; i++) {
        if(cpu->data_memory[i] != 0){
           printf("|   MEM[%d]\t|\tData Value = %d    |\n", i, cpu->data_memory[i]);
        }
    }


}

static void
simulate(APEX_CPU* cpu)
{
  printf("\n\n== STATE OF UNIFIED PHYSICAL REGISTER FILE ==\n\n");
  for(int i = 0; i < 32; i++) {
    printf("|    REG[%d]\t|\tValue = %d    |\n", i, cpu->regs[i]);
  }

  printf("\n\n========== STATE OF DATA MEMORY ==========\n\n");
  for(int i = 0; i < 100; i++) {
    printf("|   MEM[%d]\t|\tData Value = %d    |\n", i, cpu->data_memory[i]);
  }
}


/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
     if(cpu->stalled){
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;
        cpu->fetch.rs3 = current_ins->rs3;
        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
    }
    else{
        if(!sim){
           printf("Fetch            :EMPTY\n");
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        //printf("flag =%d\n", flag);
        switch (cpu->decode.opcode)
        {
            
            

            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                
                if(cpu->forward_flag){

                  if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                    cpu->stalled = 0;
                        
                    }
                    else{
                        if(cpu->writeback.rd == cpu->decode.rs1 || cpu->writeback.rd == cpu->decode.rs2){
                          if(cpu->writeback.rd == cpu->decode.rs1){
                            cpu->decode.rs1_value = cpu->writeback.result_buffer;
                          }
                          

                          if(cpu->writeback.rd == cpu->decode.rs2){
                            cpu->decode.rs2_value = cpu->writeback.result_buffer;
                          }
                       }
                    

                      if(cpu->execute.rd == cpu->decode.rs1 || cpu->execute.rd == cpu->decode.rs2){
                        if(flag){
                         if(cpu->execute.rd == cpu->decode.rs1){
                             cpu->decode.rs1_value = cpu->execute.result_buffer;
                         }
                        
                         if(cpu->execute.rd == cpu->decode.rs2){
                             cpu->decode.rs2_value = cpu->execute.result_buffer;
                         }
                            
                        }
                        flag = 1;
                      }

                      if(strcmp(cpu->execute.opcode_str,"LOADP") == 0){
                        if(cpu->execute.rs1 == cpu->decode.rs1 || cpu->execute.rs1 == cpu->decode.rs2){
                        if(flag){
                         if(cpu->execute.rs1 == cpu->decode.rs1){
                             cpu->decode.rs1_value = cpu->execute.rs1_value + 4;
                         }
                        
                         if(cpu->execute.rs1 == cpu->decode.rs2){
                             cpu->decode.rs2_value = cpu->execute.rs1_value + 4;
                         }
                            
                        }
                        flag = 1;
                      }
                      }

                      if(strcmp(cpu->execute.opcode_str,"STOREP") == 0){
                        if(cpu->execute.rs2 == cpu->decode.rs1 || cpu->execute.rs2 == cpu->decode.rs2){
                        if(flag){
                         if(cpu->execute.rs2 == cpu->decode.rs1){
                             cpu->decode.rs1_value = cpu->execute.rs2_value + 4;
                         }
                        
                         if(cpu->execute.rs2 == cpu->decode.rs2){
                             cpu->decode.rs2_value = cpu->execute.rs2_value + 4;
                         }
                            
                        }
                        flag = 1;
                      }
                      }

                        cpu->stalled = 1;
                    }
                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                    cpu->stalled = 0;
                    }
                    else{
                    cpu->arr[cpu->decode.rd]++;
                    cpu->stalled = 1;
                    }
                }  
                break;
            }

            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_NOP:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BN:
            case OPCODE_BNN:
            {
               /* BZ,BNZ and NOP doesn't have register operands */
               break;
            }

            case OPCODE_LOADP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];


                if(cpu->forward_flag){
                  
                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->arr[cpu->decode.rd]++;
                        cpu->arr[cpu->decode.rs1]++;
                        if(cpu->writeback.rd == cpu->decode.rs1){
                           cpu->decode.rs1_value = cpu->writeback.result_buffer;
                        }
                        
                        if(flag){
                        if(cpu->execute.rd == cpu->decode.rs1){
                            cpu->decode.rs1_value = cpu->execute.result_buffer;
                            
                        }
                        
                        if(strcmp(cpu->execute.opcode_str,"LOADP")== 0){
                            if(cpu->execute.rs1 == cpu->decode.rs1){
                            cpu->decode.rs1_value = cpu->execute.rs1_value + 4;
                            }
                            
                        }

                        if(strcmp(cpu->execute.opcode_str,"STOREP")== 0){
                            if(cpu->execute.rs2 == cpu->decode.rs1){
                            cpu->decode.rs1_value = cpu->execute.rs2_value + 4;
                            }
                            
                        }

                        }
                        flag = 1;
                        cpu->stalled = 1; //unstalling
                    }

                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {
                    cpu->stalled = 0;
                    }
                    else{
                    cpu->arr[cpu->decode.rd]++;
                    cpu->arr[cpu->decode.rs1]++;
                    cpu->stalled = 1; //unstalling
                    }
                }

                break;
            }

            case OPCODE_LOAD:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                
                if(cpu->forward_flag){
                  

                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->arr[cpu->decode.rd]++;
                        if(cpu->writeback.rd == cpu->decode.rs1){
                           cpu->decode.rs1_value = cpu->writeback.result_buffer;
                        }
                        
                        if(flag){
                        if(cpu->execute.rd == cpu->decode.rs1){
                            cpu->decode.rs1_value = cpu->execute.result_buffer;
                            
                        }

                        if(strcmp(cpu->execute.opcode_str,"LOADP") == 0){
                            if(cpu->execute.rs1 == cpu->decode.rs1){
                                cpu->decode.rs1_value = cpu->execute.rs1_value + 4;
                            }    
                        }

                        if(strcmp(cpu->execute.opcode_str,"STOREP") == 0){
                            if(cpu->execute.rs2 == cpu->decode.rs1){
                                cpu->decode.rs1_value = cpu->execute.rs2_value + 4;
                            }    
                        }


                        }
                        flag = 1;
                        cpu->stalled = 1; //unstalling
                    }

                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {
                    cpu->stalled = 0;
                    }
                    else{
                    cpu->arr[cpu->decode.rd]++;
                    cpu->stalled = 1; //unstalling
                    }
                }
                break;
            }


            case OPCODE_STOREP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                
                if(cpu->forward_flag){

                    if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->arr[cpu->decode.rs2]++;
                        if(cpu->writeback.rd == cpu->decode.rs1 || cpu->writeback.rd == cpu->decode.rs2){
                           if(cpu->writeback.rd == cpu->decode.rs1)
                             cpu->decode.rs1_value = cpu->writeback.result_buffer;
 
                           if(cpu->writeback.rd == cpu->decode.rs2)
                             cpu->decode.rs2_value = cpu->writeback.result_buffer;

                        }
                        
                        if(cpu->execute.rd == cpu->decode.rs1 || cpu->execute.rd == cpu->decode.rs2 ){
                         if(flag){
                            if(cpu->execute.rd == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->execute.result_buffer;

                            if(cpu->execute.rd == cpu->decode.rs2)
                              cpu->decode.rs2_value = cpu->execute.result_buffer;

                        }
                         flag = 1;
                        }

                        if(strcmp(cpu->execute.opcode_str,"LOADP") == 0){
                        if(cpu->execute.rs1 == cpu->decode.rs1 || cpu->execute.rs1 == cpu->decode.rs2 ){
                         if(flag){
                            if(cpu->execute.rs1 == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->execute.rs1_value + 4;

                            if(cpu->execute.rs1 == cpu->decode.rs2)
                              cpu->decode.rs2_value = cpu->execute.rs1_value +4;

                        }
                         flag = 1;
                        }
                        }

                        if(strcmp(cpu->execute.opcode_str,"STOREP") == 0){
                        if(cpu->execute.rs2 == cpu->decode.rs1 || cpu->execute.rs2 == cpu->decode.rs2 ){
                         if(flag){
                            if(cpu->execute.rs2 == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->execute.rs2_value + 4;

                            if(cpu->execute.rs1 == cpu->decode.rs2)
                              cpu->decode.rs2_value = cpu->execute.rs2_value +4;

                        }
                         flag = 1;
                        }
                        }
                            
                            cpu->stalled = 1;
                    }
                  

                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->arr[cpu->decode.rs2]++;
                        cpu->stalled = 1;
                    }
                }
                break;
            }

            case OPCODE_STORE:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                
                if(cpu->forward_flag){

                    if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        if(cpu->writeback.rd == cpu->decode.rs1 || cpu->writeback.rd == cpu->decode.rs2){
                           if(cpu->writeback.rd == cpu->decode.rs1)
                             cpu->decode.rs1_value = cpu->writeback.result_buffer;
 
                           if(cpu->writeback.rd == cpu->decode.rs2)
                             cpu->decode.rs2_value = cpu->writeback.result_buffer;

                        }
                        
                        if(cpu->execute.rd == cpu->decode.rs1 || cpu->execute.rd == cpu->decode.rs2){
                         if(flag){
                            if(cpu->execute.rd == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->execute.result_buffer;

                            if(cpu->execute.rd == cpu->decode.rs2)
                              cpu->decode.rs2_value = cpu->execute.result_buffer;

                        }
                         flag = 1;
                        }

                        if(strcmp(cpu->execute.opcode_str,"LOADP") == 0){
                        if(cpu->execute.rs1 == cpu->decode.rs1 || cpu->execute.rs1 == cpu->decode.rs2 ){
                         if(flag){
                            if(cpu->execute.rs1 == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->execute.rs1_value + 4;

                            if(cpu->execute.rs1 == cpu->decode.rs2)
                              cpu->decode.rs2_value = cpu->execute.rs1_value +4;

                        }
                         flag = 1;
                        }
                        }

                        if(strcmp(cpu->execute.opcode_str,"STOREP") == 0){
                        if(cpu->execute.rs2 == cpu->decode.rs1 || cpu->execute.rs2 == cpu->decode.rs2 ){
                         if(flag){
                            if(cpu->execute.rs2 == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->execute.rs2_value + 4;

                            if(cpu->execute.rs1 == cpu->decode.rs2)
                              cpu->decode.rs2_value = cpu->execute.rs2_value +4;

                        }
                         flag = 1;
                        }
                        }
                            
                            cpu->stalled = 1;
                    }
                  

                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->stalled = 1;
                    }
                }
                
                break;
            }


            case OPCODE_CML:
            case OPCODE_JUMP:
            {
                
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];


                if(cpu->forward_flag){
                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->stalled = 1;
                        if(cpu->writeback.rd == cpu->decode.rs1){
                            if(cpu->writeback.rd == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->writeback.result_buffer;

                        }
                        if(cpu->execute.rd == cpu->decode.rs1){
                          if(flag){
                            if(cpu->execute.rd == cpu->decode.rs1)
                                cpu->decode.rs1_value = cpu->execute.result_buffer;
                          }
                          flag = 1;
                        }

                        if(strcmp(cpu->execute.opcode_str,"LOADP") == 0){
                            if(flag){
                                if(cpu->execute.rs1 == cpu->decode.rs1)
                                    cpu->decode.rs1_value = cpu->execute.rs1_value + 4;
                            }
                          flag = 1;

                        }

                        if(strcmp(cpu->execute.opcode_str,"STOREP") == 0){
                          if(flag){
                            if(cpu->execute.rs2 == cpu->decode.rs1)
                                cpu->decode.rs1_value = cpu->execute.rs2_value + 4;
                          }
                          flag = 1;
                        }
                    }

                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {   
                        cpu->stalled = 0;
                        //printf("CMP Stalled\n");
                    }
                    else{
                        cpu->stalled = 1;
                    }
                }
                break;
            }

            case OPCODE_CMP:
            {
                
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                
                if(cpu->forward_flag){
                    if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->stalled = 1;
                        if(cpu->writeback.rd == cpu->decode.rs1 || cpu->writeback.rd == cpu->decode.rs2){
                            if(cpu->writeback.rd == cpu->decode.rs1)
                               cpu->decode.rs1_value = cpu->writeback.result_buffer;
                            
                            if(cpu->writeback.rd == cpu->decode.rs2)
                              cpu->decode.rs2_value = cpu->writeback.result_buffer;

                        }
                        if(cpu->execute.rd == cpu->decode.rs1 || cpu->execute.rd == cpu->decode.rs2){
                          if(flag){
                            if(cpu->execute.rd == cpu->decode.rs1)
                                cpu->decode.rs1_value = cpu->execute.result_buffer;
                            
                            if(cpu->execute.rd == cpu->decode.rs2)
                                cpu->decode.rs2_value = cpu->execute.result_buffer;
                          }
                          flag = 1;
                        }

                        if(strcmp(cpu->execute.opcode_str,"LOADP") == 0){
                        if(cpu->execute.rs1 == cpu->decode.rs1 || cpu->execute.rs1 == cpu->decode.rs2){
                          if(flag){
                            if(cpu->execute.rs1 == cpu->decode.rs1)
                                cpu->decode.rs1_value = cpu->execute.rs1_value + 4;
                            
                            if(cpu->execute.rs1 == cpu->decode.rs2)
                                cpu->decode.rs2_value = cpu->execute.rs1_value + 4;
                          }
                          flag = 1;
                        }

                        }

                        if(strcmp(cpu->execute.opcode_str,"STOREP") == 0){
                         if(cpu->execute.rs2 == cpu->decode.rs1 || cpu->execute.rs2 == cpu->decode.rs2){
                          if(flag){
                            if(cpu->execute.rs2 == cpu->decode.rs1)
                                cpu->decode.rs1_value = cpu->execute.rs2_value + 4;
                            
                            if(cpu->execute.rs2 == cpu->decode.rs2)
                                cpu->decode.rs2_value = cpu->execute.rs2_value + 4;
                          }
                          flag = 1;
                        }
                        }
                    }

                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0 || cpu->arr[cpu->decode.rs2] != 0)
                    {
                        cpu->stalled = 0;
                    }
                    else{
                        cpu->stalled = 1;
                    }
                }
                
                break;
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                if(cpu->forward_flag){

                }
                else{
                   cpu->arr[cpu->decode.rd]++;
                }
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JALR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                 if(cpu->forward_flag){
                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {
                       cpu->stalled = 0;
                    }
                    else{
                        //printf("enter else in ADDL\n");
                        if(cpu->writeback.rd == cpu->decode.rs1){
                            cpu->decode.rs1_value = cpu->writeback.result_buffer;
                        }
                        
                        if(flag){
                           // printf("enter flag else in ADDL\n");
                        if(cpu->execute.rd == cpu->decode.rs1){
                                cpu->decode.rs1_value = cpu->execute.result_buffer;   
                        }

                        if(strcmp(cpu->execute.opcode_str,"LOADP") == 0){
                            if(cpu->execute.rs1 == cpu->decode.rs1){
                                cpu->decode.rs1_value = cpu->execute.rs1_value + 4;   
                        }
                        }

                        if(strcmp(cpu->execute.opcode_str,"STOREP") == 0){
                            //printf("enter STOREP else in ADDL\n");
                            if(cpu->execute.rs2 == cpu->decode.rs1){
                                cpu->decode.rs1_value = cpu->execute.rs2_value + 4;   
                        }
                        }


                        flag = 1;
                        }
                        cpu->stalled = 1;
                    }
                }
                else{
                    if(cpu->arr[cpu->decode.rs1] != 0)
                    {
                    cpu->stalled = 0;
                    }
                    else{
                    cpu->arr[cpu->decode.rd]++;
                    cpu->stalled = 1;
                    }
                }
                break;
            }

        }
        if(cpu->stalled){
           /* Copy data from decode latch to execute latch*/
           cpu->execute = cpu->decode;
           cpu->decode.has_insn = FALSE;
        }

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
     
    }
    else{
        if(!sim){
          printf("Decode           :EMPTY\n");
        }
    }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    //printf(" %s %d\n",cpu->execute.opcode_str,cpu->execute.has_insn);
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_SUB:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_MUL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value * cpu->execute.rs2_value;
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_AND:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value & cpu->execute.rs2_value;
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_OR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value | cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_XOR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value ^ cpu->execute.rs2_value;

                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.imm;

                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_SUBL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.imm;

                
                if (cpu->execute.result_buffer > 0){
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 

                if (cpu->execute.result_buffer < 0){
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }    

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer != 0)
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                }
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }

            case OPCODE_LOADP:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                lsp = cpu->execute.rs1_value + 4;
                //printf("%d loadp\n",lsp);
                break;
            }

            case OPCODE_JUMP:
            {
               /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.rs1_value+ cpu->execute.imm;
                    //printf("  %d  ", cpu->pc);
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
            }

            case OPCODE_JALR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.pc + 4;
                    
                /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;
                    //printf("  %d  ", cpu->pc);
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
            }

            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }


            case OPCODE_BP:
            {
                if (cpu->p_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNP:
            {
                if (cpu->p_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BN:
            {
                if (cpu->n_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNN:
            {
                if (cpu->n_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }


            case OPCODE_STORE:
            {
               cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                
               break;
            }

            case OPCODE_STOREP:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                lsp = cpu->execute.rs2_value + 4;
               break;
            }

            case OPCODE_CMP:
            {
               if (cpu->execute.rs1_value == cpu->execute.rs2_value)
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                 
                if (cpu->execute.rs1_value > cpu->execute.rs2_value)
                {
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 
                else 
                {
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }

               break;
            }


            case OPCODE_CML:
            {
                if (cpu->execute.rs1_value == cpu->execute.imm)
                {
                    cpu->zero_flag = TRUE;
                    cpu->p_flag = FALSE;
                    cpu->n_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.rs1_value > cpu->execute.imm)
                {
                    cpu->p_flag = TRUE;
                    cpu->n_flag = FALSE;
                } 
                else 
                {
                    cpu->p_flag = FALSE;
                    cpu->n_flag = TRUE;
                }

               break;

            }


            case OPCODE_MOVC: 
            {
                cpu->execute.result_buffer = cpu->execute.imm;
                break;
            }
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;
        
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
    else{
        if(!sim){
           printf("Execute         :EMPTY\n");
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_BNZ:
			case OPCODE_BZ:
			case OPCODE_HALT:
			case OPCODE_NOP:
			case OPCODE_MOVC:
			case OPCODE_CMP:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_CML:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BN:
            case OPCODE_BNN:
            case OPCODE_JALR:
            case OPCODE_JUMP:
            {
                /* No work for ADD */
                //printf("memory stage %d",cpu->memory.rd);
                break;
            }

            case OPCODE_LOAD:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                    
                    if(cpu->forward_flag){
                      cpu->arr[cpu->memory.rd]--;
                      if(cpu->memory.rd == cpu->decode.rs1 || cpu->memory.rd == cpu->decode.rs2){
                        flag = 0;
                      }
                    }
                break;
            }

            case OPCODE_STORE:
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
                break;
            }

            case OPCODE_STOREP:
            {
                
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
                if(cpu->forward_flag){
                    cpu->arr[cpu->memory.rs2]--;
                    
                }
                break;
            }

            case OPCODE_LOADP:
            {
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                if(cpu->forward_flag){
                      cpu->arr[cpu->memory.rd]--;
                      cpu->arr[cpu->memory.rs1]--;
                      if(cpu->memory.rd == cpu->decode.rs1 || cpu->memory.rd == cpu->decode.rs2 || cpu->memory.rs1 == cpu->decode.rs1 || cpu->memory.rs1 == cpu->decode.rs2){
                        flag = 0;
                      }
                    }
                break;
            }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
    else{
        if(!sim){
           printf("Memory           :EMPTY\n");
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_ADDL:
			case OPCODE_SUB:
			case OPCODE_SUBL:
			case OPCODE_MUL:
			case OPCODE_AND:
			case OPCODE_OR:
			case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                if(cpu->forward_flag){

                }
                else{
                    printf("entered writeback else\n\n");
                cpu->arr[cpu->writeback.rd]--;
                }
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                if(cpu->forward_flag){

                }
                else{
                cpu->arr[cpu->writeback.rd]--;
                }
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                if(cpu->forward_flag){

                }
                else{
                    printf("entered writeback else\n\n");
                    cpu->arr[cpu->writeback.rd]--;
                }
                break;
            }

            case OPCODE_STORE:
			{
                break;
			}

            case OPCODE_LOADP:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rs1] = lsp;

                if(cpu->forward_flag){

                }
                else{
                cpu->arr[cpu->writeback.rd]--;
                cpu->arr[cpu->writeback.rs1]--;
                }
                break;
            }

            case OPCODE_STOREP:
            {
                cpu->regs[cpu->writeback.rs2] = lsp;

                if(cpu->forward_flag){

                }
                else{
                cpu->arr[cpu->writeback.rs2]--;
                }
            
                break;
            }


            case OPCODE_JALR: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                if(cpu->forward_flag){

                }
                else{
                cpu->arr[cpu->writeback.rd]--;
                }
                break;
            }



            case OPCODE_CMP:
            case OPCODE_CML:
			{
				break;
			}
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }

        cpu->stalled = 1;
    }
    else{
        if(!sim){
          printf("WriteBack        :EMPTY\n");
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename, const int num, const int cycles, const int forward_flag)
{
    int i;
    APEX_CPU *cpu;
    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }
    cpu->clock = 0;
    if(num == 1){
        cpu->simulate = 1;
        cpu->cycles = cycles;
    }
    else if(num == 2){
        cpu->display = 2;
        cpu->cycles =cycles;
        cpu->forward_flag = forward_flag;
    }
    else if(num == 3){
        cpu->showmem = 3;
        cpu->mem = cycles;
        cpu->forward_flag = forward_flag;
        sim = 1;
    }
    else if(num == 4){
        cpu->fwd = 4;
        cpu->forward_flag = forward_flag;
        sim = 0;
    }
    else{
        sig = 1;
    }

    /* To start fetch stage */
    cpu->stalled = 1;
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        //printf("sim = %d", sim);
        if(!sim){
         if (ENABLE_DEBUG_MESSAGES)
         {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
         }
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        
        if(cpu->simulate == 1){
            //sim = 0;
            if(cpu->cycles == cpu->clock){
               simulate(cpu);
             
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
            }
        }
        if(cpu->display == 2){
            sim = 0;
           print_reg_file(cpu);
           if(cpu->cycles == cpu->clock){
        
            //display(cpu);
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
           }
        }
        if(cpu->fwd == 4){
            sim = 0;
            //printf("\n\n entered fwd single\n");
            print_reg_file(cpu);
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        if(sig == 1){
            sim = 0;
            print_reg_file(cpu);
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }



        cpu->clock++;
    }

    if(cpu->showmem == 3){
        printf("Data at location |    M[%d] = %d   |\n",cpu->mem,cpu->data_memory[cpu->mem]);
    }

    if(cpu->simulate == 1)
    {
        sim = 1;
        if(cpu->cycles > cpu->clock){
            simulate(cpu);
            printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
        }
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
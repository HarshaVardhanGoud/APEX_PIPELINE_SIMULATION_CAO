/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"

int 
main(int argc, char const *argv[])
{
    int cmd = 0, cycle = 0; 
    const char* scmd;
    APEX_CPU *cpu;
    printf(" argc  %d   ",argc);
    //int cmd = 0;
    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);
    if (argc != 4)
    {   
        if(argc != 3){
            
            if(argc !=2){
              fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
              exit(1);
            }
        }
    }

    if(argc > 2){
        scmd = argv[2];
    }
   
    //printf("Argument = %s , %d \n",argv[2],strcmp(argv[2], "Display"));
    //strcmp(argv[2], "Simulate")
    if (strcmp(scmd, "Simulate") == 0)
    { 
        //printf("Argument = %s \n", argv[2]); 
        cmd = 1;
        cycle = atoi(argv[3]);
        //printf("Argument = %s cmd = %d, cycle = %d\n", argv[2],cmd,cycle); 
    }
    else if(strcmp(scmd, "Display") == 0)
    {
        //printf("Argument = %s \n", argv[2]); 
        cmd = 2;
        cycle = atoi(argv[3]);
        //printf("Argument = %s cmd = %d\n", argv[2],cmd); 
    }
    else if(strcmp(scmd,"ShowMem") == 0){
     //printf("Argument = %s \n", argv[2]);
        cmd = 3;
        cycle = atoi(argv[3]);
        //printf("Argument = %s cmd = %d\n", argv[2],cmd); 
    }
    else if(strcmp(scmd,"Single_step") == 0){
     //printf("Argument = %s \n", argv[2]);
        cmd = 4;
    }
   // printf("\narg3 = %d\n", atoi(argv[3]));
    cpu = APEX_cpu_init(argv[1],cmd,cycle);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }

    APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
    return 0;
}
# APEX_PIPELINE_SIMULATION_CAO
In Order, Apex Pipeline Simulator uses stalling, Score boarding, and register naming.

APEX Pipeline Simulator

A simple implementation of 5 Stage APEX Pipeline

Notes:
----------------------------------------------------------------------------------
1) This code is a simple implementation of the 5-Stage APEX Pipeline. 
	 
	 Fetch -> Decode -> Execute -> Memory -> Writeback
	 
	 You can read, modify, and build upon the given codebase to add other features as
	 required in the project description. You are also free to write your own 
	 implementation from scratch.

2) All the stages have a latency of one cycle. There is a single functional unit in 
	 EX stage which performs all the arithmetic and logic operations in Apex Simulator 1.

3) Apex Simulator 2 uses scoreboarding logic and transfers values from the execute stage to
   decode stage instead of waiting till instruction reaches the writeback stage. 


File-Info
----------------------------------------------------------------------------------
1) Makefile 			- You can edit as needed
2) file_parser.c 	- Contains Functions to parse input files. No need to change this file
3) apex_cpu.c          - Contains Implementation of APEX CPU. You can edit as needed
4) apex_cpu.h          - Contains various data structures declarations needed by 'cpu.c'. You can edit as needed
	 

How to compile and run
----------------------------------------------------------------------------------
1) go to the terminal, cd into the project directory, and type 'make' to compile the project
2) Run using ./apex_sim <input file name>

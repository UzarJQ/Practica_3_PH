# *******************************************************
# * NAME    : 44BINIT.S					*
# * Version : 10.April.2000				*
# * Description:					*
# *	C start up codes				*
# *	Configure memory, Initialize ISR ,stacks	*
# *	Initialize C-variables				*
# *	Fill zeros into zero-initialized C-variables	*
# *******************************************************

    .include "option.a"
    .include "memcfg.a"

#Memory Area
#GCS6    8M 16bit(8MB) DRAM/SDRAM(0xc000000-0xc7fffff)
#APP     RAM=0xc000000~0xc7effff 
#EV_boot RAM=0xc7f0000-0xc7ff000 // if EV_boot
#STACK	 =0xc7ffa00

#Interrupt Control
.equ 	INTPND,		0x01e00004
.equ 	INTMOD,		0x01e00008
.equ 	INTMSK,		0x01e0000c
.equ 	I_ISPR,		0x01e00020
.equ 	I_CMST,		0x01e0001c

#Watchdog timer
.equ 	WTCON,		0x01d30000

#Clock Controller
.equ 	PLLCON,		0x01d80000
.equ 	CLKCON,		0x01d80004
.equ 	LOCKTIME,	0x01d8000c
	
#Memory Controller
.equ 	REFRESH,	0x01c80024

#BDMA destination register
.equ 	BDIDES0,	0x1f80008
.equ 	BDIDES1,	0x1f80028

#Pre-defined constants
.equ 	USERMODE,	0x10
.equ 	FIQMODE,	0x11
.equ 	IRQMODE,	0x12
.equ 	SVCMODE,	0x13
.equ 	ABORTMODE,	0x17
.equ 	UNDEFMODE,	0x1b
.equ 	MODEMASK,	0x1f
.equ 	NOINT,		0xc0
.equ    IRQ_MODE,	0x40       /* disable Interrupt Mode (IRQ) */
.equ    FIQ_MODE,	0x80       /* disable Fast Interrupt Mode (FIQ) */

.macro HANDLER HandleLabel
    sub	    sp,sp,#4	    /* decrement sp(to store jump address) */							
    stmfd   sp!,{r0}	    /* PUSH the work register to stack (lr does not push because it return to original address) */
    ldr	    r0,=\HandleLabel/* load the address of HandleXXX to r0 */
    ldr	    r0,[r0]	    	/* load the contents(service routine start address) of HandleXXX */
    str	    r0,[sp,#4]	    /* store the contents(ISR) of HandleXXX to stack */
    ldmfd   sp!,{r0,pc}	    /* POP the work register and pc(jump to ISR) */
.endm

    .extern       Image_RO_Limit    /* End of ROM code (=start of ROM data) */
    .extern       Image_RW_Base     /* Base of RAM to initialise */           
    .extern       Image_ZI_Base     /* Base and limit of area */              
    .extern       Image_ZI_Limit    /* to zero initialise */       

    .extern Main			/* The main entry of mon program */
    
    .text

    ENTRY:
    b ResetHandler			/* for debug            */
    b HandlerUndef      	/* handlerUndef         */
    b HandlerSWI        	/* SWI interrupt handler*/
    b HandlerPabort     	/* handlerPAbort        */
    b HandlerDabort     	/* handlerDAbort        */
    b .                 	/* handlerReserved      */
    b HandlerIRQ
    b HandlerFIQ
	#***IMPORTANT NOTE***
	#If the H/W vectored interrutp mode is enabled, The above two instructions should
	#be changed like below, to work-around with H/W bug of S3C44B0X interrupt controller. 
	# b HandlerIRQ  ->  subs pc,lr,#4
	# b HandlerIRQ  ->  subs pc,lr,#4

VECTOR_BRANCH:
    ldr pc,=HandlerEINT0    /*mGA    H/W interrupt vector table  */
    ldr pc,=HandlerEINT1    /*	                                 */	
    ldr pc,=HandlerEINT2    /*                                   */  
    ldr pc,=HandlerEINT3    /*                                   */  
    ldr pc,=HandlerEINT4567 /*                                   */  
    ldr pc,=HandlerTICK	    /*mGA                                */   
    b .                                                          
    b .                                                         
    ldr pc,=HandlerZDMA0    /*mGB                                */  
    ldr pc,=HandlerZDMA1    /*                                   */  
    ldr pc,=HandlerBDMA0    /*                                   */  
    ldr pc,=HandlerBDMA1    /*                                   */  
    ldr pc,=HandlerWDT	    /*                                   */   
    ldr pc,=HandlerUERR01   /*mGB                                */  
    b .                                                          
    b .                                                          
    ldr pc,=HandlerTIMER0   /*mGC                                */  
    ldr pc,=HandlerTIMER1   /*                                   */  
    ldr pc,=HandlerTIMER2   /*                                   */  
    ldr pc,=HandlerTIMER3   /*                                   */  
    ldr pc,=HandlerTIMER4   /*                                   */  
    ldr pc,=HandlerTIMER5   /*mGC                                */  
    b .                                                          
    b .                                                          
    ldr pc,=HandlerURXD0    /*mGD                                */  
    ldr pc,=HandlerURXD1    /*                                   */  
    ldr pc,=HandlerIIC	    /*                                   */   
    ldr pc,=HandlerSIO	    /*                                   */   
    ldr pc,=HandlerUTXD0    /*                                   */  
    ldr pc,=HandlerUTXD1    /*mGD                                */  
    b .                                                          
    b .                                                          
    ldr pc,=HandlerRTC	    /*mGKA                               */   
    b .					    /*                     		         */
    b .					    /*                     		         */
    b .					    /*                     		         */
    b .					    /*                     		         */
    b .					    /*mGKA                 			     */
    b .                                                          
    b .                                                          
    ldr pc,=HandlerADC	    /*mGKB                               */  
    b .					    /*                     		         */
    b .					    /*                     		         */
    b .					    /*                     		         */
    b .					    /*                     		         */
    b .					    /*mGKB                 		         */
    b .                                                          
    b .                                                          
@0xe0=EnterPWDN                                                 
    ldr pc,=EnterPWDN

@   .ltorg
          	/* the current contents of the literal pool\
               to be dumped into the current section\ 
               (which is assumed to be the .text section)\ 
               at the current location (aligned to a word boundary).*/
   .align

HandlerFIQ:		HANDLER HandleFIQ
HandlerIRQ:		HANDLER HandleIRQ
HandlerUndef:	HANDLER HandleUndef
HandlerSWI:		HANDLER HandleSWI
HandlerDabort:	HANDLER HandleDabort
HandlerPabort:	HANDLER HandlePabort
HandlerADC:		HANDLER HandleADC
HandlerRTC:		HANDLER HandleRTC
HandlerUTXD1:	HANDLER HandleUTXD1
HandlerUTXD0:	HANDLER HandleUTXD0
HandlerSIO:		HANDLER HandleSIO
HandlerIIC:		HANDLER HandleIIC
HandlerURXD1:	HANDLER HandleURXD1
HandlerURXD0:	HANDLER HandleURXD0
HandlerTIMER5:	HANDLER HandleTIMER5
HandlerTIMER4:	HANDLER HandleTIMER4
HandlerTIMER3:	HANDLER HandleTIMER3
HandlerTIMER2:	HANDLER HandleTIMER2
HandlerTIMER1:	HANDLER HandleTIMER1
HandlerTIMER0:	HANDLER HandleTIMER0
HandlerUERR01:	HANDLER HandleUERR01
HandlerWDT:		HANDLER HandleWDT
HandlerBDMA1:	HANDLER HandleBDMA1
HandlerBDMA0:	HANDLER HandleBDMA0
HandlerZDMA1:	HANDLER HandleZDMA1
HandlerZDMA0:	HANDLER HandleZDMA0
HandlerTICK:	HANDLER HandleTICK
HandlerEINT4567:HANDLER HandleEINT4567
HandlerEINT3:	HANDLER HandleEINT3
HandlerEINT2:	HANDLER HandleEINT2
HandlerEINT1:	HANDLER HandleEINT1
HandlerEINT0:	HANDLER HandleEINT0

#One of the following two routines can be used for non-vectored interrupt.

IsrIRQ:						/* using I_ISPR register. */
    sub	    sp,sp,#4       	/* reserved for PC	  */
    stmfd   sp!,{r8-r9}   

#IMPORTANT CAUTION
# when I_ISPC is not used properly, I_ISPR can be 0 in this routine.

    ldr	    r9,=I_ISPR
    ldr	    r9,[r9]

	cmp		r9, #0x0		/* If the IDLE mode work-around is used, 	*/
							/* r9 may be 0 sometimes.			*/
	beq		l2

    mov	    r8,#0x0
l0:
    movs    r9,r9,lsr #1
    bcs	    l1
    add	    r8,r8,#4
    b	    l0

l1:
    ldr	    r9,=HandleADC
    add	    r9,r9,r8
    ldr	    r9,[r9]
    str	    r9,[sp,#8]
    ldmfd   sp!,{r8-r9,pc}

l2:
	ldmfd	sp!,{r8-r9}
	add		sp,sp,#4
	subs	pc,lr,#4

#****************************************************
#*	START											*
#****************************************************
ResetHandler:
    ldr	    r0,=WTCON	    	/* watch dog disable*/
    ldr	    r1,=0x0 		
    str	    r1,[r0]

    ldr	    r0,=INTMSK
    ldr	    r1,=0x07ffffff  	/* all interrupt disable */
    str	    r1,[r0]

    #****************************************************
    #*	Set clock control registers						*
    #****************************************************
    ldr		r0,=LOCKTIME
    ldr		r1,=0xfff
    str		r1,[r0]

.if PLLONSTART
	ldr		r0,=PLLCON			/* temporary setting of PLL */
	ldr		r1,=((M_DIV<<12)+(P_DIV<<4)+S_DIV)	/* Fin=8MHz,Fout=64MHz     */
	str		r1,[r0]
.endif

    ldr	    r0,=CLKCON		
    ldr	    r1,=0x7ff8	    	/* All unit block CLK enable */
    str	    r1,[r0]

    #****************************************
    #*  change BDMACON reset value for BDMA *   
    #****************************************
    ldr     r0,=BDIDES0      
    ldr     r1,=0x40000000   	/* BDIDESn reset value should be 0x40000000 */
    str     r1,[r0]

    ldr     r0,=BDIDES1      
    ldr     r1,=0x40000000   	/* BDIDESn reset value should be 0x40000000 */	 
    str     r1,[r0]

    #****************************************************
    #*	Set memory control registers					* 	
    #****************************************************
    ldr	    r0,=SMRDATA
    ldmia   r0,{r1-r13}
    ldr	    r0,=0x01c80000  	/* BWSCON Address */
    stmia   r0,{r1-r13}

    #;****************************************************
    #;*	Initialize stacks								* 
    #;****************************************************
    ldr	    sp, =SVCStack		/* Why	*/		
    bl	    InitStacks

    #;****************************************************
    #;*	Setup IRQ handler								*
    #;****************************************************
    ldr	    r0,=HandleIRQ		/* This routine is needed */
    ldr	    r1,=IsrIRQ			/* if there is not 'subs pc,lr,#4' at 0x18, 0x1c */
    str	    r1,[r0]

    #********************************************************
    #*	Copy and paste RW data/zero initialized data	    *
    #********************************************************
    LDR	    r0, =Image_RO_Limit	/* Get pointer to ROM data */
    LDR	    r1, =Image_RW_Base	/* and RAM copy	*/
    LDR	    r3, =Image_ZI_Base	
	/* Zero init base => top of initialised data */
			
    CMP	    r0, r1	    		/* Check that they are different */
    BEQ	    F1
F0:
    CMP	    r1, r3				/* Copy init data                        */
    LDRCC   r2, [r0], #4        /* --> LDRCC r2, [r0] + ADD r0, r0, #4	 */
    STRCC   r2, [r1], #4        /* --> STRCC r2, [r1] + ADD r1, r1, #4   */ 
    BCC	    F0
F1:
    LDR	    r1, =Image_ZI_Limit	/* Top of zero init segment */
    MOV	    r2, #0
F2:
    CMP	    r3, r1	    		/* Zero init */
    STRCC   r2, [r3], #4
    BCC	    F2

	MRS	r0, CPSR
	BIC	r0, r0, #NOINT /* enable interrupt */
	MSR	CPSR_cxsf, r0
	/* jump to main() */
   	BL	Main
   	B   .	    

#;****************************************************
#;*	The function for initializing stack				*
#;****************************************************
InitStacks:
	# Don't use DRAM,such as stmfd,ldmfd......
	# SVCstack is initialized before
	# Under toolkit ver 2.50, 'msr cpsr,r1' can be used instead of 'msr cpsr_cxsf,r1'

    mrs	    r0,cpsr
    bic	    r0,r0,#MODEMASK
    orr	    r1,r0,#UNDEFMODE
    msr	    cpsr_cxsf,r1		/* UndefMode */
    ldr	    sp,=UndefStack
	
    orr	    r1,r0,#ABORTMODE|NOINT
    msr	    cpsr_cxsf,r1 	    /* AbortMode */	
    ldr	    sp,=AbortStack

    orr	    r1,r0,#IRQMODE|FIQ_MODE
    msr	    cpsr_cxsf,r1 	    /* IRQMode */
    ldr	    sp,=IRQStack
	
    orr	    r1,r0,#FIQMODE|IRQ_MODE
    msr	    cpsr_cxsf,r1 	    /* FIQMode */
    ldr	    sp,=FIQStack

    bic	    r0,r0,#MODEMASK
    orr	    r1,r0,#SVCMODE
    msr	    cpsr_cxsf,r1 	    /* SVCMode */
    ldr	    sp,=SVCStack

	#USER mode is not initialized.
    mov	    pc,lr 				/* The LR register may be not valid for the mode changes. */

#****************************************************
#*	The function for entering power down mode		*
#****************************************************
#void EnterPWDN(int CLKCON);
EnterPWDN:
    mov	    r2,r0               /* r0=CLKCON */
    ldr	    r0,=REFRESH		
    ldr	    r3,[r0]
    mov	    r1, r3
    orr	    r1, r1, #0x400000   /* self-refresh enable */
    str	    r1, [r0]

    nop     /* Wait until self-refresh is issued. May not be needed. */
    nop     /* If the other bus master holds the bus, ... */
    nop	    /* mov r0, r0 */
    nop
    nop
    nop
    nop

#enter POWERDN mode
    ldr	    r0,=CLKCON
    str	    r2,[r0]

#wait until enter SL_IDLE,STOP mode and until wake-up
    ldr	    r0,=0x10
U0: subs    r0,r0,#1
    bne	    U0

#exit from DRAM/SDRAM self refresh mode.
    ldr	    r0,=REFRESH
    str	    r3,[r0]
    mov	    pc,lr
    
    .ltorg

SMRDATA:
#*****************************************************************
#* Memory configuration has to be optimized for best performance *
#* The following parameter is not optimized.                     *
#*****************************************************************

#*** memory access cycle parameter strategy ***
# 1) Even FP-DRAM, EDO setting has more late fetch point by half-clock
# 2) The memory settings,here, are made the safe parameters even at 66Mhz.
# 3) FP-DRAM Parameters:tRCD=3 for tRAC, tcas=2 for pad delay, tcp=2 for bus load.
# 4) DRAM refresh rate is for 40Mhz. 

#bank0	16bit BOOT ROM
#bank1	NandFlash(8bit)/IDE/USB/rtl8019as/LCD
#bank2	No use 
#bank3	Keyboard 
#bank4	No use
#bank5	No use
#bank6	16bit SDRAM
#bank7	No use

.ifeq BUSWIDTH-16
	.long 0x11110102		/* Bank0=16bit BootRom(AT29C010A*2) :0x0 */
.else
   	.long 0x22222220		/* Bank0=OM[1:0], Bank1~Bank7=32bit 	 */
.endif
	.long ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))	/* GCS0 */
	.long ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))	/* GCS1 */
	.long ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))	/* GCS2 */
	.long ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))	/* GCS3 */
	.long ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))	/* GCS4 */
	.long ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))	/* GCS5 */
	.ifc "DRAM",BDRAMTYPE
	    .long ((B6_MT<<15)+(B6_Trcd<<4)+(B6_Tcas<<3)+(B6_Tcp<<2)+(B6_CAN))	/* GCS6 check the MT value in parameter.a */
	    .long ((B7_MT<<15)+(B7_Trcd<<4)+(B7_Tcas<<3)+(B7_Tcp<<2)+(B7_CAN))	/* GCS7                                   */
	.else
		.long ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))	/* GCS6 */
		.long ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))	/* GCS7 */
	.endif
	.long ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)	/* REFRESH RFEN=1, TREFMD=0, trp=3clk, trc=5clk, tchr=3clk,count=1019 */
	.long 0x10				/* SCLK power down mode, BANKSIZE 32M/32M */
	.long 0x20				/* MRSR6 CL=2clk                          */
	.long 0x20				/* MRSR7                                  */


.equ 	UserStack,	_ISR_STARTADDRESS-0xf00    		/* c7ff000 */   	
.equ	SVCStack,	_ISR_STARTADDRESS-0xf00+256    	/* c7ff100 */
.equ	UndefStack,	_ISR_STARTADDRESS-0xf00+256*2   /* c7ff200 */
.equ	AbortStack,	_ISR_STARTADDRESS-0xf00+256*3   /* c7ff300 */
.equ	IRQStack,	_ISR_STARTADDRESS-0xf00+256*4   /* c7ff400 */
.equ	FIQStack,	_ISR_STARTADDRESS-0xf00+256*5   /* c7ff500 */

.equ	HandleReset,	_ISR_STARTADDRESS
.equ	HandleUndef,	_ISR_STARTADDRESS+4
.equ	HandleSWI,		_ISR_STARTADDRESS+4*2
.equ	HandlePabort,	_ISR_STARTADDRESS+4*3
.equ	HandleDabort,	_ISR_STARTADDRESS+4*4
.equ	HandleReserved,	_ISR_STARTADDRESS+4*5
.equ	HandleIRQ,		_ISR_STARTADDRESS+4*6
.equ	HandleFIQ,		_ISR_STARTADDRESS+4*7

# Don't use the label 'IntVectorTable',
# because armasm.exe can not recognize this label correctly.
# the value is different with an address you think it may be.
# IntVectorTable
.equ	HandleADC,    	_ISR_STARTADDRESS+4*8
.equ	HandleRTC,		_ISR_STARTADDRESS+4*9
.equ	HandleUTXD1, 	_ISR_STARTADDRESS+4*10
.equ	HandleUTXD0,	_ISR_STARTADDRESS+4*11
.equ	HandleSIO,		_ISR_STARTADDRESS+4*12
.equ	HandleIIC,		_ISR_STARTADDRESS+4*13
.equ	HandleURXD1,	_ISR_STARTADDRESS+4*14
.equ	HandleURXD0,	_ISR_STARTADDRESS+4*15
.equ	HandleTIMER5,	_ISR_STARTADDRESS+4*16
.equ	HandleTIMER4,	_ISR_STARTADDRESS+4*17
.equ	HandleTIMER3,	_ISR_STARTADDRESS+4*18
.equ	HandleTIMER2,	_ISR_STARTADDRESS+4*19
.equ	HandleTIMER1,	_ISR_STARTADDRESS+4*20
.equ	HandleTIMER0,	_ISR_STARTADDRESS+4*21
.equ	HandleUERR01,	_ISR_STARTADDRESS+4*22
.equ	HandleWDT,		_ISR_STARTADDRESS+4*23
.equ	HandleBDMA1, 	_ISR_STARTADDRESS+4*24
.equ	HandleBDMA0,	_ISR_STARTADDRESS+4*25
.equ	HandleZDMA1, 	_ISR_STARTADDRESS+4*26
.equ	HandleZDMA0,	_ISR_STARTADDRESS+4*27
.equ	HandleTICK,		_ISR_STARTADDRESS+4*28
.equ	HandleEINT4567,	_ISR_STARTADDRESS+4*29
.equ	HandleEINT3,	_ISR_STARTADDRESS+4*30
.equ	HandleEINT2,	_ISR_STARTADDRESS+4*31
.equ	HandleEINT1,	_ISR_STARTADDRESS+4*32
.equ	HandleEINT0,	_ISR_STARTADDRESS+4*33		/* 0xc1(c7)fff84 */

.global sudoku_candidatos_propagar_arm
.global sudoku_candidatos_init_arm

################################################################################
.arm
sudoku_candidatos_init_arm:
    @ r0 = cuadricula
    @ r1 = fila
    @ r2 = columna
    @ r3 = valor actual
    @ r4 = desplazamiento fila
    @ r5 = desplazamiento columna
    @ r6 = 0x1FF0 lista de candidatos
    @ r7 = desplazamiento total

	MOV r9, r1						@ r9 = version propagar (C=0, ARM=1, THUMB=2)

    MOV r1, #0
    MOV r6, #0x1F00               	@ Cargar parte alta de (0x1F00)
    ORR r6, r6, #0x00F0           	@ Combinar parte baja (0x00F0) para obtener 0x1FF0

init_fila:
    CMP r1, #9
    BGE fin_init

    LSL r4, r1, #5					@ Desplazamiento a siguiente fila (r1 * 32 bytes)

    MOV r2, #0
init_columna:
    CMP r2, #9
    BGE fin_init_columna

    LSL r5, r2, #1					@ Desplazamiento a siguiente columna (r2 * 2 bytes)

    ADD r7, r4, r5        			@ Desplazamiento total (fila + columna)

    LDRH r3, [r0, r7]

   	AND r8, r3, #0x8000				@ Objeter valor de la pista
	CMP r8, #0x8000					@ Si pista == 1 se omite la inicializacion de candidatos
    BGE skip_init

    ORR r3, r3, r6        			@ celda |= 0x1FF0 - Activar todos los candidatos
    STRH r3, [r0, r7]

skip_init:
    ADD r2, r2, #1
    B init_columna

fin_init_columna:
    ADD r1, r1, #1
    B init_fila

fin_init:

@ Recorrer la cuadricula para llamar a las versiones de propagar

	MOV r10, #0							@ Contador de celdas vacias
	MOV r1, #0
fila_cuadricula:
	CMP r1, #9
	BGE fin_cuadricula

	LSL r4, r1, #5

	MOV r2, #0
recorrer_columna:
	CMP r2, #9
	BGE siguiente_fila

	LSL r6, r2, #1

	ADD r7, r4, r6

	LDRH r3, [r0, r7]

	AND r3, r3, #0xF				@celda_leer_valor()
	CMP r3, #0
	BEQ skip_propagation

	CMP r9, #0
	BEQ propagar_C

	CMP r9, #1
	BEQ propagar_ARM
skip_propagation:
	ADD r10, r10, #1
	B final_columna
propagar_C:
	STMED SP!, {r0-r12, r14}
	BL sudoku_candidatos_propagar_c
	LDMED SP!, {r0-r12, r14}
	B final_columna
propagar_ARM:
	STMED SP!, {r0-r12, r14}
	BL sudoku_candidatos_propagar_arm
	LDMED SP!, {r0-r12, r14}
	B final_columna
#propagar_THUMB:
#	BL sudoku_candidatos_propagar_arm
#	B final_columna
final_columna:
	ADD r2, r2, #1
	B recorrer_columna
siguiente_fila:
	ADD r1, r1, #1
	B fila_cuadricula
fin_cuadricula:
	MOV r0, r10
  	BX lr


@ Calcula el desplazamiento necesario para llegar a la celda, y llama a las funciones de propagacion (fila, columna y region)
sudoku_candidatos_propagar_arm:
	STMFD sp!, {r11, r12, lr}
	@ r0 = cuadricula
	@ r1 = indice fila
	@ r2 = indice columna
	@ r3 = valor actual celda
	@ r4 = desplazamiento de la fila
	@ r5 = desplazamiento de la columna
	@ r6 = bit desplazado (se usara para desactivar los candidatos)
	@ r7 = desplazamiento total (r4 + r5)

	LSL r4, r1, #5							@ Desplazamiento fila (r1 * 32 bytes)

	LSL r5, r2, #1							@ Desplazamiento columna (r2 * 2 bytes)

	ADD r7, r4, r5							@ Desplazamiento total (fila + columna)

	ADD r8, r3, #3							@ r8 = displace = (3 + r3)

	MOV r6, #1
	LSL r6, r6, r8							@ desplazar bit a la izquierda r8 posiciones

	LDRH r8, [r0, r7]

@ Actualiza los candidatos de la fila (verticalmente)
	MOV r9, #0
fila_loop:
    CMP r9, #9
    BGE fin_fila

    LSL r10, r9, #5               			@ Desplazar la fila (r9 * 32)
    ADD r11, r10, r5               			@ r11 = desplazamiento total = fila + columna (columna fija para r5)

    LDRH r8, [r0, r11]

    AND r12, r8, #0x8000
    CMP r12, #0x8000
    BEQ skip_row

    CMP r9, r1
    BEQ skip_row

    BIC r8, r8, r6               			@ celda &= ~(1 << displace) - Actualizar el bit candidato
    STRH r8, [r0, r11]
skip_row:
    ADD r9, r9, #1
    B fila_loop
fin_fila:
@ Actualiza los candidatos de la columna (Horizontalmente)
	MOV r9, #0
col_loop:
    CMP r9, #9
    BGE fin_col

    LSL r10, r9, #1               			@ Desplazar la columna (r9 * 2)
    ADD r11, r4, r10               			@ r11 = desplazamiento total = fila + columna (fila fija para r4)

    LDRH r8, [r0, r11]

    AND r12, r8, #0x8000
    CMP r12, #0x8000
    BEQ skip_column

    CMP r9, r2
    BEQ skip_column

    BIC r8, r8, r6               			@ celda &= ~(1 << displace) - Actualizar bit candidato
    STRH r8, [r0, r11]
skip_column:
    ADD r9, r9, #1
    B col_loop
fin_col:
@ Calcula el limite inferior (esquina superior izquierda) y limite superior de la region 3x3 correspondiente a la celda
	MOV r8, r1
	MOV r9, #0
row_index:
	CMP r8, #3
	BLT row_index_end						@ Si es menor a 3, termina el calculo de fila

	SUB r8, r8, #3							@ Restas sucesivas para determinar la region de la celda (1°,2° o 3° region vertical)
	ADD r9, r9, #3							@ Aumentar #3 para ubicarse al inicio de la region correspondiente
	B row_index
row_index_end:
	MOV r7, r9								@ Guardar el indice calculado en r8 (fila)

	MOV r8, r2
	MOV r9, #0
col_index:
	CMP r8, #3
	BLT col_index_end						@ Si es menor a 3, termina el calculo de columna

	SUB r8, r8, #3							@ Restas sucesivas para determinar la region de la celda (1°,2° o 3° region horizontal)
	ADD r9, r9, #3							@ Aumentar #3 para ubicarse al inicio de la region correspondiente
	B col_index
col_index_end:
	MOV r8, r9								@ Guardar el indice calculado en r7 (columna)

	ADD r9, r7, #3							@ r9 = Limite superior de la fila
	ADD r10, r8, #3							@ r10 = Limite superior de la columna
@ Actualiza los candidatos de la region 3x3
region_row_loop:
	CMP r7, r9
	BGE end_of_region
region_col_loop:
	CMP r8, r10
	BGE end_of_column

	LSL r4, r7, #5							@ Desplazamiento fila (r7 * 32)
	LSL r5, r8, #1							@ Desplazamiento columna (r10 * 2)
	ADD r11, r4, r5							@ Desplazamiento total de la celda (fila + columna)

	LDRH r12, [r0, r11]

	AND r4, r12, #0x8000
	CMP r4, #0x8000
	BEQ skip_cell

	CMP r7, r1
	BEQ skip_cell

	CMP r8, r2
	BEQ skip_cell

	BIC r12, r12, r6						@ celda &= ~(1 << displace) - Actualizar bit candidato
	STRH r12, [r0, r11]
skip_cell:
	ADD r8, r8, #1
	B region_col_loop
end_of_column:
	ADD r7, r7, #1
	SUB r8, r8, #3						@ Regresar a la primera columa de la región
	B region_row_loop
end_of_region:
	LDMFD sp!, {r11, r12, lr}
	BX lr

################################################################################
.data
.ltorg
.align 5    /* guarantees 32-byte alignment (2^5) */

# huecos para cuadrar
.global cuadricula

cuadricula:
     /* 9 filas de 16 entradas para facilitar la visualizacion, 16 bits por celda */
    .hword   0x8005,0x0000,0x0000,0x8003,0x0000,0x0000,0x0000,0x0000,0x0000,0, 0,0,0,0,0,0
    .hword   0x0000,0x0000,0x0000,0x0000,0x8009,0x0000,0x0000,0x0000,0x8005,0,0,0,0,0,0,0
    .hword   0x0000,0x8009,0x8006,0x8007,0x0000,0x8005,0x0000,0x8003,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x8008,0x0000,0x8009,0x0000,0x0000,0x8006,0x0000,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x0000,0x8005,0x8008,0x8006,0x8001,0x8004,0x0000,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x0000,0x8004,0x8002,0x0000,0x8003,0x0000,0x8007,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x8007,0x0000,0x8005,0x0000,0x8009,0x8002,0x8006,0x0000,0,0,0,0,0,0,0
    .hword   0x8006,0x0000,0x0000,0x0000,0x8008,0x0000,0x0000,0x0000,0x0000,0,0,0,0,0,0,0
    .hword   0x0000,0x0000,0x0000,0x0000,0x0000,0x8002,0x0000,0x0000,0x8001,0,0,0,0,0,0,0
.end

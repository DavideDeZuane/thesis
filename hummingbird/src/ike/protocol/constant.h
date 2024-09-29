#ifndef RFC_CONSTANTS_H
#define RFC_CONSTANTS_H

#define MAX_TRANSFORMS 4

#define MID_NULL 0x00000000
#define SPI_NULL 0x0000000000000000 //valore speciale del campo che indica che l'initiator non conosce l'SPI del responder
#define SPI_LENGTH 64

/*#######################################################
Flag, sono le maschere binarie per settare i flag
#######################################################*/
#define FLAG_R 0x20  // Bit R (0010 0000 in binario)
#define FLAG_V 0x10  // Bit V (0001 0000 in binario)
#define FLAG_I 0x08  // Bit I (0000 1000 in binario)

#endif
/*!
*  \file       cmbs_uart.h
*  \brief	   
*  \author     stein
*
*  @(#)  %filespec: cfr_uart.h-1 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*******************************************************************************
*  COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*  DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if   !defined( CFR_UART_H )
#define  CFR_UART_H

/*! current packet transmission size */
#define     CFR_BUFFER_WINDOW_SIZE  3

/*! identifier for receive path */
#define		CFR_BUFFER_UART_REC		0
/*! identifier for transmit path */
#define		CFR_BUFFER_UART_TRANS	1

#if defined( __cplusplus )
extern "C"
{
#endif
void  *      cfr_uartThread( void * pVoid );
int          cfr_uartInitialze( PST_UART_CONFIG pst_Config );
int			 cfr_uartPacketPartWrite( u8* pu8_Buffer, u16 u16_Size );
void         cfr_uartPacketWriteFinish( u8 u8_BufferIDX );
CFR_E_RETVAL cfr_uartPacketPrepare ( u16 u16_size );
void			 cfr_uartDataTransmitKick ( void );


#if defined( __cplusplus )
}
#endif

#endif   // CFR_UART_H
//*/

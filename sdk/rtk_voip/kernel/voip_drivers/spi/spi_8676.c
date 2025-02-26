/*****************************************************************************
*	SPI driver shared with SPI flash	
*
*	Initial version: 			2008/04/15
*
*
*****************************************************************************/
//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "rtk_voip.h"
#include "gpio/gpio.h"
#include "spi.h"
//#ifdef CONFIG_RLE0412
#include <bspchip.h>
//#endif

static inline void printk_off( const char *fmt, ... ) {}
#define SPI_PRINTK		printk_off
//#define SPI_PRINTK		printk

//#define FEATURE_COP3_PROFILE 1
#ifdef FEATURE_COP3_PROFILE
#include "cp3/cp3_profile.h"
extern st_CP3_VoIP_param cp3_voip_param;
#endif

// gpio simulated spi interface
#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO

/************************************* Set GPIO Pin to SPI ***********************************************************/
/*
@func int32 | __rtl867x_spi_init | Initialize SPI device
@parm rtl_spi_dev_t* | pDev | Structure to store device information
@parm uint32 | gpioSCLK | GPIO ID of SCLK
@parm uint32 | gpioCS_ | GPIO ID of CS_
@parm uint32 | gpioSDI | GPIO ID of SDI
@parm uint32 | gpioSDO | GPIO ID of SDO
@parm uint32 | maxSpeed | how fast SPI driver can generate the SCLK signal (unit: HZ)
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI)
{
	
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;

	_rtl867x_initGpioPin( gpioSCLK, GPIO_DIR_OUT);
	_rtl867x_initGpioPin( gpioCS_, GPIO_DIR_OUT);
	_rtl867x_initGpioPin( gpioSDI, GPIO_DIR_IN);
	_rtl867x_initGpioPin( gpioSDO, GPIO_DIR_OUT);
	
	return SUCCESS;
}

/*
@func int32 | _rtl867x_spi_exit | Called when a SPI device is released
@parm rtl_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 __rtl867x_spi_exit( rtl_spi_dev_t* pDev )
{
	return SUCCESS;
}
 
#if speed_booting_rating
#define BUSY_LOOP 50000
#else
#define BUSY_LOOP 80000000 //~ 500 msec
#endif

void RST_Slic(void)
{
#ifndef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	int i;
#if defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
	uint32 pin_reset = PIN_RESET2;
#else
	uint32 pin_reset = PIN_RESET1;
#endif

	_rtl867x_initGpioPin(pin_reset, GPIO_DIR_OUT);
	for(i=0;i<BUSY_LOOP;i++);

	_rtl867x_setGpioDataBit(pin_reset, 1);	// reset high
	for(i=0;i<BUSY_LOOP;i++);

	_rtl867x_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
	for(i=0;i<BUSY_LOOP;i++);

	_rtl867x_setGpioDataBit(pin_reset, 1);	// release reset
	for(i=0;i<BUSY_LOOP;i++);
#endif
}

void _rtl867x_init_spi_channels(int size, rtl_spi_dev_t* pDev[], uint32 pin_cs[], uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
	int i;

	if (GPIO_PORT(pin_reset) < GPIO_PORT_MAX)
		_rtl867x_initGpioPin(pin_reset, GPIO_DIR_OUT);

	for( i = 0; i < size; i ++ ) {
		_rtl867x_initGpioPin(pin_cs[i], GPIO_DIR_OUT);
		__rtl867x_spi_init(pDev[i], pin_clk, pin_cs[i], pin_do, pin_di);
	}
	
	for(i=0;i<BUSY_LOOP;i++);

	for( i = 0; i < size; i ++ )
		_rtl867x_setGpioDataBit(pin_cs[i], 1); 	/* CS_ preset to high state*/

	_rtl867x_setGpioDataBit(pin_clk, 1); 	/* SCLK preset to high state*/

	if (GPIO_PORT(pin_reset) < GPIO_PORT_MAX)	
	_rtl867x_setGpioDataBit(pin_reset, 1);	// reset high
	for(i=0;i<BUSY_LOOP;i++);

	if (GPIO_PORT(pin_reset) < GPIO_PORT_MAX)
	_rtl867x_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
	for(i=0;i<BUSY_LOOP;i++);

	if (GPIO_PORT(pin_reset) < GPIO_PORT_MAX)
	_rtl867x_setGpioDataBit(pin_reset, 1);	// release reset
	for(i=0;i<BUSY_LOOP;i++);
}

#if 0
static void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
	int i;
	static unsigned char inital = 0;

	_rtl867x_initGpioPin(pin_reset, GPIO_DIR_OUT);
	_rtl867x_initGpioPin(pin_cs, GPIO_DIR_OUT);

	__rtl867x_spi_init(&spi_dev[channel], pin_clk, pin_cs, pin_do, pin_di);

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
	static int delay_cnt = 50000;
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
	static int delay_cnt = 100000;
#endif
	
	if ((channel == 0) || (inital == 0)) 
	{
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif
		_rtl867x_setGpioDataBit(pin_cs, 1); 	/* CS_ preset to high state*/
		_rtl867x_setGpioDataBit(pin_clk, 1); 	/* SCLK to high state*/
		
		_rtl867x_setGpioDataBit(pin_reset, 1);	// Reset high
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif
		_rtl867x_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif
		_rtl867x_setGpioDataBit(pin_reset, 1);	// release reset
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif				// wait more than 100ms 
			inital++;

	}

}

//------------------------------------------
void init_spi(int ch_spi)
{
	
	SPI_PRINTK("( GPIO %s )  ", GPIO );
	
	if (ch_spi == 0)
	{
		SPI_PRINTK("for SLIC[%d]...", ch_spi);
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		SPI_PRINTK("=> OK !\n");
	}
	else if (ch_spi == 1)
	{
		SPI_PRINTK("for SLIC/DAA[%d]...", ch_spi);
		init_channel(1, PIN_CS3_DAA, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA);
		SPI_PRINTK("=> OK !\n");
	}
	else 
	{
		SPI_PRINTK("No GPIO Pin assign for any device -> Can't initialize any device\n");
		
	}
}
#endif

#endif //CONFIG_RTK_VOIP_8676_SPI_GPIO



// 8676 HW SPI
#ifdef CONFIG_RTK_VOIP_8676_SHARED_SPI

//#define SHARE_SPI
#define SPI_VOIP_MIN_DELAY 10000

extern spinlock_t spi_lock;
/*---------------- External Function Prototypes -----------------*/
extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);

void dumpSPIREG(void)
{
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	SPI_PRINTK("   SPICNR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x00 , spi_reg->SPICNR);	
	SPI_PRINTK("   SPISTR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x04 , spi_reg->SPISTR);	
	SPI_PRINTK(" SPICKDIV:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x08 , spi_reg->SPICKDIV);
	SPI_PRINTK("   SPIRDR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x0c , spi_reg->SPIRDR);
	SPI_PRINTK("   SPITCR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x10 , spi_reg->SPITCR);	
	SPI_PRINTK("SPICDTCR0:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x14 , spi_reg->SPICDTCR0);
	SPI_PRINTK("SPICDTCR1:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x18 , spi_reg->SPICDTCR1);		
	SPI_PRINTK(" SPITCALR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x1c , spi_reg->SPITCALR);
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB

#if defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x)

void rtl8676_share_spi_write(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned int data)
{
	unsigned int		spicnr;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;
	
	spin_lock_irqsave(spi_lock,flags);

	//trans. conf
	//	spi_reg->SPITCR = 0x80e0010c;


	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000; //MSIF LOCK ?

	//ctrl 
	spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | address<<16 | control<<8 | SPI_LSB(0) | SPI_CMD(1) | SPI_START(1) | SPI_CSP(0) ;

	//data
	spi_reg->SPIRDR = (data << 24);

	//start transfer
	spi_reg->SPICNR = spicnr;
	
	//wait unitl finish
	while ((spi_reg->SPISTR & SPI_WDIP(1)) == 0) ;

	spin_unlock_irqrestore(spi_lock,flags);

	return;

}
/*---------------------------------------------------------
*	Name:
*		reset_spi
*	Description:
*		Initialize the corresponding spi channel.
*	Input:	
		ch_spi - the spi channel to setup
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
unsigned int  rtl8676_share_spi_read(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned char *data)
{
	unsigned int		spicnr;
	uint8 buf;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();

	int flags;
	
	spin_lock_irqsave(spi_lock,flags);

	//trans. conf
	//spi_reg->SPITCR = 0x80e0010c; 

	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;

	//ctrl
	spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | address<<16 | control<<8 | SPI_LSB(0) | SPI_CMD(0) | SPI_START(1) | SPI_CSP(0);

	//start transfer
	spi_reg->SPICNR = spicnr;

	//wait unitl finish
	while ((spi_reg->SPISTR & SPI_RDIP(1)) == 0) ;
	buf = (unsigned char) (spi_reg->SPIRDR >> 24);
	*data = buf;

	spin_unlock_irqrestore(spi_lock,flags);
	return buf; 	

}


/*---------------------------------------------------------
*	Name:
*		init_spi
*	Description:
*		Initialize the corresponding spi channel.
*	Input:	
		ch_spi - the spi channel to setup
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
void __rtl867x_spi_init(int ch_spi)
{
	rtl_8676_reg_t	*spi_reg;
	int flags;	

	if (ch_spi !=0)
		return;

	spin_lock_irqsave(spi_lock,flags);

	*(volatile int*) BSP_IP_SEL |= BSP_EN_NEW_VOIP_SPI;
	*(volatile int*) BSP_IP_SEL &= ~(BSP_EN_XSI_VOIP_SPI);
	SPI_PRINTK("BSP_IP_SEL:[%08x]=%08x\n", BSP_IP_SEL, *(volatile int*) BSP_IP_SEL );

	spi_reg = get_spi_reg();
	
	/*Initialize Registers*/	
	spi_reg->SPICKDIV = 0x40000000;
	spi_reg->SPITCR = SPI_SEL_CS(0) | SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x5045; //0x80e05045
	//SPI_PRINTK("SPITCR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x10 , spi_reg->SPITCR);

	spin_unlock_irqrestore(spi_lock,flags);


}
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x || CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)

#if 0
#define ISI_LOOP 10
static void rtl8676_check_msif_lock(rtl_8676_reg_t *reg) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	int count = 0;	
	while ((reg->SPISTR & 0x3f000000)==0 && (count < ISI_LOOP)) {
		count ++;
		//SPI_PRINTK("MSIF_LOCK=0 %x\n", ++count);
		SPI_PRINTK("reg->SPISTR = 0x[%x]\n", reg->SPISTR);
	}
#endif	
}
#else
static void rtl8676_check_msif_lock(rtl_8676_reg_t *reg) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	int count = 0;	
	while ((reg->SPISTR & 0x3f000000)==0) {
		SPI_PRINTK("MSIF_LOCK=0 %x\n", ++count);
	}
#endif	
}
#endif

#if 0
void rtl8676_share_spi_write(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned int data)
{
	//unsigned int		spicnr;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;

	//rtl8676_check_msif_lock(spi_reg);
	spin_lock_irqsave(spi_lock,flags);

	spi_reg->SPITCR |= SPI_SEL_CS(0);


	spi_reg->SPITCR &= ~(SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1)| SPI_D1_EN(1)); 
	spi_reg->SPITCR |= SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x1823; 
	
	//data
	spi_reg->SPIRDR = (data << 24);

	rtl8676_check_msif_lock(spi_reg);
	//trans. conf
	//spi_reg->SPITCR =0x80e0110e;
		
	
	//clear all interrupt
	//spi_reg->SPISTR = 0xC0000000; //MSIF LOCK ?

	//ctrl
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	spi_reg->SPICNR = SPI_CSTIME(0) | ((address<<16) &0x00FF0000 ) | ((control<<8) & 0x0000FF00) | 
		SPI_LSB(0) | SPI_CMD(1) | SPI_SCLK_TYPE(1)| SPI_CSP(0) ;
#else
	spi_reg->SPICNR = SPI_CSTIME(0) | ((address<<16) &0x00FF0000 ) | ((control<<8) & 0x0000FF00) | 
		SPI_LSB(0) | SPI_CMD(1) | SPI_SCLK_TYPE(0)| SPI_CSP(0) ;
#endif


	//start transfer
	//spi_reg->SPICNR = spicnr;

	spi_reg->SPICNR |= SPI_START(1);

	//wait unitl finish
	//while ( (spi_reg->SPISTR & 0xC0000000)  == 0) ;
	
	//while ( (spi_reg->SPISTR & SPI_WDIP(1))  == 0) ;
	while ( spi_reg->SPICNR & SPI_START(1) );

	rtl8676_check_msif_lock(spi_reg);

	spi_reg->SPITCR &= ~(SPI_SEL_CS(0)); 

	SPI_PRINTK("[%s]:%x %x %x = %x\n", __FUNCTION__, control,address,spi_reg->SPICNR,data);
	
	spin_unlock_irqrestore(spi_lock,flags);

	return;

}

unsigned int  rtl8676_share_spi_read(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned char *data)
{
	//unsigned int		spicnr;
	uint8 buf;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();

	int flags;
	
	
	spin_lock_irqsave(spi_lock,flags);

	spi_reg->SPITCR = SPI_SEL_CS(0);

	//cs_select(ch);

	//trans. conf
	//spi_reg->SPITCR =0x80e0110e;
	spi_reg->SPITCR &= ~(SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | SPI_D1_EN(1));
	spi_reg->SPITCR = SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x1823; 	

	spi_reg->SPIRDR = 0xDE << 24;
	
	rtl8676_check_msif_lock(spi_reg);

	//clear all interrupt
	//spi_reg->SPISTR = 0xC0000000;

	//ctrl
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	spi_reg->SPICNR = SPI_CSTIME(0) |  ((address<<16) &0x00FF0000 ) | ((control<<8) & 0x0000FF00) | 
		SPI_LSB(0) | SPI_CMD(0) | SPI_SCLK_TYPE(1) | SPI_CSP(0);
#else
	spi_reg->SPICNR = SPI_CSTIME(0) |  ((address<<16) &0x00FF0000 ) | ((control<<8) & 0x0000FF00) | 
		SPI_LSB(0) | SPI_CMD(0) | SPI_SCLK_TYPE(0) | SPI_CSP(0);
#endif

	//start transfer
	//spi_reg->SPICNR = spicnr;
	spi_reg->SPICNR |= SPI_START(1);

	//wait unitl finish
//	while ( (spi_reg->SPISTR & 0xC0000000)  == 0) ;
//	while ( (spi_reg->SPISTR & SPI_RDIP(1))  == 0) ;	
	while ( spi_reg->SPICNR & SPI_START(1) );

	rtl8676_check_msif_lock(spi_reg);

	buf = (unsigned char) (spi_reg->SPIRDR >> 24);
	*data = buf;

	SPI_PRINTK("[%s]:%x %x %x = %x\n", __FUNCTION__, control,address,spi_reg->SPICNR,buf);

	spi_reg->SPITCR &= ~(SPI_SEL_CS(0));
	
	spin_unlock_irqrestore(spi_lock,flags);
	return buf; 	

}
#else

void rtl8676_share_spi_write(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned int data)
{
	unsigned int		spicnr;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;

	rtl8676_check_msif_lock(spi_reg);
	spin_lock_irqsave(spi_lock,flags);

	//cs_select(ch);
	
	//trans. conf
	spi_reg->SPITCR = SPI_SEL_CS(0) | SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x1823; 
	
	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000; //MSIF LOCK ?

	//ctrl
	spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | address<<16 | control<<8 | SPI_LSB(0) | SPI_CMD(1) | SPI_START(1) | SPI_SCLK_TYPE(1)| SPI_CSP(0) ;

	//data
	spi_reg->SPIRDR = (data << 24);

	//start transfer
	spi_reg->SPICNR = spicnr;

	//wait unitl finish
	while ( (spi_reg->SPISTR & SPI_WDIP(1))  == 0) ;

	spi_reg->SPITCR = SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x1823; 
	
	spin_unlock_irqrestore(spi_lock,flags);

	return;

}

unsigned int  rtl8676_share_spi_read(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned char *data)
{
	unsigned int		spicnr;
	uint8 buf;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();

	int flags;
	
	rtl8676_check_msif_lock(spi_reg);
	spin_lock_irqsave(spi_lock,flags);

	//cs_select(ch);

	//trans. conf
	spi_reg->SPITCR = SPI_SEL_CS(0) | SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x1823; 

	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;

	//ctrl
	spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | address<<16 | control<<8 | SPI_LSB(0) | SPI_CMD(0) | SPI_START(1) | SPI_SCLK_TYPE(1) | SPI_CSP(0);
	
	//start transfer
	spi_reg->SPICNR = spicnr;

	//wait unitl finish
	while ( (spi_reg->SPISTR & SPI_RDIP(1))  == 0) ;

	buf = (unsigned char) (spi_reg->SPIRDR >> 24);
	*data = buf;

	//SPI_PRINTK("spir:%x %x %x = %x\n",control,address,spicnr,buf);

	spi_reg->SPITCR = SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x1823;
	
	spin_unlock_irqrestore(spi_lock,flags);
	return buf; 	

}

#endif

void __rtl867x_spi_init(int ch_spi)

{
	rtl_8676_reg_t	*spi_reg;
	int flags;	

	if (ch_spi !=0)
		return;

	spin_lock_irqsave(spi_lock,flags);

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	*(volatile int*) BSP_IP_SEL |= BSP_EN_XSI_VOIP_SPI;
	*(volatile int*) BSP_IP_SEL |= BSP_EN_ISI_VOIP_SPI;
	//EG32(BSP_MISC_CR_BASE) &= 0xfffffff5;
	REG32(BSP_MISC_CR_BASE) |= (1<<4) | (1<<3) | (1<<1);
	SPI_PRINTK("BSP_IP_SEL:[%08x]=%08x\n", BSP_IP_SEL, *(volatile int*) BSP_IP_SEL );
	SPI_PRINTK("BSP_MISC_CR_BASE:[%08x]=%08x\n", BSP_MISC_CR_BASE, *(volatile int*) BSP_MISC_CR_BASE );
#endif

	spi_reg = get_spi_reg();
	/*Initialize Registers*/	
	spi_reg->SPICKDIV = 0x26000000;
//	spi_reg->SPITCALR = 0x08272700; // ISI
	spi_reg->SPITCALR = 0x082a2a00; // ISI	
	//spi_reg->SPITCR = SPI_SEL_CS(0) | SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x110e; //0x80e0110e
	spi_reg->SPITCR = /*SPI_SEL_CS(0) |*/ SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | 0x1823;
	SPI_PRINTK("SPICKDIV:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x08 , spi_reg->SPICKDIV);
	SPI_PRINTK("SPITCALR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x1c , spi_reg->SPITCALR);
	SPI_PRINTK("SPITCR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x10 , spi_reg->SPITCR);

	dumpSPIREG();
	spin_unlock_irqrestore(spi_lock,flags);

}

#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226


/*---------------------------------------------------------
*
*--------------------------------------------------------*/
static unsigned char share_spi_read_nodaisy(unsigned int reg, unsigned char *data)
{
	//unsigned int		val,i,spicnr;
	//uint8 buf;
	//rtl_8676_reg_t	*spi_reg=get_spi_reg();

	return rtl8676_share_spi_read(0, 0, (reg | 0x80), 0, data );
}


static void share_spi_write_nodaisy(unsigned int reg, unsigned int data)
{
	//unsigned int		val,i;
	//uint8 buf;
	//rtl_8676_reg_t	*spi_reg=get_spi_reg();

	rtl8676_share_spi_write(0, 0, reg, 0, data);
}

unsigned char read_spi_nodaisy_hw(unsigned int reg)
{
	unsigned char buf;
	share_spi_read_nodaisy(reg, &buf);
	return buf;
}


void write_spi_nodaisy_hw(unsigned int reg, unsigned int data)
{
	share_spi_write_nodaisy(reg, data);
}

/*---------------------------------------------------------
*	Name:
*		read_spi
*	Description:
*		read the spi data from specified channel id. (for Si3215/3210)
*	Input: 
*		chid - channel id (daisy_chain index)
*		reg - reg number
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
 unsigned char read_spi_hw(unsigned int chid, unsigned int reg)
{
	uint8 buf;
	//unsigned long flags, i;
	//volatile rtl_8676_reg_t	*spi_reg=get_spi_reg();

	rtl8676_share_spi_read(0, 1, (chid+1) , (reg | 0x80), &buf );
	return buf;
}

/*---------------------------------------------------------
*	Name:
*		write_spi
*	Description:
*		write the spi data to specified channel id. (for Si3215/3210)
*	Input: 
*		chid - 
*		reg -
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
 void write_spi_hw(unsigned int chid, unsigned int reg, unsigned int data)
{
	//unsigned long flags;
	//int i;
	//rtl_8676_reg_t	*spi_reg=get_spi_reg();

	rtl8676_share_spi_write(0, 1, (chid+1)  , reg,  data);
	return;
}

/*---------------------------------------------------------
*	Name:
*		read_spi_daa
*	Description:
*
*	Input: 
*		chid - 
*		reg -
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
unsigned char read_spi_hw_daa(int chid, unsigned int reg)
{
	uint8 buf;
	//unsigned long flags;
	//rtl_8676_reg_t	*spi_reg=get_spi_reg();

	rtl8676_share_spi_read(1, 1, 0x60, reg , &buf);
	return buf;
}

void write_spi_hw_daa(int chid, unsigned int reg, unsigned int data)
{
	//unsigned long flags;
	//rtl_8676_reg_t	*spi_reg = get_spi_reg();

	rtl8676_share_spi_write(1, 1, 0x20  , reg,  data);
	return;
}

#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK 

#if defined ( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116 ) || defined ( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316 )

#define spi_cs_delay() __ndelay(2500)	//chip select off time MIN 2500ns

void vp_write(rtl_spi_dev_t *pDev, unsigned int data)
{
	unsigned int		spicnr;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;

	//spin_lock_irqsave(spi_lock,flags);

	//trans. conf
	//spi_reg->SPITCR = 0x00205045 | (1<<(31 - pDev->SPI_SEL_CS));
	spi_reg->SPITCR &= ~((0x3f) << 26); //clear chip selection
//#if (defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116) &&  defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316))	|| (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR == 2)
#ifdef CONFIG_RTK_VOIP_GPIO_8676PN_IAD_2LAYER_DEMO_BOARD_V01
	_rtl867x_setGpioDataBit(PIN_CSEN, pDev->SPI_SEL_CS);
	spi_reg->SPITCR |= SPI_SEL_CS( 0);
#else
	spi_reg->SPITCR |= SPI_SEL_CS( pDev->SPI_SEL_CS );
#endif
	
	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;

	//ctrl
	//spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | SPI_LSB(0) | SPI_CMD(1) | SPI_START(1) | SPI_CSP(0);
	spicnr = SPI_CSTIME(0) | SPI_WRIE(1)/*| SPI_RDIE(1)*/| SPI_LSB(0) | SPI_CMD(1) | SPI_START(1) | SPI_CSP(0);

	//data
	spi_reg->SPIRDR = (data) << 24;

	//start transfer
	spi_reg->SPICNR = spicnr;

#ifdef FEATURE_COP3_PROFILE
	if (cp3_voip_param.bCp3Count_Temp200 == 1) {
		save_flags(flags); cli();
		ProfileEnterPoint(PROFILE_INDEX_TEMP200);
	}
#endif
	
	//wait unitl finish
	//while (spi_reg->SPISTR  == 0) ;
	while ((spi_reg->SPISTR & SPI_WDIP(1))  == 0) ;
	spi_reg->SPISTR |= SPI_WDIP(1);
	while ((spi_reg->SPICNR & SPI_START(1)) != 0) ;

	//spi_cs_delay();
	
#ifdef FEATURE_COP3_PROFILE
	if (cp3_voip_param.bCp3Count_Temp200 == 1) {
		ProfileExitPoint(PROFILE_INDEX_TEMP200);
		restore_flags(flags);
		ProfilePerDump(PROFILE_INDEX_TEMP200, cp3_voip_param.cp3_dump_period);
	}
#endif

	//spin_unlock_irqrestore(spi_lock,flags);
}

void vp_read(rtl_spi_dev_t *pDev, unsigned char *data)
{
	unsigned int		spicnr;
	uint8 buf;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;
	
	//spin_lock_irqsave(spi_lock,flags);

#ifdef FEATURE_COP3_PROFILE
	if (cp3_voip_param.bCp3Count_Temp201 == 1) {
		save_flags(flags); cli();
		ProfileEnterPoint(PROFILE_INDEX_TEMP201);
	}
#endif

	//trans. conf
	//spi_reg->SPITCR = 0x00205045 | (1<<(31 - pDev->SPI_SEL_CS));
	spi_reg->SPITCR &= ~((0x3f) << 26); //clear chip selection
//#if (defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116) && defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316)) || (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR == 2)
#ifdef CONFIG_RTK_VOIP_GPIO_8676PN_IAD_2LAYER_DEMO_BOARD_V01
	_rtl867x_setGpioDataBit(PIN_CSEN, pDev->SPI_SEL_CS);
	spi_reg->SPITCR |= SPI_SEL_CS( 0);
#else
	spi_reg->SPITCR |= SPI_SEL_CS( pDev->SPI_SEL_CS );
#endif
	
	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;

	//ctrl
	//spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | SPI_LSB(0) | SPI_CMD(0) | SPI_START(1) | SPI_CSP(0);
	spicnr = SPI_CSTIME(0)/*| SPI_WRIE(1)*/| SPI_RDIE(1) | SPI_LSB(0) | SPI_CMD(0) | SPI_START(1) | SPI_CSP(0);

	//clear data
	spi_reg->SPIRDR = 0x0;

	//start transfer
	spi_reg->SPICNR = spicnr;
	
	//wait unitl finish
	//while (spi_reg->SPISTR  == 0) ;
	while ((spi_reg->SPISTR & SPI_RDIP(1))  == 0) ;
	spi_reg->SPISTR |= SPI_RDIP(1);
	while ((spi_reg->SPICNR & SPI_START(1)) != 0) ;

	buf = (unsigned char) (spi_reg->SPIRDR >> 24);
	*data = buf;

	//spi_cs_delay();
	
	//spin_unlock_irqrestore(spi_lock,flags);

#ifdef FEATURE_COP3_PROFILE
	if (cp3_voip_param.bCp3Count_Temp201 == 1) {
		ProfileExitPoint(PROFILE_INDEX_TEMP201);
		restore_flags(flags);
		ProfilePerDump(PROFILE_INDEX_TEMP201, cp3_voip_param.cp3_dump_period);
	}
#endif
	
}

int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 SPI_SEL_CS)
{

	rtl_8676_reg_t	*spi_reg;
	int flags;
	spin_lock_irqsave(spi_lock,flags);

	//zarlink mpi setting 
	*(volatile int*) BSP_IP_SEL |= BSP_EN_NEW_VOIP_SPI;
	*(volatile int*) BSP_IP_SEL &= ~(BSP_EN_XSI_VOIP_SPI);
	SPI_PRINTK("BSP_IP_SEL:[%08x]=%08x\n", BSP_IP_SEL, *(volatile int*) BSP_IP_SEL );

	spi_reg = get_spi_reg();
	/*Initialize Registers*/	

#if 1
	spi_reg->SPICKDIV = 0x40000000; // ~960K
	spi_reg->SPITCR = SPI_D0_EN(1) | 0x0728;

	//spi_reg->SPICKDIV = 0x07000000; // 7.8M
	//spi_reg->SPITCR = SPI_D0_EN(1) | 0x0955;
#else
	spi_reg->SPICKDIV = 0x1E000000; // ~ 2M
	//spi_reg->SPICKDIV = 0x13000000; // ~ 3.125M
	//spi_reg->SPICKDIV = 0xF000000; // ~ 4M
	//spi_reg->SPICKDIV = 0x9000000; // ~ 6.25M
	//spi_reg->SPICKDIV = 0x7000000; // ~ 8M
	spi_reg->SPITCR = SPI_D0_EN(1) | 0x0728;
#endif
	//SPI_PRINTK("SPITCR:[%08x]=%08x\n", RTL8676_SPI_BASE + 0x10 , spi_reg->SPITCR);
//#if (defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116) && defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316)) || (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR == 2)
#ifdef CONFIG_RTK_VOIP_GPIO_8676PN_IAD_2LAYER_DEMO_BOARD_V01
	_rtl867x_initGpioPin( PIN_CSEN, GPIO_DIR_OUT);
#endif
	//chip select
	pDev->SPI_SEL_CS = SPI_SEL_CS;

	spin_unlock_irqrestore(spi_lock,flags);
	
	return SUCCESS;
}
#endif // CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116 ||  CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221

void vp_write(rtl_spi_dev_t *pDev, unsigned int data)
{
	unsigned int		i,spicnr;
	//uint8 buf;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;
//	SPI_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); //2011-01-21 paula
	//spi_rw_delay();
	
	spin_lock_irqsave(spi_lock,flags);
	//spi_reg->SPITCR = 0x802050e0;
	//spi_reg->SPITCR = 0x002050e0 | (1<<(31 - pDev->SPI_CS_SEL)); 

	//trans. conf
	//spi_reg->SPITCR = 0x00801007 | (1<<(31 - pDev->SPI_SEL_CS)); 
	spi_reg->SPITCR = 0x00801007 | SPI_SEL_CS( pDev->SPI_SEL_CS );
	//spi_reg->SPITCR = 0x80801007; 
	//SPI_PRINTK("\nwrite\n");
	//SPI_PRINTK("SPITCR: %x\n",spi_reg->SPITCR);
	//mdelay(1);
	
	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;

	//ctrl
	spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | SPI_LSB(0) | SPI_CMD(1) | SPI_START(1) | SPI_CSP(0) | data<<8;
	//spi_reg->SPIRDR = (data)<<24;
	
	//SPI_PRINTK("SPIRDR: %x\n",spi_reg->SPIRDR);

	//start transfer
	spi_reg->SPICNR = spicnr;
	
	//SPI_PRINTK("SPICNR: %x\n",spi_reg->SPICNR);
	//wait unitl finish
	//for(i=0;i<SPI_VOIP_MIN_DELAY;i++);
	//mdelay(1);
	while (spi_reg->SPISTR  == 0) ;
	//SPI_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); //2011-01-21 paula
	spin_unlock_irqrestore(spi_lock,flags);
}

void vp_read(rtl_spi_dev_t *pDev, unsigned char *data/*, unsigned int cmd*/)
{
	unsigned int		i,spicnr;
	uint8 buf;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;

	//spi_rw_delay();
	//SPI_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); //2011-01-21 paula
	//dump_stack();
	spin_lock_irqsave(spi_lock,flags);
	//spi_reg->SPITCR = 0x802050e0;
	//spi_reg->SPITCR = 0x002050e0 | (1<<(31 - pDev->SPI_CS_SEL));

	//if(cmd !=0)
	//spi_reg->SPITCR = 0x80801007; 
	//else 

	//trans. conf
	//spi_reg->SPITCR = 0x00201007 | (1<<(31 - pDev->SPI_SEL_CS)); 
	spi_reg->SPITCR = 0x00201007 |  SPI_SEL_CS( pDev->SPI_SEL_CS );
	//spi_reg->SPITCR = 0x80201007; 

	//SPI_PRINTK("\nread\n");
	//SPI_PRINTK("SPITCR: %x\n",spi_reg->SPITCR);

	//mdelay(1);
	
	//SPI_PRINTK("%x\n",spi_reg->SPITCR);

	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;

	//ctrl
	spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | SPI_LSB(0) | SPI_CMD(0) | SPI_START(1) | SPI_CSP(0);

	//clear data
	spi_reg->SPIRDR=0x0;
	//SPI_PRINTK("SPIRDR: %x\n",spi_reg->SPIRDR);

	//start transfer
	spi_reg->SPICNR = spicnr;
	//SPI_PRINTK("SPICNR: %x\n",spi_reg->SPICNR);
	
	//wait unitl finish
	//for(i=0;i<SPI_VOIP_MIN_DELAY;i++);

	//mdelay(1);
	
	while (spi_reg->SPISTR  == 0) ;
	buf = (unsigned char) ((spi_reg->SPIRDR & 0xff000000) >> 24);
	//SPI_PRINTK("spi_reg->SPIRDR: %x\n", spi_reg->SPIRDR);
	*data = buf;
	//SPI_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); //2011-01-21 paula
	spin_unlock_irqrestore(spi_lock,flags);
}

int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 SPI_SEL_CS)
{

	rtl_8676_reg_t	*spi_reg;
	int flags;
	spin_lock_irqsave(spi_lock,flags);

	*(volatile int*) BSP_IP_SEL |= BSP_EN_XSI_VOIP_SPI;
	*(volatile int*) BSP_IP_SEL &= ~(BSP_EN_ISI_VOIP_SPI);
	SPI_PRINTK("BSP_IP_SEL:[%08x]=%08x\n", BSP_IP_SEL, *(volatile int*) BSP_IP_SEL );
	
	/*Initialize Registers*/	
	spi_reg = get_spi_reg();
	//spi_reg->SPICKDIV = 0x0c000000;
	spi_reg->SPICKDIV = 0x01000000;
	spi_reg->SPITCALR = 0x01010000;
	
	//zarlink zsi setting 2011-03-25 paula
	//*((volatile unsigned int *)(0xb800330c)) |= (1 << 17); //zarlink mpi
	//*((volatile unsigned int *)(0xb800330c)) |= (1 << 18); //zsi
	//SPI_PRINTK("%x\n", *(volatile unsigned int *)(0xb800330c));

	//chip select
	pDev->SPI_SEL_CS = SPI_SEL_CS;
	//pDev->gpioSCLK = gpioSCLK;
	//pDev->gpioCS_ = gpioCS_;
	//pDev->gpioSDI = gpioSDI;
	//pDev->gpioSDO = gpioSDO;

	//_rtl867x_initGpioPin( gpioSCLK, GPIO_DIR_OUT);
	//_rtl867x_initGpioPin( gpioCS_, GPIO_DIR_OUT);
	//_rtl867x_initGpioPin( gpioSDI, GPIO_DIR_IN);
	//_rtl867x_initGpioPin( gpioSDO, GPIO_DIR_OUT);

	spin_unlock_irqrestore(spi_lock,flags);
	
	return SUCCESS;
}


#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9662) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601)
void vp_write(rtl_spi_dev_t *pDev, unsigned int data)
{
	unsigned int		i,spicnr;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;

	spin_lock_irqsave(&spi_lock,flags);

	//trans. conf
	spi_reg->SPITCR = 0x00201823 | SPI_SEL_CS( pDev->SPI_SEL_CS );

	spi_reg->SPITCR &= ~( SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | SPI_D1_EN(1) );
	spi_reg->SPITCR |= SPI_D0_EN(1);

	//clear all interrupt
	//spi_reg->SPISTR = 0xC0000000;

	spi_reg->SPIRDR = (u8)data << 24;

	//ctrl
	//spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_LSB(0) | SPI_CMD(1) | SPI_START(1) | SPI_CSP(0);
	spi_reg->SPICNR = SPI_CSTIME(0) | SPI_LSB(0) | SPI_CMD(1) | SPI_CSP(0);	
	spi_reg->SPICNR |= SPI_START(1);  

	//start transfer
	//spi_reg->SPICNR = spicnr;
	//wait unitl finish
	while (spi_reg->SPICNR & SPI_START(1)) ;

	spi_reg->SPITCR = 0x00201823;

	spin_unlock_irqrestore(&spi_lock,flags);
}


void vp_read(rtl_spi_dev_t *pDev, unsigned char *data/*, unsigned int cmd*/)
{
	unsigned int		i,spicnr;
	uint8 buf;
	rtl_8676_reg_t	*spi_reg=get_spi_reg();
	int flags;

	spin_lock_irqsave(&spi_lock,flags);

	//trans. conf
	spi_reg->SPITCR = 0x00201823 | SPI_SEL_CS( pDev->SPI_SEL_CS );

	spi_reg->SPITCR &= ~( SPI_CTL_EN(1) | SPI_ADD_EN(1) | SPI_D0_EN(1) | SPI_D1_EN(1) );
	spi_reg->SPITCR |= SPI_D0_EN(1);

	//clear all interrupt
	//spi_reg->SPISTR = 0xC0000000;

	//clear data
	spi_reg->SPIRDR=0x06;

	//ctrl
	spi_reg->SPICNR = SPI_CSTIME(0) | SPI_RDIE(1) | SPI_LSB(0) | SPI_CMD(0) | SPI_CSP(0);

	spi_reg->SPICNR |= SPI_START(1);

	//start transfer
	//spi_reg->SPICNR = spicnr;

	//wait unitl finish
	while (spi_reg->SPICNR & SPI_START(1)) ;
	
	buf = (unsigned char) ((spi_reg->SPIRDR & 0xff000000) >> 24);
	*data = buf;
	spi_reg->SPITCR = 0x00201823;
	
	spin_unlock_irqrestore(&spi_lock,flags);
}



int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 SPI_SEL_CS)
{

	rtl_8676_reg_t	*spi_reg;
	int flags;
	spin_lock_irqsave(&spi_lock,flags);

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI)
	*(volatile int*) BSP_IP_SEL |= BSP_EN_XSI_VOIP_SPI;
	*(volatile int*) BSP_IP_SEL &= ~(BSP_EN_ISI_VOIP_SPI);
	REG32(BSP_MISC_CR_BASE) |= ((1<<4) | (1<<3) | (1<<1));	
#endif	
	SPI_PRINTK("BSP_IP_SEL:[%08x]=%08x\n", BSP_IP_SEL, *(volatile int*) BSP_IP_SEL );
	SPI_PRINTK("BSP_MISC_CR_BASE:[%08x]=%08x\n", BSP_MISC_CR_BASE, *(volatile int*) BSP_MISC_CR_BASE );
	
	
	/*Initialize Registers*/	
	spi_reg = get_spi_reg();
	spi_reg->SPICKDIV = 0x07000000;
	spi_reg->SPITCALR = 0x05040000;
	spi_reg->SPITCALR = 0x00201823;

	SPI_PRINTK("SPI_SEL_CS = [%d]\n", SPI_SEL_CS);
	//chip select
	pDev->SPI_SEL_CS = SPI_SEL_CS;
	spin_unlock_irqrestore(&spi_lock,flags);

	dumpSPIREG();
	return SUCCESS;
}

#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9662, CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601

#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK

#endif //CONFIG_RTK_VOIP_8676_SHARED_SPI


#if 0
__inline__ void __nsdelay(unsigned long nssecs, unsigned long lpj)
{
	/* This function is a copy of __udelay() in linux-2.4.18/include/asm-mips/delay.h */
    unsigned long lo;

    nssecs *= 0x000001AE;        /* 2**32 / (1000000 * 1000 / HZ) */
    __asm__("multu\t%2,%3"
        :"=h" (nssecs), "=l" (lo)
        :"r" (nssecs),"r" (lpj));
    __delay(nssecs);
}

__inline__ void spi_nsdelay( unsigned long delay )
{
#if 0
	__nsdelay( delay, __udelay_val );	// __udelay_val defined in linux-2.4.18/include/asm-mips/delay.h
#else
	ndelay(delay);
#endif
}
#endif

#if 0

/*---------------------------------------------------------
*	Name:
*		read_legerity_spi
*	Description:
*		Read the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void read_legerity_spi(unsigned int reg, Le88xxx *Legerity)
{
	// TBD
	return;
}

/*---------------------------------------------------------
*	Name:
*		write_legerity_spi
*	Description:
*		Write the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void write_legerity_spi(unsigned int reg, Le88xxx *data)
{
	//TBD
}

/*---------------------------------------------------------
*	Name:
*		write_legerity_spi
*	Description:
*		Write the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void readLegerityReg(unsigned char address, Le88xxx *Legerity)
{
	//TBD 
}
/*---------------------------------------------------------
*	Name:
*		write_legerity_spi
*	Description:
*		Write the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void writeLegerityReg(unsigned char address, Le88xxx *data)
{
	//TBD
}

#endif



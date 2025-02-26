#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#if defined(CONFIG_DEFAULTS_KERNEL_2_6) || defined(CONFIG_DEFAULTS_KERNEL_3_10)
#include <linux/interrupt.h>
#endif
#include "rtk_voip.h"
#include "voip_init.h"

#include "con_register.h"
#include "con_defer_init.h"
#include "snd_pin_cs.h"
#include "snd_proslic_type.h"

//static ctrl_S gSpiGciObj_3217x[ NUMBER_OF_HWINTF_3217X ];
//static ProslicContainer_t gProslicContainer_3217x[ TOTAL_NUM_OF_CH_3217X ];
//static voip_snd_t snd_proslic_3217x[ TOTAL_NUM_OF_CH_3217X ];

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176		// Daisy Chain
static ctrl_S gSpiGciObj_32176[ 1 ];
static ProslicContainer_t gProslicContainer_32176[ CONFIG_RTK_VOIP_SLIC_SI32176_NR ];
static voip_snd_t snd_proslic_32176[ CONFIG_RTK_VOIP_SLIC_SI32176_NR ];
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS	// Chip Select 
static ctrl_S gSpiGciObj_32176_CS[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR ];
static ProslicContainer_t gProslicContainer_32176_CS[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR ];
static voip_snd_t snd_proslic_32176_CS[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR ];
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32178		// Chip Select 
static ctrl_S gSpiGciObj_32178[ CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
static ProslicContainer_t gProslicContainer_32178[ 2 * CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
static voip_snd_t snd_proslic_32178[ 2 * CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
static daa_det_t daa_det_32178[ CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178	// Daisy Chain 
static ctrl_S gSpiGciObj_32176_32178[ 1 ];
static ProslicContainer_t gProslicContainer_32176_32178[ 2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR ];
static voip_snd_t snd_proslic_32176_32178[ 2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR ];
static daa_det_t daa_det_32176_32178[ 1 ];
#endif

static int SLIC_init_si3217x(int pcm_mode, int initonly);

#define ts2count( ts )	( 1 + ( ts ) * 8 )		// Time slot to silab's count 

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176		// Daisy Chain
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS_USE_SW
static const uint32 * const pin_cs_si32176 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS - 1 ];
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS	// Chip Select 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176CS_PIN_CS_USE_SW
static const uint32 * const pin_cs_si32176_CS = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_PIN_CS - 1 ];
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32178		// Chip Select 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS_USE_SW
static const uint32 * const pin_cs_si32178 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS - 1 ];
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178	// Daisy Chain 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS_USE_SW
static const uint32 * const pin_cs_si32176_32178 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS - 1 ];
#endif
#endif

#define HW_SPI_PIN_CS_MARKER	0xFFFF0000	// in GPIO ID domain 

#if 0
static const uint32 pin_cs[] = {
	PIN_CS1, 
#ifdef PIN_CS2
	PIN_CS2, 
#ifdef PIN_CS4
	PIN_CS3, 
	PIN_CS4, 
#ifdef PIN_CS8
	PIN_CS5, 
	PIN_CS6, 
	PIN_CS7, 
	PIN_CS8, 
#endif
#endif
#endif
};

CT_ASSERT( ( sizeof( pin_cs ) / sizeof( pin_cs[ 0 ] ) ) >= ( CONFIG_RTK_VOIP_SLIC_SI32178_NR + !!CONFIG_RTK_VOIP_SLIC_SI32176_NR ) );
#endif

static const proslic_args_t proslic_args_3217x;

static inline void _SLIC_init_si3217x(
		int i_size, int i_device, int i_channel, 
		ProslicContainer_t gProslicContainer_3217x[],
		voip_snd_t snd_proslic_3217x[],
		ctrl_S *gSpiGciObj_3217x, 
		spi_type_t spi_type, uint32 pin_cs_3217x,
		int pcm_mode, 
		int initonly )
{
	// once call this function:
	//  - one control interface 
	//  - i_size total channels 
	//  - i_device chip number 
	//  - i_channel fxs channels
	int j;
	
	switch( spi_type ) {
#ifdef CONFIG_RTK_VOIP_SOFTWARE_SPI
	case SPI_TYPE_SW:	// software SPI (GPIO)
		init_spi_pins( &gSpiGciObj_3217x ->spi_dev, pin_cs_3217x, PIN_CLK, PIN_DO, PIN_DI);
		break;
#endif
#ifdef CONFIG_RTK_VOIP_HARDWARE_SPI
	case SPI_TYPE_HW:	// hardware SPI fully (SPI CS)
		_init_rtl_spi_dev_type_hw( &gSpiGciObj_3217x ->spi_dev, pin_cs_3217x );
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
                extern void slicInternalCS(int cs);
                slicInternalCS(pin_cs_3217x);
#endif
		break;
		
	//case SPI_TYPE_HW2:	// hardware SPI + GPIO CS 
	//	break;
#endif
	default:
		printk( "%s error on line %d: spi type? %d\n", __FUNCTION__, __LINE__, spi_type );
		break;
	}
	
	if( initonly )
		goto label_do_init_only;
		
	// create objs
	for( j = 0; j < i_size; j ++ ) {
		gProslicContainer_3217x[ j ].spiGciObj = gSpiGciObj_3217x;
	}

	proslic_alloc_objs( &gProslicContainer_3217x[ 0 ], 
		i_size, i_device, i_channel, SI3217X_TYPE );
	//si3217x_alloc_objs( &gProslicContainer[ sidx ], 
	//	i_size, i_device, i_channel );

	// init proslic 
label_do_init_only:

	proslic_init_user_objs( &gProslicContainer_3217x[ 0 ], i_size, 
					SI3217X_TYPE );

	//si3217x_init( &snd_proslic[ sidx ], &gProslicContainer[ sidx ], 
	//				i_size, pcm_mode );
	proslic_init( &snd_proslic_3217x[ 0 ], &gProslicContainer_3217x[ 0 ], 
					i_size, 
					&proslic_args_3217x,
					pcm_mode );
}

static int SLIC_init_si3217x(int pcm_mode, int initonly)
{
	//int ch;
#if defined( CONFIG_RTK_VOIP_SLIC_SI32178) || defined(CONFIG_RTK_VOIP_SLIC_SI32176_CS)
	int i;
#endif
	//int i, j, sidx;
	//int i_size, i_device, i_channel;
	//rtl_spi_dev_t *spi_devs[ NUMBER_OF_HWINTF_3217X ];
	
	printk ("\n<<<<<<<<<<< Si3217x Driver Version %s >>>>>>>>>>\n", ProSLIC_Version());
	
	//if( !initonly ) {
	//	// init spi first, because reset pin will affect all SLIC 
	//	printk( "Preparing spi channel for SLIC...\n" );
	//	
	//	for( i = 0; i < NUMBER_OF_HWINTF_3217X; i ++ ) {
	//		spi_devs[ i ] = &gSpiGciObj_3217x[ i ].spi_dev;
	//	}
	//
	//	init_spi_channels( NUMBER_OF_HWINTF_3217X, spi_devs, pin_cs, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
	//}

#if 1
	
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176		// Daisy Chain
	printk( "------------------------------\n" );
	printk( "SLIC 32176 HW intf %d CS=%X\n", 0, 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS_USE_SW
											pin_cs_si32176[ 0 ]
#else
											CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS
#endif
											);
		
	_SLIC_init_si3217x( 
			CONFIG_RTK_VOIP_SLIC_SI32176_NR, 
			CONFIG_RTK_VOIP_SLIC_SI32176_NR,
			CONFIG_RTK_VOIP_SLIC_SI32176_NR,
			gProslicContainer_32176,
			snd_proslic_32176, 
			&gSpiGciObj_32176[ 0 ], 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS_USE_SW
			SPI_TYPE_SW, pin_cs_si32176[ 0 ],
#else
			SPI_TYPE_HW, CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS,
#endif
			pcm_mode,
			initonly );
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS	// Chip Select 
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR; i ++ ) {
		printk( "------------------------------\n" );
		printk( "SLIC 32176 HW intf %d CS=%X\n", i, 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176CS_PIN_CS_USE_SW
												pin_cs_si32176_CS[ i ] 
#else
												CONFIG_RTK_VOIP_SLIC_SI32176_CS_PIN_CS + i
#endif
												);
		
		_SLIC_init_si3217x( 
				1, 1, 1,
				&gProslicContainer_32176_CS[ i ],
				&snd_proslic_32176_CS[ i ],
				&gSpiGciObj_32176_CS[ i ], 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176CS_PIN_CS_USE_SW
				SPI_TYPE_SW, pin_cs_si32176_CS[ i ],
#else
				SPI_TYPE_HW, CONFIG_RTK_VOIP_SLIC_SI32176_CS_PIN_CS + i,
#endif
				pcm_mode, 
				initonly );
	}
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32178		// Chip Select 
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32178_NR; i ++ ) {
		
		printk( "------------------------------\n" );
		printk( "SLIC 31278 HW intf %d CS=%X\n", i, 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS_USE_SW
												pin_cs_si32178[ i ] 
#else
												CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS + i
#endif
												);

		
		_SLIC_init_si3217x( 
				2, 1, 1,
				&gProslicContainer_32178[ i * 2 ],
				&snd_proslic_32178[ i * 2 ],
				&gSpiGciObj_32178[ i ], 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS_USE_SW
				SPI_TYPE_SW, pin_cs_si32178[ i ],	// consider 32176 
#else
				SPI_TYPE_HW, CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS + i,	// consider 32176 
#endif
				pcm_mode, 
				initonly );
		
	}
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178	// Daisy Chain 
	printk( "------------------------------\n" );
	printk( "SLIC 32176/78 HW intf %d CS=%X\n", 0, 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS_USE_SW
									pin_cs_si32176_32178[ 0 ] 
#else
									CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS
#endif
									);
	
	_SLIC_init_si3217x(
			2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR,
			1 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR,
			1 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR,
			gProslicContainer_32176_32178,
			snd_proslic_32176_32178, 
			&gSpiGciObj_32176_32178[ 0 ],
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS_USE_SW
			SPI_TYPE_SW, pin_cs_si32176_32178[ 0 ],
#else
			SPI_TYPE_HW, CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS, 
#endif
			pcm_mode,
			initonly );
#endif
		
#elif 0
	for( i = 0, sidx = 0; i < NUMBER_OF_HWINTF_3217X; i ++ ) {
		
		printk( "------------------------------\n" );
		printk( "SLIC HW intf %d starting at %d\n", i, sidx );
		
		init_spi_pins( &gSpiGciObj_3217x[ i ].spi_dev, pin_cs[ i ], PIN_CLK, PIN_DO, PIN_DI);
		
  #if defined( CONFIG_RTK_VOIP_MULTIPLE_SI32178 )
		i_size = CHAN_PER_DEVICE_3217X + 1;	// number of FXS/FXO port 
		i_device = 1;					// number of chip 
		i_channel = CHAN_PER_DEVICE_3217X;	// number of FXS port 
  #elif defined( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_CS )
  		// This assume CS of 32176 at preceding index 
		i_size = ( i == NUMBER_OF_HWINTF_3217X - 1 ? 
					CHAN_PER_DEVICE_3217X + 1 :
					CHAN_PER_DEVICE_3217X );
		i_device = 1;
		i_channel = CHAN_PER_DEVICE_3217X;
  #else
		i_size = TOTAL_NUM_OF_CH_3217X;
		i_device = NUMBER_OF_PROSLIC_3217X;
		i_channel = NUMBER_OF_CHAN_3217X;
  #endif
		
		if( initonly )
			goto label_do_init_only;
			
		// create objs
		//init_spi_channel( &gSpiGciObj[ i ].spi_dev, pin_cs[ i ], PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		for( j = 0; j < i_size; j ++ ) {
			gProslicContainer_3217x[ sidx + j ].spiGciObj = &gSpiGciObj_3217x[ i ];
		}

		proslic_alloc_objs( &gProslicContainer_3217x[ sidx ], 
			i_size, i_device, i_channel, SI3217X_TYPE );
		//si3217x_alloc_objs( &gProslicContainer[ sidx ], 
		//	i_size, i_device, i_channel );

		// init proslic 
label_do_init_only:

		proslic_init_user_objs( &gProslicContainer_3217x[ sidx ], i_size, 
						SI3217X_TYPE );

		//si3217x_init( &snd_proslic[ sidx ], &gProslicContainer[ sidx ], 
		//				i_size, pcm_mode );
		proslic_init( &snd_proslic_3217x[ sidx ], &gProslicContainer_3217x[ sidx ], 
						i_size, 
						&proslic_args_3217x,
						pcm_mode );
		
		sidx += i_size;
	}
#else
	// create objs
	//gSpiGciObj[ 0 ].portID = 0;
	init_spi_channel( &gSpiGciObj_3217x[ 0 ].spi_dev, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
	
	for( i = 0; i < TOTAL_NUM_OF_CH_3217X; i ++ )
		gProslicContainer_3217x[ i ].spiGciObj = &gSpiGciObj_3217x[ 0 ];
	
	si3217x_alloc_objs( gProslicContainer_3217x, 
		TOTAL_NUM_OF_CH_3217X, NUMBER_OF_PROSLIC_3217X, TOTAL_NUM_OF_CH_3217X - NUM_OF_DAA_3217X );
	si3217x_init( snd_proslic, gProslicContainer_3217x, TOTAL_NUM_OF_CH_3217X, pcm_mode );
#endif

	printk("<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>\n\n");
	
	return 0;
}

static void SLIC_reset_si3217x(voip_snd_t *this, int codec_law)
{
	// This function will cause system reset, if watch dog is enable!
	// Because calibration need mdelay(1900).
	unsigned long flags;
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Disable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  0xA5000000;
	restore_flags(flags);
	
	SLIC_init_si3217x( codec_law, 1 /* don't allocate */ );
	//si3217x_init(snd_proslic, gProslicContainer, TOTAL_NUM_OF_CH_3217X, law);
	
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Enable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  1 << 23;
	restore_flags(flags);
}

/* This API is workable only for Si32178, chid is SLIC's chid. */
#if defined( CONFIG_RTK_VOIP_SLIC_SI32178 ) || defined( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178 )
static void FXS_FXO_DTx_DRx_Loopback_si3217x(voip_snd_t *this, voip_snd_t *daa_snd, unsigned int enable)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pfxs = container ->ProObj;
	ProslicContainer_t * const daa_container = ( ProslicContainer_t * )daa_snd ->priv;
	vdaaChanType * const daas = daa_container ->daas;
	//proslicChanType *pfxs;	
	//pfxs = ports[chid].ProObj;
	const unsigned char chid = this ->sch;
	
	if( container ->ProSLICDevices != daa_container ->ProSLICDevices )
		printk( "Si3217x different devices loopback\n" );
	
	ProSLIC_SO_DTRx_Loopback(pfxs, enable);

	if (enable == 1)
	{
		//ProSLIC_PCMTimeSlotSetup(pfxs, 1+(pfxs->channel*8), 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
		ProSLIC_PCMTimeSlotSetup(pfxs, ts2count( this ->TS1 ), ts2count( daa_snd ->TS1 ));
		
#if 0//def CONFIG_RTK_VOIP_MULTIPLE_SI32178
		Vdaa_PCMTimeSlotSetup(daas[chid], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel*8));
#else
		//Vdaa_PCMTimeSlotSetup(daas, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel*8));
		Vdaa_PCMTimeSlotSetup(daas, ts2count( daa_snd ->TS1 ), ts2count( this ->TS1 ));
#endif
		//printk("fxo-%d: 0x%p, %d, %d\n", chid, daas[0], 1, 65);
		PRINT_MSG("Set SI32178 FXS/O loopback mode for FXS port%d\n", chid);
	}
	else if (enable == 0)
	{
		ProSLIC_PCMTimeSlotSetup(pfxs, ts2count( this ->TS1 ), ts2count( this ->TS1 ));
#if 0 //def CONFIG_RTK_VOIP_MULTIPLE_SI32178
		Vdaa_PCMTimeSlotSetup(daas[chid], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
#else
		Vdaa_PCMTimeSlotSetup(daas, ts2count( daa_snd ->TS1 ), ts2count( daa_snd ->TS1 ));
#endif
		//printk("fxo-%d: 0x%p, %d, %d\n", chid, daas[chid], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
		//Vdaa_PCMTimeSlotSetup(daas[0], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
		PRINT_MSG("Disable SI32178 FXS/O loopback mode for FXS port%d\n", chid);
	}
}
#endif

static void SLIC_show_ID_si3217x( voip_snd_t *this )
{
	unsigned char reg_val, reg_len;
	
	reg_len = sizeof(reg_val);

	//reg_val = R_reg_dev(&spiGciObj ->spi_dev, i, 0);
	this ->fxs_ops ->SLIC_read_reg( this, 0, &reg_len, &reg_val );
	
	switch ((reg_val&0x38)>>3)
	{
		case 0:
			printk("Si32171 ");
			break;
		case 3:
			printk("Si32175 ");
			break;
		case 4:
			printk("Si32176 ");
			break;
		case 5:
			printk("Si32177 ");
			break;
		case 6:
			printk("Si32178 ");
			break;
		default:
			PRINT_R("Unknow SLIC ");
			break;
	}
	
	
	if ((reg_val&0x07) == 0)
		printk("Revision A\n");
	else if ((reg_val&0x07) == 1)
		printk("Revision B\n");
	else if ((reg_val&0x07) == 2)
		printk("Revision C\n");
	else
		PRINT_R("Unknow Revision\n");
}


// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

__attribute__ ((section(".snd_desc_data")))
static snd_ops_fxs_t snd_si3217x_fxs_ops;

static const proslic_args_t proslic_args_3217x = {
	.ring_setup_preset = 2,	// 20Hz, 48VRMS
};

static void __init fill_3217x_register_info( 
	voip_snd_t snd_proslic_3217x[],
	int n_fxs, int m_daa, uint16 TS_base,
	ProslicContainer_t gProslicContainer_3217x[], 
	daa_det_t *daa_det)
{
	// once call this function:
	//  - one control interface 
	//  - n fxs
	//  - m daa 
	int sch;
	int daa = 0;

#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	extern const snd_ops_daa_t snd_si3050_daa_ops;
#endif
	
	for( sch = 0; sch < n_fxs + m_daa; sch ++ ) {
	
		if( sch == n_fxs )
			daa = 1;

		snd_proslic_3217x[ sch ].sch = sch;
		snd_proslic_3217x[ sch ].name = "si3217x";
		snd_proslic_3217x[ sch ].snd_type = SND_TYPE_FXS;
		snd_proslic_3217x[ sch ].bus_type_sup = BUS_TYPE_PCM;
		snd_proslic_3217x[ sch ].TS1 = TS_base + sch * 2;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x_WIDEBAND
		snd_proslic_3217x[ sch ].TS2 = ( daa || TS_base + sch * 2 >= 16 ? 0 : TS_base + ( sch + 8 ) * 2 );
		snd_proslic_3217x[ sch ].band_mode_sup = ( daa ? BAND_MODE_8K : BAND_MODE_8K | BAND_MODE_16K );
#else
		snd_proslic_3217x[ sch ].TS2 = 0;
		snd_proslic_3217x[ sch ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_proslic_3217x[ sch ].snd_ops = ( const snd_ops_t * )&snd_si3217x_fxs_ops;
		snd_proslic_3217x[ sch ].priv = &gProslicContainer_3217x[ sch ];
		
		// DAA port  
		if( daa ) {
			gProslicContainer_3217x[ sch ].daa_det = daa_det;
			
			snd_proslic_3217x[ sch ].snd_type = SND_TYPE_DAA;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
			snd_proslic_3217x[ sch ].snd_ops = ( const snd_ops_t * )&snd_si3050_daa_ops;
#else
			printk( "No snd_ops for DAA!!\n" );
#endif
		}		
	}
}

static int __init voip_snd_proslic_init_3217x( void )
{
	extern int law;	// FIXME: chmap 
	extern const snd_ops_fxs_t snd_proslic_fxs_ops;
#if defined( CONFIG_RTK_VOIP_SLIC_SI32178) || defined(CONFIG_RTK_VOIP_SLIC_SI32176_CS)
	int i;//, sch, daa;
#endif
	int TS_base;
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	static defer_init_t si3217x_defer;
#endif
	
	// si3217x override proslic base ops 
	snd_si3217x_fxs_ops = snd_proslic_fxs_ops;

#if defined( CONFIG_RTK_VOIP_SLIC_SI32178 ) || defined( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178 )	
	snd_si3217x_fxs_ops.FXS_FXO_DTx_DRx_Loopback = FXS_FXO_DTx_DRx_Loopback_si3217x;
#endif
	snd_si3217x_fxs_ops.SLIC_reset = SLIC_reset_si3217x;
	snd_si3217x_fxs_ops.SLIC_show_ID = SLIC_show_ID_si3217x;
	
	// get TS base 
	TS_base = get_snd_free_timeslot();
	if( TS_base < 0 )
		TS_base = 0;
	
	// common port definition 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176
	fill_3217x_register_info( snd_proslic_32176, 
			CONFIG_RTK_VOIP_SLIC_SI32176_NR, 0, TS_base,
			gProslicContainer_32176, 
			NULL );
	
	register_voip_snd( &snd_proslic_32176[ 0 ], CONFIG_RTK_VOIP_SLIC_SI32176_NR );	
	
	TS_base += CONFIG_RTK_VOIP_SLIC_SI32176_NR * 2;
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR; i ++ ) {
		
		fill_3217x_register_info( &snd_proslic_32176_CS[ i ], 
				1 /* fxs */, 0 /* daa */, TS_base,
				&gProslicContainer_32176_CS[ i ], 
				NULL);
		
		register_voip_snd( &snd_proslic_32176_CS[ i ], 1 );	
		
		TS_base += 2;
	}	
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32178
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32178_NR; i ++ ) {
		
		fill_3217x_register_info( &snd_proslic_32178[ i * 2 ], 
				1 /* fxs */, 1 /* daa */, TS_base,
				&gProslicContainer_32178[ i * 2 ],
				&daa_det_32178[ i ] );
		
		register_voip_snd( &snd_proslic_32178[ i * 2 ], 2 );	
		
		TS_base += 4;
	}
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178
	fill_3217x_register_info( snd_proslic_32176_32178, 
			CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR + 1, 1, TS_base,
			gProslicContainer_32176_32178,
			daa_det_32176_32178 );
	
	register_voip_snd( &snd_proslic_32176_32178[ 0 ], 2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR );		
	
	TS_base += ( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR + 2 ) * 2;
#endif

	// SLIC init use ops 
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	si3217x_defer.fn_defer_func = ( fn_defer_func_t )SLIC_init_si3217x;
	si3217x_defer.p0 = law;
	si3217x_defer.p1 = 0;
	
	add_defer_initialization( &si3217x_defer );
#else
	SLIC_init_si3217x( law, 0 /* allocate */ );
#endif
	
	return 0;
}

voip_initcall_snd( voip_snd_proslic_init_3217x );

#ifdef SLICDUMPREG
#define VDUMPREGNAME1 "slicdumpreg3217x"
extern void SLIC_read_proslic_reg(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val);
extern void SLIC_read_proslic_ram(voip_snd_t *this, unsigned int num, unsigned int *val);
extern void SLIC_write_proslic_reg(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val);
extern void SLIC_write_proslic_ram(voip_snd_t *this, unsigned int num, unsigned int val);
extern void SLIC_dump_proslic_reg(voip_snd_t *this);
extern void SLIC_dump_proslic_ram(voip_snd_t *this);


#if defined(CONFIG_RTK_VOIP_SLIC_SI32176)
#define snd_proslic	snd_proslic_32176
#elif defined(CONFIG_RTK_VOIP_SLIC_SI32176_CS)
#define snd_proslic	snd_proslic_32176_CS
#elif defined(CONFIG_RTK_VOIP_SLIC_SI32178)
#define snd_proslic	snd_proslic_32178
#elif defined(CONFIG_RTK_VOIP_SLIC_SI32176_SI32178)
#define snd_proslic	snd_proslic_32176_32178
#endif

static int voip_dumpreg_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	//SLIC_dump_proslic(&snd_proslic_32176_CS[0]);
	return 0;
}

void writeram(void){
#if 0
	unsigned int t3 = 0;
	int t2 = 0, *len = 1;
			//TXACEQ
			t2 = 540;
			t3 = 0x07F4E380;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 541;
			t3 = 0x000D8F00;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 542;
			t3 = 0x0000A980;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 543;
			t3 = 0x1FFD5200;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			//RXACEQ
			t2 = 546;
			t3 = 0x07F2DB00;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 547;
			t3 = 0x0013B280;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 548;
			t3 = 0x1FFE3C00;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 549;
			t3 = 0x1FFCC900;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);				
			
			//ECFIR
			t2 = 563;
			t3 = 0x0012DC80;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			
			t2 = 564;
			t3 = 0x1FED3780;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);										
			
			t2 = 565;
			t3 = 0x019C0300;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													

			t2 = 566;
			t3 = 0x00BF5C80;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													
	
			t2 = 567;
			t3 = 0x01ACD900;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			
			t2 = 568;
			t3 = 0x1F73A980;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);										
			
			t2 = 569;
			t3 = 0x00468880;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													

			t2 = 570;
			t3 = 0x1FEA7580;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													
		
			t2 = 571;
			t3 = 0x1FFC1180;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			
			t2 = 572;
			t3 = 0x00034800;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);										
			
			t2 = 573;
			t3 = 0x0F60F800;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													

			t2 = 574;
			t3 = 0x189DA600;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													

			// ZSYNTH

			t2 = 653;
			t3 = 0x007DD200;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																

			t2 = 654;
			t3 = 0x1F053480;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																

			t2 = 655;
			t3 = 0x007CFC00;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																

			t2 = 656;
			t3 = 0x0FF66E00;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																

			t2 = 657;
			t3 = 0x18098F80;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																			
			
			t2 = 45;
			t3 = 0x5D;
			printk("reg = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_reg(&snd_proslic_32176_CS[0], t2, len, &t3);			
			
			//TXACGAIN
			t2 = 544;
			t3 = 0x08C4D280;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);				
			//RXACGAIN
			t2 = 545;
			t3 = 0x014D1380;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			
			//RXACHPF
			t2 = 658;
			t3 = 0x07A0BF00;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 659;
			t3 = 0x185F4180;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);

			t2 = 660;
			t3 = 0x07417E00;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);			
			
			t2 = 906;
			t3 = 0x14D1380;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);				
			
			t2 = 666;
			t3 = 0x8000000;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			
			t2 = 533;
			t3 = 0x71EB851;
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
#endif			
}


void readram(void){
#if 0	
	unsigned int t3 = 0;
	int t2 = 0, len;

			//TXACEQ
			t2 = 540;
			t3 = 0x07F4E380;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 541;
			t3 = 0x000D8F00;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 542;
			t3 = 0x0000A980;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 543;
			t3 = 0x1FFD5200;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			//RXACEQ
			t2 = 546;
			t3 = 0x07F2DB00;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 547;
			t3 = 0x0013B280;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 548;
			t3 = 0x1FFE3C00;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 549;
			t3 = 0x1FFCC900;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);				
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			//ECFIR
			t2 = 563;
			t3 = 0x0012DC80;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);			
			
			t2 = 564;
			t3 = 0x1FED3780;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);										
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 565;
			t3 = 0x019C0300;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 566;
			t3 = 0x00BF5C80;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
	
			t2 = 567;
			t3 = 0x01ACD900;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 568;
			t3 = 0x1F73A980;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);										
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 569;
			t3 = 0x00468880;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);			

			t2 = 570;
			t3 = 0x1FEA7580;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
		
			t2 = 571;
			t3 = 0x1FFC1180;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 572;
			t3 = 0x00034800;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);										
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 573;
			t3 = 0x0F60F800;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 574;
			t3 = 0x189DA600;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);													
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			// ZSYNTH

			t2 = 653;
			t3 = 0x007DD200;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 654;
			t3 = 0x1F053480;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);			

			t2 = 655;
			t3 = 0x007CFC00;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 656;
			t3 = 0x0FF66E00;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);			

			t2 = 657;
			t3 = 0x18098F80;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);																			
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 45;
			t3 = 0x5D;
			SLIC_read_proslic_reg(&snd_proslic_32176_CS[0], t2, &len, &t3);			
			printk("reg = [%d], value = 0x[%x]\n", t2, t3);
			
			//TXACGAIN
			t2 = 544;
			t3 = 0x08C4D280;
			SLIC_write_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			//RXACGAIN
			t2 = 545;
			t3 = 0x014D1380;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			//RXACHPF
			t2 = 658;
			t3 = 0x07A0BF00;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 659;
			t3 = 0x185F4180;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);

			t2 = 660;
			t3 = 0x07417E00;
			
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);			
			
			t2 = 906;
			t3 = 0x14D1380;
			
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);			
			
			t2 = 666;
			t3 = 0x8000000;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);							
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
			
			t2 = 533;
			t3 = 0x71EB851;
			SLIC_read_proslic_ram(&snd_proslic_32176_CS[0], t2, &t3);			
			printk("ram = [%d], value = 0x[%x]\n", t2, t3);
#endif			
}

static int voip_dumpreg_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[128];
	unsigned char t3;
	unsigned int p2 = 0, t4 = 0;
	int t0 = 0, t1 = 0, t2 = 0, *len = 1, p0 = 0, p1 = 0, len2;

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d %d 0x%x", &p0, &p1, &p2);
		printk("[%d] reg = [%d], value = 0x[%x]\n", p0, p1, p2);

		if ( p0 == 0 ) { 	
			if ( p1 == 0 ){
				//dump reg
				SLIC_dump_proslic_reg(&snd_proslic[0]);
			}else if ( p1 == 1 ){
				//dump ram
				SLIC_dump_proslic_ram(&snd_proslic[0]);						
			}else if ( p1 == 2 ){
				t2 = 30;
				t3 = 0x14;
				printk("reg = [%d], value = 0x[%x]\n", t1, t3);
				SLIC_write_proslic_reg(&snd_proslic[0], t2, len, &t3);			
			}else if ( p1 == 3 ){
				t2 = 126;
				t3 = 0x3;
				printk("reg = [%d], value = 0x[%x]\n", t1, t3);
				SLIC_write_proslic_reg(&snd_proslic[0], t2, len, &t3);			
			}
		}else if ( p0 == 1 ) {	// write reg
			t2 = p1;
			t3 = (unsigned char)p2;
			printk("reg = [%d], value = 0x[%x]\n", t2, t3);
			SLIC_write_proslic_reg(&snd_proslic[0], t2, len, &t3);		
		}else if ( p0 == 2 ) {	// write ram
			t2 = p1;
			printk("ram = [%d], value = 0x[%x]\n", t2, p2);
			SLIC_write_proslic_ram(&snd_proslic[0], t2, p2);			
		}else if ( p0 == 3 ) {	// read reg
			t2 = p1;
			SLIC_read_proslic_reg(&snd_proslic[0], t2, &len2, &t3);			
			printk("reg = [%d], value = 0x[%x]\n", t2, t3);
		}else if ( p0 == 4 ) {	// read ram
			t2 = p1;
			SLIC_read_proslic_ram(&snd_proslic[0], t2, &t4);			
			printk("ram = [%d], value = 0x[%x]\n", t2, t4);
		}
		

	}	
	return count;
}

static int __init voip_proc_dumpreg_init(void)
{
	struct proc_dir_entry *voip_add_test_proc;

	voip_add_test_proc = create_proc_entry( VDUMPREGNAME1, 0644, NULL);
	if (voip_add_test_proc == NULL) {
		remove_proc_entry(VDUMPREGNAME1, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				VDUMPREGNAME1);
		return -ENOMEM;
	}
	voip_add_test_proc->read_proc  = (read_proc_t *)voip_dumpreg_read;
	voip_add_test_proc->write_proc  = (write_proc_t *)voip_dumpreg_write;

	
	return 0;
}

static void __exit voip_proc_dumpreg_exit( void )
{
	remove_proc_entry( VDUMPREGNAME1, NULL );
}

module_init( voip_proc_dumpreg_init );
module_exit( voip_proc_dumpreg_exit );
#endif


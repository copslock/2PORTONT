#ifndef _RTL9607B_RAW_PBO_H_
#define _RTL9607B_RAW_PBO_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <common/rt_type.h>
#include <rtk/port.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/rtl9607b/dal_rtl9607b.h>


/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
typedef enum rtl9607b_pbo_ponMode_e {
    RTL9607B_PBO_PONMODE_GPON = 0,
    RTL9607B_PBO_PONMODE_EPON,
    RTL9607B_PBO_PONMODE_END
} rtl9607b_pbo_ponMode_t;


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef struct rtl9607b_pbo_cnt_s {
    uint32 rx_err_cnt;
    uint32 rx_drop_cnt;
    uint32 tx_empty_eob_cnt;
    uint32 rx_in_err;
} rtl9607b_pbo_cnt_t;

typedef struct rtl9607b_pbo_ponnic_cnt_s {
    uint16 pkt_ok_cnt_tx;
    uint16 pkt_ok_cnt_rx;
    uint16 pkt_err_cnt_tx;
    uint16 pkt_err_cnt_rx;
    uint32 pkt_miss_cnt;
} rtl9607b_pbo_ponnic_cnt_t;

typedef struct rtl9607b_pbo_group_cnt_s {
    uint32 rx_good_cnt;
    uint32 rx_bad_cnt;
    uint32 rx_cnt;
    uint32 tx_cnt;
    uint32 tx_frag_cnt;
    uint32 rx_frag_cnt;
} rtl9607b_pbo_group_cnt_t;

typedef struct rtl9607b_memUsage_s {
    uint32 us_base;
    uint32 us_offset;
    uint32 us_page_size;
    uint32 us_page_cnt;

    uint32 ds_base;
    uint32 ds_offset;
    uint32 ds_page_size;
    uint32 ds_page_cnt;
} rtl9607b_memUsage_t;

typedef enum rtl9607b_pbo_fc_status_e {
    RTL9607B_PBO_FLOWCONTROL_STATUS_CURRENT,
    RTL9607B_PBO_FLOWCONTROL_STATUS_LATCHED,
    RTL9607B_PBO_FLOWCONTROL_STATUS_END
} rtl9607b_pbo_fc_status_t;

typedef enum rtl9607b_pbo_dsQueue_type_e {
    RTL9607B_PBO_DSQUEUE_TYPE_NORMAL0,
    RTL9607B_PBO_DSQUEUE_TYPE_NORMAL1,
    RTL9607B_PBO_DSQUEUE_TYPE_HIGH,
    RTL9607B_PBO_DSQUEUE_TYPE_END
} rtl9607b_pbo_dsQueue_type_t;

typedef enum rtl9607b_pbo_dsQueueQos_type_e {
    RTL9607B_PBO_DSQUEUEQOS_TYPE_ROUNDROBIN,
    RTL9607B_PBO_DSQUEUEQOS_TYPE_STRICTPRIORITY,
    RTL9607B_PBO_DSQUEUEQOS_TYPE_END
} rtl9607b_pbo_dsQueueQos_type_t;

extern int32 rtl9607b_raw_pbo_stopAllThreshold_set(uint32 threshold);
extern int32 rtl9607b_raw_pbo_stopAllThreshold_get(uint32 *pThreshold);
extern int32 rtl9607b_raw_pbo_globalThreshold_set(uint32 onThreshold, uint32 offThreshold);
extern int32 rtl9607b_raw_pbo_globalThreshold_get(uint32 *pOnThreshold, uint32 *pOffThreshold);
extern int32 rtl9607b_raw_pbo_sidThreshold_set(uint32 sid, uint32 onThreshold, uint32 offThreshold);
extern int32 rtl9607b_raw_pbo_sidThreshold_get(uint32 sid,uint32 *pOnThreshold, uint32 *pOffThreshold);
extern int32 rtl9607b_raw_pbo_usState_set(rtk_enable_t enable);
extern int32 rtl9607b_raw_pbo_usState_get(rtk_enable_t *pEnable);
extern int32 rtl9607b_raw_pbo_dsState_set(rtk_enable_t enable);
extern int32 rtl9607b_raw_pbo_dsState_get(rtk_enable_t *pEnable);
extern int32 rtl9607b_raw_pbo_usStatus_get(rtk_enable_t *pEnable);
extern int32 rtl9607b_raw_pbo_dsStatus_get(rtk_enable_t *pEnable);
extern int32 rtl9607b_raw_pbo_usDscRunoutThreshold_set(uint32 dram, uint32 sram);
extern int32 rtl9607b_raw_pbo_usDscRunoutThreshold_get(uint32 *pDram, uint32 *pSram);
extern int32 rtl9607b_raw_pbo_dsDscRunoutThreshold_set(uint32 dram, uint32 sram);
extern int32 rtl9607b_raw_pbo_dsDscRunoutThreshold_get(uint32 *pDram, uint32 *pSram);
extern int32 rtl9607b_raw_pbo_usUsedPageCount_get(uint32 *pSram, uint32 *pDram);
extern int32 rtl9607b_raw_pbo_usMaxUsedPageCount_get(uint32 *pSram, uint32 *pDram);
extern int32 rtl9607b_raw_pbo_usMaxUsedPageCount_clear(void);
extern int32 rtl9607b_raw_pbo_dsUsedPageCount_get(uint32 *pSram, uint32 *pDram);
extern int32 rtl9607b_raw_pbo_dsMaxUsedPageCount_get(uint32 *pSram, uint32 *pDram);
extern int32 rtl9607b_raw_pbo_dsMaxUsedPageCount_clear(void);
extern int32 rtl9607b_raw_pbo_usCounterGroupMember_add(uint32 group, uint32 sid);
extern int32 rtl9607b_raw_pbo_usCounterGroupMember_clear(uint32 group);
extern int32 rtl9607b_raw_pbo_usCounterGroupMember_get(uint32 group, uint32 sid, rtk_enable_t *pEnabled);
extern int32 rtl9607b_raw_pbo_usCounter_get(rtl9607b_pbo_cnt_t *pCnt);
extern int32 rtl9607b_raw_pbo_usCounter_clear(void);
extern int32 rtl9607b_raw_pbo_usPonnicCounter_get(rtl9607b_pbo_ponnic_cnt_t *pCnt);
extern int32 rtl9607b_raw_pbo_usPonnicCounter_clear(void);
extern int32 rtl9607b_raw_pbo_usGroupCounter_clear(uint32 group);
extern int32 rtl9607b_raw_pbo_usGroupCounter_get(uint32 group, rtl9607b_pbo_group_cnt_t *pCnt);
extern int32 rtl9607b_raw_pbo_dsCounterGroupMember_add(uint32 group, uint32 sid);
extern int32 rtl9607b_raw_pbo_dsCounterGroupMember_clear(uint32 group);
extern int32 rtl9607b_raw_pbo_dsCounterGroupMember_get(uint32 group, uint32 sid, rtk_enable_t *pEnabled);
extern int32 rtl9607b_raw_pbo_dsCounter_get(rtl9607b_pbo_cnt_t *pCnt);
extern int32 rtl9607b_raw_pbo_dsCounter_clear(void);
extern int32 rtl9607b_raw_pbo_dsPonnicCounter_get(rtl9607b_pbo_ponnic_cnt_t *pCnt);
extern int32 rtl9607b_raw_pbo_dsPonnicCounter_clear(void);
extern int32 rtl9607b_raw_pbo_dsGroupCounter_clear(uint32 group);
extern int32 rtl9607b_raw_pbo_dsGroupCounter_get(uint32 group, rtl9607b_pbo_group_cnt_t *pCnt);
extern int32 rtl9607b_raw_pbo_flowcontrolStatus_get(uint32 sid, rtl9607b_pbo_fc_status_t latched, rtk_enable_t *pStatus);
extern int32 rtl9607b_raw_pbo_flowcontrolLatchStatus_clear(uint32 sid);
extern int32 rtl9607b_raw_pbo_init(rtl9607b_pbo_ponMode_t mode, uint32 usPageSize, uint32 dsPageSize);

#endif /* _RTL9607B_RAW_PBO_H_ */


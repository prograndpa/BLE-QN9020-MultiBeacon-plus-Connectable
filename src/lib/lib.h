/**
 ****************************************************************************************
 *
 * @file lib.h
 *
 * @brief QN9020 library API header file.
 *
 * Copyright(C) 2015 NXP Semiconductors N.V.
 * All rights reserved.
 *
 * $Rev: 1.0 $
 *
 ****************************************************************************************
 */
 
#ifndef LIB_H_
#define LIB_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>
#include "qnrf.h"
#if (QN_WORK_MODE == WORK_MODE_SOC)
#include "app_env.h"
#endif
#include "ke_task.h"

/*
 * DEFINITIONS
 ****************************************************************************************
 */
#define QN_DBG_INFO_REG                     0x1000FFFC

#define QN_DBG_INFO_XTAL_WAKEUP_DURATION    0x00000001
#define QN_DBG_INFO_BLE_HEAP_FULL           0x00000002

/*
 * ENUMERATION DEFINITIONS
 ****************************************************************************************
 */
/// Work mode
enum WORK_MODE
{
    SOC_MODE,   // Wireless SoC
    NP_MODE,    // Network processor
    HCI_MODE    // Controller only
};

/// Power mode
enum PW_MODE
{
    NORMAL_MODE,        // Low power
    HIGH_PERFORMANCE    // High Power
};

/// Status of ke_evt
enum KE_EVENT_STATUS
{
    KE_EVENT_OK = 0,
    KE_EVENT_FAIL,
    KE_EVENT_UNKNOWN,
    KE_EVENT_CAPA_EXCEEDED,
    KE_EVENT_ALREADY_EXISTS,
};

/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

/// Contrl structure of BLE new features
struct qn_feature_ctrl
{
   /**
    * flag bit field
    *
    *   7    6    5    4    3    2    1    0
    * +----+----+----+----+----+----+----+----+
    * |    |    |    |    |    |    |read|ntf |
    * +----+----+----+----+----+----+----+----+
    *
    * Bit [0] : (1/0 = use/no multi-notification)
    * Bit [1] : (1/0 = use/no read indication)
    */
    uint8_t flag;
    /// atts default state table
    struct ke_msg_handler *atts_default_state_new;
    /// atts default handler table
    struct ke_state_handler *atts_default_handler_new;
    #if (defined(QN_9020_B4))
    // Idle state handlers definition
    struct ke_msg_handler *gatt_idle_new;
    struct ke_state_handler *gatt_idle_handler_new;
    #endif
    /// Patches for new features
    ke_msg_func_t gatt_read_req_cfm_handler_patch;
    /// function of atts_hdl_val_ntf_req_handler
    ke_msg_func_t atts_hdl_val_ntf_req_handler_patch;
    #if (defined(QN_9020_B2))
    /// function of l2cc_data_send_rsp_att_handler
    ke_msg_func_t l2cc_data_send_rsp_att_handler_patch;
    #endif
    ke_msg_func_t l2cc_data_packet_ind_handler_patch;
    
    ke_msg_func_t llcp_con_up_req_handler_patch;
    ke_msg_func_t llcp_channel_map_req_handler_patch;
    #if (defined(QN_9020_B2))
    void (*lld_evt_schedule_patch)(void);
    void (*lld_evt_end_patch)(void);
    #endif
    #if (defined(QN_9020_B4))
    void (*lld_evt_restart_patch)(void *p_evt);
    #endif
    void (*lld_evt_schedule_next)(void *p_evt);
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef void (*hci_api)(void *port, uint8_t *bufptr, uint32_t size, void (*callback)(void));

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable 32k low power mode
 ***************************************************************************************
 */
extern void enable_32k_mode(void);

/**
 ****************************************************************************************
 * @brief Enable high operating ambient temperature support
 ***************************************************************************************
 */
extern void enable_hoat_support(void);

/**
 ****************************************************************************************
 * @brief DC-DC Enable
 * @param[in]   enable true - enable dc-dc; false - disable
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void dc_dc_enable(bool enable);
#elif defined(QN_9020_B4)
typedef void (*p_dc_dc_enable)(bool enable);
#define dc_dc_enable ((p_dc_dc_enable)(_dc_dc_enable))
#endif

/**
 ****************************************************************************************
 * @brief Set 32k xtal ppm.
 * @param[in]     ppm
 ***************************************************************************************
 */
typedef void (*p_set_32k_ppm)(int32_t ppm);
#define set_32k_ppm ((p_set_32k_ppm)(_set_32k_ppm))

/**
 ****************************************************************************************
 * @brief Set 32k xtal frequency.
 * @param[in]     freq  frequency (Hz)
 ***************************************************************************************
 */
typedef void (*p_set_32k_freq)(int32_t ppm);
#define set_32k_freq ((p_set_32k_freq)(_set_32k_freq))

/**
 ****************************************************************************************
 * @brief Initilaize BLE hardware platform
 * @param[in]   pw_mode             NORMAL_MODE - low power; HIGH_PERFORMANCE - high power
 * @param[in]   xtal                which crystal is used - 16MHz or 32MHz
 * @param[in]   clk_32k             which 32k is used - 0:xtal, 1:rco
 * @param[in]   nvds_tmp_buf        pointer of nvds temp buffer
 * @param[in]   nvds_tmp_buf_len    length of nvds temp buffer
 ****************************************************************************************
 */
extern void plf_init(enum PW_MODE pw_mode, uint32_t xtal, uint8_t clk_32k, uint8_t* nvds_tmp_buf, uint32_t nvds_tmp_buf_len);

/**
 ****************************************************************************************
 * @brief Initialize BLE stack
 * @param[in]   mode            SoC, Network Processor or Controller
 * @param[in]   port            The UART/SPI interface which is used in NP or Controller mode
 * @param[in]   uart_read       The UART/SPI read API
 * @param[in]   uart_write      The UART/SPI write API
 * @param[in]   ble_heap_addr   The start address of BLE heap
 * @param[in]   ble_heap_size   The size of BLE heap
 * @param[in]   sleep_enable    BLE stack sleep enable
 ****************************************************************************************
 */
extern void ble_init(enum WORK_MODE mode, void *port, hci_api hci_read, hci_api hci_write, uint8_t *ble_heap_addr, uint32_t ble_heap_size, bool sleep_enable);

/**
 ****************************************************************************************
 * @brief Configure work mode 
 * @param[in]   mode            Work mode (Soc, Network Processor, Controller)
 * @param[in]   port            The UART/SPI interface which is used in NP or Controller mode
 * @param[in]   uart_read       The UART/SPI read API
 * @param[in]   uart_write      The UART/SPI write API
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void config_work_mode(enum WORK_MODE mode, void *port, hci_api hci_read, hci_api hci_write);
#elif defined(QN_9020_B4)
typedef void (*p_config_work_mode)(enum WORK_MODE mode, void *port, hci_api hci_read, hci_api hci_write);
#define config_work_mode ((p_config_work_mode)(_config_work_mode))
#endif

/**
 ****************************************************************************************
 * @brief Register EACI tx done function
 * @param[in]   p_eaci_tx_done  The pointer of eaci_tx_done()
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void reg_eaci_tx_done(void (*p_eaci_tx_done)(void));
#elif defined(QN_9020_B4)
typedef void (*p_reg_eaci_tx_done)(void (*p_eaci_tx_done)(void));
#define reg_eaci_tx_done ((p_reg_eaci_tx_done)(_reg_eaci_tx_done))
#endif

/**
 ****************************************************************************************
 * @brief Scheduler
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void ke_schedule(void);
#elif defined(QN_9020_B4)
typedef void (*p_ke_schedule)(void);;
#define ke_schedule ((p_ke_schedule)(_ke_schedule))
#endif

/**
 ****************************************************************************************
 * @brief Allow or prevent from the BLE hardware going to sleep
 * @param[in]   enable  ture - allow; false - prevent
 ****************************************************************************************
 */
typedef void (*p_enable_ble_sleep)(bool enable);
#define enable_ble_sleep ((p_enable_ble_sleep)(_enable_ble_sleep))

/**
 ****************************************************************************************
 * @brief Set maximum sleep duration of BLE sleep timer. If the BLE stack works, it will
 *        revises the sleep duration based on BLE stack's requirement, otherwise this
 *        sleep duration will be the setting value.
 * @param[in]   duration    unit 625us, maximum is 209715199(36hours16mins)
 ****************************************************************************************
 */
typedef bool (*p_set_max_sleep_duration)(uint32_t duration);
#define set_max_sleep_duration ((p_set_max_sleep_duration)(_set_max_sleep_duration))

/**
 ****************************************************************************************
 * @brief Set BLE program latency. The software should program BLE hardware before 
 *        the BLE hardware handle BLE event. This value specifices this duration.
 * @param[in]   latency    0 < latency <= 8, unit 625us, default value is 4
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void set_ble_program_latency(uint8_t latency);
#elif defined(QN_9020_B4)
typedef void (*p_set_ble_program_latency)(uint8_t latency);
#define set_ble_program_latency ((p_set_ble_program_latency)(_set_ble_program_latency))
#endif

/**
 ***************************************************************
 * @brief Check the sleep status of ble hardware.
 ***************************************************************
*/
#if defined(QN_9020_B2)
extern bool ble_hw_sleep(void);
#elif defined(QN_9020_B4)
typedef bool (*p_ble_hw_sleep)(void);
#define ble_hw_sleep ((p_ble_hw_sleep)(_ble_hw_sleep))
#endif

/**
 ***************************************************************
 * @brief Check whether to do BLE external wakeup.
 ***************************************************************
*/
#if defined(QN_9020_B2)
extern bool ble_ext_wakeup_allow(void);
#elif defined(QN_9020_B4)
typedef bool (*p_ble_ext_wakeup_allow)(void);
#define ble_ext_wakeup_allow ((p_ble_ext_wakeup_allow)(_ble_ext_wakeup_allow))
#endif

/**
 ***************************************************************
 * @brief Wakeup BLE hardware by software.
 ***************************************************************
*/
typedef void (*p_sw_wakeup_ble_hw)(void);
#define sw_wakeup_ble_hw ((p_sw_wakeup_ble_hw)(_sw_wakeup_ble_hw))

/**
 ****************************************************************************************
 * @brief Register sleep callback.
 *        enter_cb is invoked before BLE entering sleep mode. If the return of
 *        callback function is FALSE, the BLE hardware will not enter into sleep mode.
 *        exit_cb provides user a way to do sth after ble hardware wakeup.
 * @param[in]     enter_cb   Callback function 
 * @param[in]     exit_cb    Callback function
 ***************************************************************************************
 */
#if defined(QN_9020_B2)
extern void reg_ble_sleep_cb(bool (*enter_cb)(void), void (*exit_cb)(void));
#elif defined(QN_9020_B4)
typedef void (*p_reg_ble_sleep_cb)(bool (*enter_cb)(void), void (*exit_cb)(void));
#define reg_ble_sleep_cb ((p_reg_ble_sleep_cb)(_reg_ble_sleep_cb))
#endif

/**
 ****************************************************************************************
 * @brief Save configuration which will lose in sleep mode
 ****************************************************************************************
 */
typedef void (*p_save_ble_setting)(void);
#define save_ble_setting ((p_save_ble_setting)(_save_ble_setting))

/**
 ****************************************************************************************
 * @brief Restore configuration which is saved before entering sleep mode
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void restore_ble_setting(void);
#elif defined (QN_9020_B4)
typedef void (*p_restore_ble_setting)(void);
#define restore_ble_setting ((p_restore_ble_setting)(_restore_ble_setting))
#endif

/**
 ****************************************************************************************
 * @brief Sleep post process
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void sleep_post_process(void);
#elif defined(QN_9020_B4)
typedef void (*p_sleep_post_process)(void);
#define sleep_post_process ((p_sleep_post_process)(_sleep_post_process))
#endif


/**
 ****************************************************************************************
 * @brief Check that BLE has already waked-up
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern uint32_t check_ble_wakeup(void);
#elif defined(QN_9020_B4)
typedef uint32_t (*p_check_ble_wakeup)(void);
#define check_ble_wakeup ((p_check_ble_wakeup)(_check_ble_wakeup))
#endif

/**
 ****************************************************************************************
 * @brief Set events
 *
 * This primitive sets one or more events in the event field variable.
 * The corresponding event handler set by ke_evt_callback_set() will be invoked
 * in the scheduler.
 *
 * @param[in]  event       Events that have to be set (bit field).
 *
 ****************************************************************************************
 */
typedef void (*p_ke_evt_set)(uint32_t const);
#define ke_evt_set ((p_ke_evt_set)(_ke_evt_set))

/**
 ****************************************************************************************
 * @brief Clear events
 *
 * This primitive clears one or more events in the event field variable.
 * Generally when the event handler is invoked, the corresponding event bit should
 * be clear in the event handler to prevent reentry.
 *
 * @param[in]  event       Events that have to be cleared (bit field).
 *
 ****************************************************************************************
 */
typedef void (*p_ke_evt_clear)(uint32_t const);
#define ke_evt_clear ((p_ke_evt_clear)(_ke_evt_clear))

/**
 ****************************************************************************************
 * @brief Register an event callback.
 *
 * When the scheduler is called in the main loop of the background, the kernel
 * checks if the event field is non-null, gets the one with highest priority and
 * executes the event handlers for which the corresponding event bit is set.
 *
 * There are total 32 events, and the highest priority events are used by BLE stack.
 * So users have 24 events could be used in the application. The most significant bit
 * has the highest priority. The LSB has the lowest priority.
 *
 * @param[in]  event_type       Event type. (user can use only 0 ~ 23)
 * @param[in]  p_callback       Pointer to callback function.
 *
 * @return                      Status
 ****************************************************************************************
 */
typedef enum KE_EVENT_STATUS (*p_ke_evt_callback_set)(uint8_t event_type, void (*p_callback)(void));
#define ke_evt_callback_set ((p_ke_evt_callback_set)(_ke_evt_callback_set))

/**
 ****************************************************************************************
 * @brief Get system tick count, unit is 10ms
 ****************************************************************************************
 */
extern uint32_t ke_time(void);

/**
 ****************************************************************************************
 * @brief Get ke timer queue status, TRUE means empty
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern bool ke_timer_empty(void);
#elif defined(QN_9020_B4)
typedef bool (*p_ke_timer_empty)(void);
#define ke_timer_empty ((p_ke_timer_empty)(_ke_timer_empty))
#endif

/**
 ****************************************************************************************
 * @brief Get the status of BLE event, TRUE means no BLE event
 *        is running.
 ****************************************************************************************
 */
extern bool ble_evt_empty(void);

/**
 ****************************************************************************************
 * @brief set debug bit, the debug information is saved at 0x1000fffc.
 * @param[in]  dbg_info_bit     Debug information
 ****************************************************************************************
 */
#if defined(QN_9020_B2)
extern void set_dbg_info(uint32_t dbg_info_bit);
#elif defined(QN_9020_B4)
typedef void (*p_set_dbg_info)(uint32_t dbg_info_bit);
#define set_dbg_info ((p_set_dbg_info)(_set_dbg_info))
#endif

/**
 ****************************************************************************************
 * @brief FCC/CE Tx Test
 * @param[in]  freq         frequency
 * @param[in]  enable_mod   0-disable modulation 1-enable modulation
 * @param[in]  txpwr        TX power
 * @param[in]  data_len     TX test data length
 * @param[in]  payload_type TX test payload type
 ****************************************************************************************
 */
extern void fcc_ce_tx_test(uint32_t freq, uint32_t enable_mod, enum TX_POWER txpwr, uint8_t data_len, uint8_t payload_type);

/**
 ****************************************************************************************
 * @brief FCC/CE Rx Test
 * @param[in]  freq         frequency
 ****************************************************************************************
 */
extern void fcc_ce_rx_test(uint32_t freq);

#if (QN_MULTI_NOTIFICATION_IN_ONE_EVENT || QN_READ_INDICATION || QN_SLAVE_LATENCY_IMPROVEMENT)
/**
 ****************************************************************************************
 * @brief Configure new features of ble stack
 * @param[in] ctrl  Configuration information
 ****************************************************************************************
 */
extern void qn_feature_config(struct qn_feature_ctrl *ctrl);

/**
 ****************************************************************************************
 * @brief Patches in the library file, and these functions will
 *        be remove by compiler if not used by application.
 ****************************************************************************************
 */
extern int atts_hdl_val_ntf_req_handler_patch(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int l2cc_data_send_rsp_att_handler_patch(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int l2cc_data_send_rsp_handler_patch(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int atts_l2cc_data_packet_ind_handler_patch(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int gatt_read_req_cfm_handler_patch(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int llcp_con_up_req_handler_patch(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int llcp_channel_map_req_handler_patch(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern void lld_evt_schedule_patch(void);
extern void lld_evt_end_patch(void);
extern void lld_evt_restart_patch(void *p_evt);
extern void lld_evt_schedule_next(void *p_evt);
#endif

/**
 ****************************************************************************************
 * @brief set seed for rand lib
 * @param[in]  seed     Real random number
 ****************************************************************************************
 */
typedef void (*p_srand)(uint32_t seed);
#define fw_srand ((p_srand)(_srand))
/**
 ****************************************************************************************
 * @brief       Used to store user's gap mode config parameter
 * @param[in]  mode      user's gap mode config parameter
 ****************************************************************************************
 */
void store_ble_dev_mode_flag(uint16_t mode);
/**
 ****************************************************************************************
 * @brief Set GAP_BONDABLE or GAP_NON_BONDABLE as user setted before
 *
 ****************************************************************************************
 */ 
void restore_ble_dev_mode_flag(void);
#endif




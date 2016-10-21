/**
 ****************************************************************************************
 *
 * @file paspc.c
 *
 * @brief Phone Alert Status Profile Client implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup PASPC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 #include "app_config.h"
#if (BLE_PAS_CLIENT)

#include "paspc.h"
#include "paspc_task.h"

/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */

struct paspc_env_tag **paspc_envs;

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void paspc_init(void)
{
    task_paspc_desc_register();
    // Reset all the profile role tasks
    PRF_CLIENT_RESET(paspc_envs, PASPC);
}

void paspc_send_cmp_evt(uint16_t src_id, uint16_t dest_id, uint16_t conhdl,
                        uint8_t operation, uint8_t status)
{
    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(src_id) == PASPC_BUSY)
    {
        ke_state_set(src_id, PASPC_CONNECTED);
    }

    // Send the message
    struct paspc_cmp_evt *evt = KE_MSG_ALLOC(PASPC_CMP_EVT,
                                             dest_id, src_id,
                                             paspc_cmp_evt);

    evt->conhdl     = conhdl;
    evt->operation  = operation;
    evt->status     = status;

    ke_msg_send(evt);
}

#endif //(BLE_PAS_CLIENT)

/// @} PASPC

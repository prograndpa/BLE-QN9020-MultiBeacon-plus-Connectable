/**
 ****************************************************************************************
 *
 * @file anpc.c
 *
 * @brief Alert Notification Profile Client implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup ANPC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "app_config.h"

#if (BLE_AN_CLIENT)
#include "anpc.h"
#include "anpc_task.h"

/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */

struct anpc_env_tag **anpc_envs;

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void anpc_init(void)
{
    task_anpc_desc_register();
    // Reset all the profile role tasks
    PRF_CLIENT_RESET(anpc_envs, ANPC);
}

bool anpc_found_next_alert_cat(struct anpc_env_tag *idx_env, uint8_t alert_type, struct anp_cat_id_bit_mask cat_id)
{
    // Next Category Found ?
    bool found = false;

    if (idx_env->last_req < CAT_ID_HIGH_PRTY_ALERT)
    {
        // Look in the first part of the categories
        while ((idx_env->last_req < CAT_ID_HIGH_PRTY_ALERT) && (!found))
        {
            if (((cat_id.cat_id_mask_0 >> idx_env->last_req) & 1) == 1)
            {
                found = true;
            }
            else
            {
                idx_env->last_req++;
            }
        }
    }

    if (idx_env->last_req >= CAT_ID_HIGH_PRTY_ALERT)
    {
        // Look in the first part of the categories
        while ((idx_env->last_req < CAT_ID_NB) && (!found))
        {
            if (((cat_id.cat_id_mask_1 >> idx_env->last_req) & 1) == 1)
            {
                found = true;
            }
            else
            {
                idx_env->last_req++;
            }
        }
    }

    return found;
}

void anpc_write_alert_ntf_ctnl_pt(struct anpc_env_tag *idx_env, uint8_t cmd_id, uint8_t cat_id)
{
    struct anp_ctnl_pt ctnl_pt = {cmd_id, cmd_id};

    // Send the write request
    prf_gatt_write(&(idx_env->con_info),
                   idx_env->ans.chars[ANPC_CHAR_ALERT_NTF_CTNL_PT].val_hdl,
                   (uint8_t *)&ctnl_pt, sizeof(struct anp_ctnl_pt), GATT_WRITE_CHAR);
}

void anpc_send_no_conn_cmp_evt(uint8_t src_id, uint8_t dest_id, uint16_t conhdl, uint8_t operation)
{
    // Send the message
    struct anpc_cmp_evt *evt = KE_MSG_ALLOC(ANPC_CMP_EVT,
                                            dest_id, src_id,
                                            anpc_cmp_evt);

    evt->conhdl     = conhdl;
    evt->operation  = operation;
    evt->status     = PRF_ERR_REQ_DISALLOWED;

    ke_msg_send(evt);
}

void anpc_send_cmp_evt(struct anpc_env_tag *anpc_env, uint8_t operation, uint8_t status)
{
    // Free the stored operation if needed
    if (anpc_env->operation != NULL)
    {
        ke_msg_free(ke_param2msg(anpc_env->operation));
        anpc_env->operation = NULL;
    }

    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(anpc_env->con_info.prf_id) == ANPC_BUSY)
    {
        ke_state_set(anpc_env->con_info.prf_id, ANPC_CONNECTED);
    }

    // Send the message
    struct anpc_cmp_evt *evt = KE_MSG_ALLOC(ANPC_CMP_EVT,
                                            anpc_env->con_info.appid, anpc_env->con_info.prf_id,
                                            anpc_cmp_evt);

    evt->conhdl     = anpc_env->con_info.conhdl;
    evt->operation  = operation;
    evt->status     = status;

    ke_msg_send(evt);
}

#endif //(BLE_AN_CLIENT)

/// @} ANPC

/**
 ****************************************************************************************
 *
 * @file hogpd.c
 *
 * @brief HID Over GATT Profile HID Device Role Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev$
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HOGPD
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "app_config.h"

#if (BLE_HID_DEVICE)

#include "gap.h"
#include "gatt_task.h"
#include "atts_util.h"
#include "hogpd.h"
#include "hogpd_task.h"

/*
 * HIDS ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// Full HIDS Database Description - Used to add attributes into the database
const struct atts_desc hids_att_db[HOGPD_IDX_NB] =
{
    // HID Service Declaration
    [HOGPD_IDX_SVC]                             = {ATT_DECL_PRIMARY_SERVICE,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hid_svc), sizeof(hid_svc),
                                                   (uint8_t *)&hid_svc},

    // HID Service Declaration
    [HOGPD_IDX_INCL_SVC]                        = {ATT_DECL_INCLUDE,
                                                   PERM(RD, ENABLE),
                                                   sizeof(struct atts_incl_desc), 0,
                                                   NULL},

    // HID Information Characteristic Declaration
    [HOGPD_IDX_HID_INFO_CHAR]                   = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_hid_info_char), sizeof(hids_hid_info_char),
                                                   (uint8_t *)&hids_hid_info_char},
    // HID Information Characteristic Value
    [HOGPD_IDX_HID_INFO_VAL]                    = {ATT_CHAR_HID_INFO,
                                                   PERM(RD, ENABLE),
                                                   sizeof(struct hids_hid_info), 0,
                                                   NULL},

    // HID Control Point Characteristic Declaration
    [HOGPD_IDX_HID_CTNL_PT_CHAR]                = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_hid_ctnl_pt_char), sizeof(hids_hid_ctnl_pt_char),
                                                   (uint8_t *)&hids_hid_ctnl_pt_char},
    // HID Control Point Characteristic Value
    [HOGPD_IDX_HID_CTNL_PT_VAL]                 = {ATT_CHAR_HID_CTNL_PT,
                                                   PERM(WR, ENABLE),
                                                   sizeof(uint8_t), 0,
                                                   NULL},

    // Report Map Characteristic Declaration
    [HOGPD_IDX_REPORT_MAP_CHAR]                 = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_report_map_char), sizeof(hids_report_map_char),
                                                   (uint8_t *)&hids_report_map_char},
    // Report Map Characteristic Value
    [HOGPD_IDX_REPORT_MAP_VAL]                  = {ATT_CHAR_REPORT_MAP,
                                                   PERM(RD, ENABLE),
                                                   HOGPD_REPORT_MAP_MAX_LEN*sizeof(uint8_t), 0,
                                                   NULL},
    // Report Map Characteristic - External Report Reference Descriptor
    [HOGPD_IDX_REPORT_MAP_EXT_REP_REF]          = {ATT_DESC_EXT_REPORT_REF,
                                                   PERM(RD, ENABLE),
                                                   sizeof(uint16_t), 0,
                                                   NULL},

    // Protocol Mode Characteristic Declaration
    [HOGPD_IDX_PROTO_MODE_CHAR]                 = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_proto_mode_char), sizeof(hids_proto_mode_char),
                                                   (uint8_t *)&hids_proto_mode_char},
    // Protocol Mode Characteristic Value
    [HOGPD_IDX_PROTO_MODE_VAL]                  = {ATT_CHAR_PROTOCOL_MODE,
                                                   (PERM(RD, ENABLE) | PERM(WR, ENABLE)),
                                                   sizeof(uint8_t), 0,
                                                   NULL},

    // Boot Keyboard Input Report Characteristic Declaration
    [HOGPD_IDX_BOOT_KB_IN_REPORT_CHAR]          = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_boot_kb_in_report_char), sizeof(hids_boot_kb_in_report_char),
                                                   (uint8_t *)&hids_boot_kb_in_report_char},
    // Boot Keyboard Input Report Characteristic Value
    [HOGPD_IDX_BOOT_KB_IN_REPORT_VAL]           = {ATT_CHAR_BOOT_KB_IN_REPORT,
                                                   (PERM(RD, ENABLE) | PERM(NTF, ENABLE)),
                                                   HOGPD_BOOT_REPORT_MAX_LEN*sizeof(uint8_t), 0,
                                                   NULL},
    // Boot Keyboard Input Report Characteristic - Client Characteristic Configuration Descriptor
    [HOGPD_IDX_BOOT_KB_IN_REPORT_NTF_CFG]       = {ATT_DESC_CLIENT_CHAR_CFG,
                                                   (PERM(RD, ENABLE) | PERM(WR, ENABLE)),
                                                   sizeof(uint16_t), 0,
                                                   NULL},

    // Boot Keyboard Output Report Characteristic Declaration
    [HOGPD_IDX_BOOT_KB_OUT_REPORT_CHAR]         = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_boot_kb_out_report_char), sizeof(hids_boot_kb_out_report_char),
                                                   (uint8_t *)&hids_boot_kb_out_report_char},
    // Boot Keyboard Output Report Characteristic Value
    [HOGPD_IDX_BOOT_KB_OUT_REPORT_VAL]          = {ATT_CHAR_BOOT_KB_OUT_REPORT,
                                                   (PERM(RD, ENABLE) | PERM(WR, ENABLE)),
                                                   HOGPD_BOOT_REPORT_MAX_LEN*sizeof(uint8_t), 0,
                                                   NULL},

    // Boot Mouse Input Report Characteristic Declaration
    [HOGPD_IDX_BOOT_MOUSE_IN_REPORT_CHAR]       = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_boot_mouse_in_report_char), sizeof(hids_boot_mouse_in_report_char),
                                                   (uint8_t *)&hids_boot_mouse_in_report_char},
    // Boot Mouse Input Report Characteristic Value
    [HOGPD_IDX_BOOT_MOUSE_IN_REPORT_VAL]        = {ATT_CHAR_BOOT_MOUSE_IN_REPORT,
                                                   (PERM(RD, ENABLE) | PERM(NTF, ENABLE)),
                                                   HOGPD_BOOT_REPORT_MAX_LEN*sizeof(uint8_t), 0,
                                                   NULL},
    // Boot Mouse Input Report Characteristic - Client Characteristic Configuration Descriptor
    [HOGPD_IDX_BOOT_MOUSE_IN_REPORT_NTF_CFG]    = {ATT_DESC_CLIENT_CHAR_CFG,
                                                   (PERM(RD, ENABLE) | PERM(WR, ENABLE)),
                                                   sizeof(uint16_t), 0,
                                                   NULL},

    // Report Characteristic Declaration
    [HOGPD_IDX_REPORT_CHAR]                     = {ATT_DECL_CHARACTERISTIC,
                                                   PERM(RD, ENABLE),
                                                   sizeof(hids_report_char), sizeof(hids_report_char),
                                                   (uint8_t *)&hids_report_char},
    // Report Characteristic Value
    [HOGPD_IDX_REPORT_VAL]                      = {ATT_CHAR_REPORT,
                                                   PERM(RD, ENABLE),
                                                   HOGPD_REPORT_MAX_LEN*sizeof(uint8_t), 0,
                                                   NULL},
    // Report Characteristic - Report Reference Descriptor
    [HOGPD_IDX_REPORT_REP_REF]                  = {ATT_DESC_REPORT_REF,
                                                   PERM(RD, ENABLE),
                                                   sizeof(struct hids_report_ref), 0,
                                                   NULL},
    // Report Characteristic - Client Characteristic Configuration Descriptor
    [HOGPD_IDX_REPORT_NTF_CFG]                  = {ATT_DESC_CLIENT_CHAR_CFG,
                                                   (PERM(RD, ENABLE) | PERM(WR, ENABLE)),
                                                   sizeof(uint16_t), 0,
                                                   NULL},
};

/// HID Service
const atts_svc_desc_t hid_svc = ATT_SVC_HID;

/// HID Information Characteristic
const struct atts_char_desc hids_hid_info_char      = ATTS_CHAR(ATT_CHAR_PROP_RD,
                                                                HOGPD_HID_INFO_CHAR,
                                                                ATT_CHAR_HID_INFO);

/// HID Control Point Characteristic
const struct atts_char_desc hids_hid_ctnl_pt_char   = ATTS_CHAR(ATT_CHAR_PROP_WR_NO_RESP,
                                                                HOGPD_HID_CTNL_PT_CHAR,
                                                                ATT_CHAR_HID_CTNL_PT);

/// Report Map Characteristic
const struct atts_char_desc hids_report_map_char    = ATTS_CHAR(ATT_CHAR_PROP_RD,
                                                                HOGPD_REPORT_MAP_CHAR,
                                                                ATT_CHAR_REPORT_MAP);

/// Protocol Mode Characteristic
const struct atts_char_desc hids_proto_mode_char    = ATTS_CHAR((ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR_NO_RESP),
                                                                HOGPD_PROTO_MODE_CHAR,
                                                                ATT_CHAR_PROTOCOL_MODE);

/// Boot Keyboard Input Report Characteristic
const struct atts_char_desc hids_boot_kb_in_report_char
                                                    = ATTS_CHAR((ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF),
                                                                 HOGPD_BOOT_KB_IN_REPORT_CHAR,
                                                                 ATT_CHAR_BOOT_KB_IN_REPORT);

/// Boot Keyboard Output Report Characteristic
const struct atts_char_desc hids_boot_kb_out_report_char
                                                    = ATTS_CHAR((ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR | ATT_CHAR_PROP_WR_NO_RESP),
                                                                 HOGPD_BOOT_KB_OUT_REPORT_CHAR,
                                                                 ATT_CHAR_BOOT_KB_OUT_REPORT);

/// Boot Mouse Input Report Characteristic
const struct atts_char_desc hids_boot_mouse_in_report_char
                                                    = ATTS_CHAR((ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF),
                                                                HOGPD_BOOT_MOUSE_IN_REPORT_CHAR,
                                                                ATT_CHAR_BOOT_MOUSE_IN_REPORT);

/// Report Characteristic
const struct atts_char_desc hids_report_char        = ATTS_CHAR(ATT_CHAR_PROP_RD,
                                                                HOGPD_REPORT_CHAR,
                                                                ATT_CHAR_REPORT);

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

struct hogpd_env_tag hogpd_env;

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void hogpd_init(void)
{
    // Reset the find me target environment
    memset(&hogpd_env, 0, sizeof(hogpd_env));
    
    // Register HOGPD task into kernel
    task_hogpd_desc_register();

    // Go to IDLE state
    ke_state_set(TASK_HOGPD, HOGPD_DISABLED);
}

void hogpd_init_ntf_cfg(uint16_t ntf_cfg, uint16_t handle, uint8_t con_type,
                        uint8_t *flag, uint8_t mask)
{
    // Default Ntf Cfg value
    uint16_t value = PRF_CLI_STOP_NTFIND;

    //Written value is 0 is discovery connection, given value if normal
    if(con_type == PRF_CON_NORMAL)
    {
        memcpy(&value, &ntf_cfg, sizeof(uint16_t));

        // Save new status in the environment
        if (value == PRF_CLI_START_NTF)
        {
            *flag |= HOGPD_REPORT_NTF_CFG_MASK;
        }
    }

    attsdb_att_set_value(handle, sizeof(uint16_t), (uint8_t *)&value);
}

uint8_t hogpd_ntf_send(uint8_t hids_nb, uint8_t char_code, uint8_t report_nb,
                       uint16_t report_len, uint8_t *p_report)
{
    // Status
    uint8_t status = PRF_ERR_OK;
    // Handle
    uint16_t handle;
    // Mask
    uint8_t mask;
    // Flag
    uint8_t flag;

    // Check if the required Report Char. is supported
    if (hogpd_env.att_tbl[hids_nb][char_code + report_nb] != 0x00)
    {
        handle = hogpd_env.shdl[hids_nb] + hogpd_env.att_tbl[hids_nb][char_code + report_nb] + 1;

        // Set value in the database
        attsdb_att_set_value(handle, report_len, p_report);

        // Check if notifications have been enabled for the required characteristic
        switch(char_code)
        {
            case HOGPD_REPORT_CHAR:
                flag = hogpd_env.features[hids_nb].report_char_cfg[report_nb];
                mask = HOGPD_REPORT_NTF_CFG_MASK;
                break;
            case HOGPD_BOOT_KB_IN_REPORT_CHAR:
                flag = hogpd_env.features[hids_nb].svc_features;
                mask = HOGPD_BOOT_KB_IN_NTF_CFG_MASK;
                break;
            case HOGPD_BOOT_MOUSE_IN_REPORT_CHAR:
                flag = hogpd_env.features[hids_nb].svc_features;
                mask = HOGPD_BOOT_MOUSE_IN_NTF_CFG_MASK;
                break;
            default:
                return PRF_ERR_INVALID_PARAM;
        }

        if ((flag & mask) != mask)
        {
            status = PRF_ERR_NTF_DISABLED;
        }

        // Check if notifications are enabled
        if (status == PRF_ERR_OK)
        {
            // Send notification through GATT
            struct gatt_notify_req * ntf = KE_MSG_ALLOC(GATT_NOTIFY_REQ, TASK_GATT,
                                                        TASK_HOGPD, gatt_notify_req);

            ntf->conhdl  = hogpd_env.con_info.conhdl;
            ntf->charhdl = handle;

            ke_msg_send(ntf);
        }
    }
    else
    {
        status = PRF_ERR_FEATURE_NOT_SUPPORTED;
    }

    return status;
}

void hogpd_ntf_cfm_send(uint8_t status, uint8_t char_code, uint8_t hids_nb, uint8_t report_nb)
{
    struct hogpd_ntf_sent_cfm *cfm = KE_MSG_ALLOC(HOGPD_NTF_SENT_CFM, hogpd_env.con_info.appid,
                                                  TASK_HOGPD, hogpd_ntf_sent_cfm);

    cfm->conhdl    = hogpd_env.con_info.conhdl;
    cfm->hids_nb   = hids_nb;
    cfm->report_nb = report_nb;
    cfm->char_code = char_code;
    cfm->status    = status;

    ke_msg_send(cfm);
}

uint8_t hogpd_ntf_cfg_ind_send(uint16_t ntf_cfg, uint16_t handle, uint8_t cfg_code,
                               uint8_t hids_nb, uint8_t report_nb)
{
    // Status
    uint8_t status = PRF_APP_ERROR;
    // Mask
    uint8_t mask = 0x00;
    // Pointer to the flag saving the notification configuration
    uint8_t *flag = &hogpd_env.features[hids_nb].svc_features;

    // Check if provided value is correct
    if ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF))
    {
        // Set value in the database
        attsdb_att_set_value(handle, sizeof(uint16_t), (uint8_t *)&ntf_cfg);

        if (cfg_code == HOGPD_BOOT_KB_IN_REPORT_CFG)
        {
            mask = HOGPD_BOOT_KB_IN_NTF_CFG_MASK;
        }
        else if (cfg_code == HOGPD_BOOT_MOUSE_IN_REPORT_CFG)
        {
            mask = HOGPD_BOOT_MOUSE_IN_NTF_CFG_MASK;
        }
        else if (cfg_code == HOGPD_REPORT_CFG)
        {
            mask = HOGPD_REPORT_NTF_CFG_MASK;
            flag = &hogpd_env.features[hids_nb].report_char_cfg[report_nb];
        }

        // Save new status in the environment
        if (ntf_cfg == PRF_CLI_START_NTF)
        {
            *flag |= mask;
        }
        else
        {
            *flag &= ~mask;
        }

        // Inform APP of configuration change
        struct hogpd_ntf_cfg_ind * ind = KE_MSG_ALLOC(HOGPD_NTF_CFG_IND,
                                                      hogpd_env.con_info.appid, TASK_HOGPD,
                                                      hogpd_ntf_cfg_ind);

        co_write16p(&ind->conhdl, hogpd_env.con_info.conhdl);
        co_write16p(&ind->ntf_en, ntf_cfg);
        ind->hids_nb   = hids_nb;
        ind->report_nb = report_nb;
        ind->cfg_code  = cfg_code;

        ke_msg_send(ind);

        status = PRF_ERR_OK;
    }

    return status;
}

uint8_t hogpd_get_att(uint16_t handle, uint8_t *char_code, uint8_t *hids_nb, uint8_t *report_nb)
{
    // Status, attribute found or not
    uint8_t found = PRF_APP_ERROR;
    // Counters
    uint8_t svc, att;
    // Offset
    uint8_t offset;

    for (svc = 0; ((svc < hogpd_env.hids_nb) && (found == PRF_APP_ERROR)); svc++)
    {
        *hids_nb = svc;
        offset = handle - hogpd_env.shdl[svc];

        for (att = HOGPD_HID_INFO_CHAR; ((att < HOGPD_CHAR_MAX) && (found == PRF_APP_ERROR)); att++)
        {
            // Characteristic Value Attribute
            if (offset == (hogpd_env.att_tbl[svc][att] + 1))
            {
                *char_code = att;

                if (att >= HOGPD_REPORT_CHAR)
                {
                    *report_nb = att - HOGPD_REPORT_CHAR;
                    *char_code = HOGPD_REPORT_CHAR;
                }

                found = PRF_ERR_OK;
            }

            if (found == PRF_APP_ERROR)
            {
                if ((att == HOGPD_BOOT_KB_IN_REPORT_CHAR) || (att == HOGPD_BOOT_MOUSE_IN_REPORT_CHAR))
                {
                    if (offset == hogpd_env.att_tbl[svc][att] + 2)
                    {
                        *report_nb = 0;
                        *char_code = att | HOGPD_DESC_MASK;

                        found = PRF_ERR_OK;
                    }
                }
            }

            if (found == PRF_APP_ERROR)
            {
                if (att >= HOGPD_REPORT_CHAR)
                {
                    if (offset == hogpd_env.att_tbl[svc][att] + 3)
                    {
                        *report_nb = att - HOGPD_REPORT_CHAR;
                        *char_code = (uint8_t)(HOGPD_REPORT_CHAR | HOGPD_DESC_MASK);

                        found = PRF_ERR_OK;
                    }
                }
            }
        }
    }

    return found;
}

void hogpd_disable(void)
{
    // HIDS instance counter
    uint8_t hids_nb;
    // Report Char. instance counter
    uint8_t report_nb;

    struct hogpd_disable_ind *ind = KE_MSG_ALLOC(HOGPD_DISABLE_IND,
                                                 hogpd_env.con_info.appid, TASK_HOGPD,
                                                 hogpd_disable_ind);

    ind->conhdl = hogpd_env.con_info.conhdl;

    for (hids_nb = 0; hids_nb < hogpd_env.hids_nb; hids_nb++)
    {
        // Disable HIDS in database
        attsdb_svc_set_permission(hogpd_env.shdl[hids_nb], PERM_RIGHT_DISABLE);

        // Save Boot Keyboard Input Report Char. Ntf Cfg
        if (hogpd_env.att_tbl[hids_nb][HOGPD_BOOT_KB_IN_REPORT_CHAR] != 0x00)
        {
            if ((hogpd_env.features[hids_nb].svc_features & HOGPD_BOOT_KB_IN_NTF_CFG_MASK)
                                                         == HOGPD_BOOT_KB_IN_NTF_CFG_MASK)
            {
                ind->ntf_cfg[hids_nb].boot_kb_in_report_ntf_en = PRF_CLI_START_NTF;
            }
        }

        // Save Boot Mouse Input Report Char. Ntf Cfg
        if (hogpd_env.att_tbl[hids_nb][HOGPD_BOOT_MOUSE_IN_REPORT_CHAR] != 0x00)
        {
            if ((hogpd_env.features[hids_nb].svc_features & HOGPD_BOOT_MOUSE_IN_NTF_CFG_MASK)
                                                         == HOGPD_BOOT_MOUSE_IN_NTF_CFG_MASK)
            {
                ind->ntf_cfg[hids_nb].boot_mouse_in_report_ntf_en = PRF_CLI_START_NTF;
            }
        }

        //Reset notifications bit field
        hogpd_env.features[hids_nb].svc_features &= ~(HOGPD_BOOT_KB_IN_NTF_CFG_MASK | HOGPD_BOOT_MOUSE_IN_NTF_CFG_MASK);

        // Save Report Char. Ntf Cfg
        for (report_nb = 0; report_nb < hogpd_env.features[hids_nb].report_nb; report_nb++)
        {
            if ((hogpd_env.features[hids_nb].report_char_cfg[report_nb] & HOGPD_REPORT_NTF_CFG_MASK)
                                                                       == HOGPD_REPORT_NTF_CFG_MASK)
            {
                ind->ntf_cfg[hids_nb].report_ntf_en[report_nb] = PRF_CLI_START_NTF;
            }

            //Reset notifications bit field
            hogpd_env.features[hids_nb].report_char_cfg[report_nb] &= ~HOGPD_REPORT_NTF_CFG_MASK;
        }
    }

    ke_msg_send(ind);

    //Go to idle state
    ke_state_set(TASK_HOGPD, HOGPD_IDLE);
}

#endif /* BLE_HID_DEVICE */

/// @} HOGPD

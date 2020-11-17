#pragma once

#include <string>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/hrs.h>

/* Device and advertising parameters */
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define ADV_LEN 12

/* BLE connection */
struct bt_conn *conn;

/* BLE data params */
static uint8 tf_buffer_len = 3;
static uint16_t tf_buffer[] = {1000,2000,3000};

static bool notify = false;


/* TF Custom Service  -- initialise a GUID */
static struct bt_uuid_128 tf_service_uuid = BT_UUID_INIT_128(
    0x8f, 0xe5, 0xb3, 0xd5, 0x2e, 0x7f, 0x4a, 0x98,
    0x2a, 0x48, 0x7a, 0xcc, 0x40, 0xfe, 0x00, 0x00);

/* TF Probabilities characteristic UUID */
static struct bt_uuid_128 tf_prob_char_id = BT_UUID_INIT_128(
	0x19, 0xed, 0x82, 0xae, 0xed, 0x21, 0x4c, 0x9d,
	0x41, 0x45, 0x22, 0x8e, 0x41, 0xfe, 0x00, 0x00);


/* Advertising data */
static uint8_t manuf_data[ADV_LEN] = {
	0x01 /*SKD version */,
	0x83 /* STM32WB - P2P Server 1 */,
	0x00 /* GROUP A Feature  */,
	0x00 /* GROUP A Feature */,
	0x00 /* GROUP B Feature */,
	0x00 /* GROUP B Feature */,
	0x00, /* BLE MAC start -MSB */
	0x00,
	0x00,
	0x00,
	0x00,
	0x00, /* BLE MAC stop */
};

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, manuf_data, ADV_LEN)
};

static void mpu_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);
    notify = value == 1 ? true : false;
	printk("Notifications set to = %d", notify);
}

static ssize_t read_tflite(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr, void *buf,
			     uint16_t len, uint16_t offset)
{

	uint16_t *value = (uint16_t*)attr->user_data;
    printk("Read callback called with: value %d, len %d, offset %d \n", value, len, offset);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(*value)*3);
}

BT_GATT_SERVICE_DEFINE(stsensor_svc,
    BT_GATT_PRIMARY_SERVICE(&tf_service_uuid),
    BT_GATT_CHARACTERISTIC(
        &tf_prob_char_id.uuid, 
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        read_tflite,
        NULL,
        &tf_buffer
        ),
    BT_GATT_CCC(mpu_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);


/* Function which starts advertising */
static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)", err);
        return;
    }
    printk("Bluetoth Initialised!");
    /* Manually create advertising params - MACRO FAILS C++ */
    struct bt_le_adv_param my_param = {
        .id = BT_ID_DEFAULT,
        .sid = 0,
        .options = BT_LE_ADV_OPT_CONNECTABLE, 
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_2, 
        .peer = NULL,
    };
    /* Start Advertising */
    err = bt_le_adv_start(
        &my_param,
        ad,
        ARRAY_SIZE(ad),
        NULL,
        0
    );
    if (err) {
        printk("Advertising failed to start (err %d)", err);
        return;
    }
    printk("Advertising Started : Waiting For Connections");
}

/* Define Connected and Disconnected Callback */

static void connected(struct bt_conn *connected, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)", err);
	} else {
		printk("Connected");
		if (!conn) {
			conn = bt_conn_ref(connected);
		}
	}
}

static void disconnected(struct bt_conn *disconn, uint8_t reason)
{
	if (conn) {
		bt_conn_unref(conn);
		conn = NULL;
	}

	printk("Disconnected (reason %u)", reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};
#include <zephyr.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <stdio.h>
#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <drivers/uart.h>

//Define the device
#define BLE_UART "UART_0"
static struct device *uart_dev;

#define UART_BUF_SIZE 1024
#define EnableRuuvi 11
#define DisableRuuvi 10
#define EnableEddy 21
#define DisableEddy 20
#define EnableIbeacon 31
#define DisableIbeacon 30
#define EnableUnknown 41
#define DisableUnknown 40

static volatile bool data_transmitted;
static volatile bool data_received;
static int char_sent;
char rx_buf[UART_BUF_SIZE];
char ble_data[512];

static volatile bool SCAN_RUUVI= true;
static volatile bool SCAN_EDDY= false;
static volatile bool SCAN_iBeacon= false;
static volatile bool SCAN_unknown= false;
#define BT_ADDR_LEN 13

static void uart_data_parse(char *msg_orig){
    
    char *msg = strdup(msg_orig);
	int cmd = atoi(msg);
	if(cmd==EnableRuuvi){
		SCAN_RUUVI= true;
		goto end;
	}
	else if(cmd==DisableRuuvi){
		SCAN_RUUVI= false;
		goto end;
	}
	else if(cmd==EnableEddy){
		SCAN_EDDY= true;
		goto end;
	}
	else if(cmd==DisableEddy){
		SCAN_EDDY = false;
		goto end;
	}
	else if(cmd==EnableIbeacon){
		SCAN_iBeacon = true;
		goto end;
	}
	else if(cmd==DisableIbeacon){
		SCAN_iBeacon= false;
		goto end;
	}
	else if(cmd==EnableUnknown){
		SCAN_unknown = true;
		goto end;
	}
	else if(cmd==DisableUnknown){
		SCAN_unknown= false;
		goto end;
	}
	else{
		goto end;
	}
end:
    free(msg);
    return;
}

static void uart_fifo_callback(struct device *dev)
{
    u8_t data;
    if (!uart_irq_update(dev)) {
        printk("Error: uart_irq_update");
    }
    if (uart_irq_rx_ready(dev)) {
        const int rxBytes = uart_fifo_read(dev, &data, 1);
        if(rxBytes >0){
            char temp[512];
            sprintf(temp, "%c", data);
            char *ptr = strstr(temp, "\n");
            if(ptr == NULL){
                strcat(rx_buf, temp);
            }
            else{
                //printk("%s \n", rx_buf);
                uart_data_parse(rx_buf);
                memset(rx_buf, 0, 512);
            }
        }
    }
}

static void scan_cb(const bt_addr_le_t *addr, s8_t rssi, u8_t adv_type,
		    struct net_buf_simple *buf)
{
	int id = 0;
	int idIterator;
	bool send = false;
	//Find Manufacturer Id
	for(idIterator=0;idIterator<buf->len;idIterator++) {
		if(id == 0){
			if(buf->data[idIterator] == 0xff){
				id = idIterator;
			}
		}
		else{
			break;
		}
	}
	//Ruuvi Check
	if(buf->data[id+1] == 0x99 &&  buf->data[id+2] == 0x04){
		if (SCAN_RUUVI){
			send = true;
		}
	}
	//Eddystone Check
	else if(buf->data[id+1] == 0xaa &&  buf->data[id+2] == 0xfe){
		if (SCAN_EDDY){
			send = true;
		}
	}
	//iBeacon Check
	else if(buf->data[id+1] == 0x4c &&  buf->data[id+2] == 0x00){
		if(SCAN_iBeacon){
			send = true;
		}
	}
	else{
		if(SCAN_unknown){
			send = true;
		}	
	}
	
	if(send){	
		sprintf(ble_data,"%02X%02X%02X%02X%02X%02X,",
			addr->a.val[5], addr->a.val[4], addr->a.val[3],
			addr->a.val[2], addr->a.val[1], addr->a.val[0]);
		int i;
		for(i=0;i<buf->len;i++) {
			sprintf(ble_data + strlen(ble_data), "%02X",buf->data[i]);
		}
		sprintf(ble_data + strlen(ble_data), ",%d,",rssi);
		strcat(ble_data, "\r\n");
		u8_t temp;
		for (i = 0; i < strlen(ble_data); i++) {
			temp = ble_data[i];
			uart_poll_out(uart_dev, temp);
		}
		memset(ble_data, 0, 512);
	}
}

u8_t uart_init()
{
	uart_dev = device_get_binding(BLE_UART);
    if (!uart_dev) {
        printk("Error: Opening UART device");
        return 1;
    }
    else{
        uart_irq_callback_set(uart_dev, uart_fifo_callback);
        uart_irq_rx_enable(uart_dev);
        printk("UART device loaded.\n");
        return 0;
    }
}

void main(void)
{
	int err;
	err= uart_init();
	//Do something if uart error

	struct bt_le_scan_param scan_param = {
		.type       = BT_HCI_LE_SCAN_PASSIVE,
		.filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_ENABLE,
		.interval   = 0x0010,
		.window     = 0x0010,
	};

	printk("Starting Scanner/Advertiser Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_scan_start(&scan_param, scan_cb);
	if (err) {
		printk("Starting scanning failed (err %d)\n", err);
		return;
	}

	do {
		k_sleep(K_MSEC(1000));

	} while (1);
}

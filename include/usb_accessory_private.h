/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TIZEN_SYSTEM_USB_ACCESSORY_PRIVATE_H__
#define __TIZEN_SYSTEM_USB_ACCESSORY_PRIVATE_H__

#include <stdio.h>
#include <libintl.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dlog.h>
#include <unistd.h>
#include <aul.h>
#include <vconf.h>
#include <stdlib.h>
//#include <Ecore.h>
#include <glib.h>

#define ACC_ELEMENT_LEN 256
#define SOCK_PATH "/tmp/usb_server_sock"
#define ACC_SOCK_PATH "/tmp/usb_acc_sock"
#define USB_ACCESSORY_NODE "/dev/usb_accessory"
#define APP_ID_LEN 64
#define SOCK_STR_LEN 1542

#define USB_TAG "USB_ACCESSORY"

#define USB_LOG(format, args...) \
	LOG(LOG_VERBOSE, USB_TAG, "[%s][Ln: %d] " format, \
					(char*)(strrchr(__FILE__, '/')+1), __LINE__, ##args)

#define USB_LOG_ERROR(format, args...) \
	LOG(LOG_ERROR, USB_TAG, "[%s][Ln: %d] " format, \
					(char*)(strrchr(__FILE__, '/')+1), __LINE__, ##args)

#define __USB_FUNC_ENTER__ \
			USB_LOG("Entering: %s()\n", __func__)

#define __USB_FUNC_EXIT__ \
			USB_LOG("Exit: %s()\n", __func__)

#define FREE(arg) \
	do { \
		if(arg) { \
			free((void *)arg); \
			arg = NULL; \
		} \
	} while (0);

#define um_retvm_if(expr, val, fmt, arg...) \
	do { \
		if(expr) { \
			USB_LOG_ERROR(fmt, ##arg); \
			return (val); \
		} \
	} while (0);

#define um_retm_if(expr, fmt, arg...) \
	do { \
		if(expr) { \
			USB_LOG_ERROR(fmt, ##arg); \
			return; \
		} \
	} while (0);

typedef enum {
	IPC_ERROR = 0,
	IPC_FAIL,
	IPC_SUCCESS
} IPC_SIMPLE_RESULT;

typedef enum {
	LAUNCH_APP = 0,
	REQUEST_PERMISSION,
	HAS_PERMISSION,
	REQ_PERM_NOTI_YES_BTN,
	REQ_PERM_NOTI_NO_BTN,
	GET_ACC_INFO,
	ERROR_POPUP_OK_BTN,
	KIESWIFI_POPUP_YES_BTN,
	KIESWIFI_POPUP_NO_BTN
} REQUEST_TO_USB_MANGER;

typedef enum {
	ACC_MANUFACTURER = 0,
	ACC_MODEL,
	ACC_DESCRIPTION,
	ACC_VERSION,
	ACC_URI,
	ACC_SERIAL
} ACCESSORY_INFO;

struct usb_accessory_s {
	bool		accPermission;

	char		manufacturer[ACC_ELEMENT_LEN];
	char		model[ACC_ELEMENT_LEN];
	char		description[ACC_ELEMENT_LEN];
	char		version[ACC_ELEMENT_LEN];
	char		uri[ACC_ELEMENT_LEN];
	char		serial[ACC_ELEMENT_LEN];
};

struct usb_accessory_list {
	struct usb_accessory_s *accessory;
	struct usb_accessory_list *next;
};

struct AccCbData {
	void *user_data;
	void (*connection_cb_func)(struct usb_accessory_s *accessory, bool is_connected, void *data);
	void (*request_perm_cb_func)(struct usb_accessory_s *accessory, bool is_granted);
	struct usb_accessory_s *accessory;
};

int ipc_request_client_init(int *sock_remote);
int ipc_request_client_close(int *sock_remote);
int request_to_usb_server(int sock_remote, int request, char *answer, char *pkgName);
char *get_app_id();
void accessory_status_changed_cb(keynode_t *in_key, void* data);
bool getAccList(struct usb_accessory_list **accList);
bool freeAccList(struct usb_accessory_list *accList);
int ipc_noti_client_init(void);
int ipc_noti_client_close(int *sock_remote);
gboolean ipc_noti_client_cb(GIOChannel *g_io_ch, GIOCondition condition, gpointer data);
#endif /* __TIZEN_SYSTEM_USB_ACCESSORY_PRIVATE_H__ */


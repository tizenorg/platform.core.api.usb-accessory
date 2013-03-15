/*
 * usb-accessory
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
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
#include <sys/utsname.h>
#include <glib.h>

#define SOCK_PATH "/tmp/usb_server_sock"
#define ACC_SOCK_PATH "/tmp/usb_acc_sock"
#define USB_ACCESSORY_NODE "/dev/usb_accessory"
#define APP_ID_LEN 64
#define SOCK_STR_LEN 1542


#undef LOG_TAG
#define LOG_TAG "USB_ACCESSORY"
#define USB_LOG(fmt, args...)         SLOGD(fmt, ##args)
#define USB_LOG_ERROR(fmt, args...)   SLOGE(fmt, ##args)
#define __USB_FUNC_ENTER__            USB_LOG("ENTER")
#define __USB_FUNC_EXIT__             USB_LOG("EXIT")

#define FREE(arg) \
	do { \
		if(arg) { \
			free((void *)arg); \
			arg = NULL; \
		} \
	} while (0);

typedef enum {
	IPC_ERROR = 0,
	IPC_FAIL,
	IPC_SUCCESS
} IPC_SIMPLE_RESULT;

typedef enum {
	/* General */
	ERROR_POPUP_OK_BTN = 0,
	IS_EMUL_BIN,

	/* for Accessory */
	LAUNCH_APP_FOR_ACC = 20,
	REQ_ACC_PERMISSION,
	HAS_ACC_PERMISSION,
	REQ_ACC_PERM_NOTI_YES_BTN,
	REQ_ACC_PERM_NOTI_NO_BTN,
	GET_ACC_INFO,

	/* for Host */
	LAUNCH_APP_FOR_HOST = 40,
	REQ_HOST_PERMISSION,
	HAS_HOST_PERMISSION,
	REQ_HOST_PERM_NOTI_YES_BTN,
	REQ_HOST_PERM_NOTI_NO_BTN,
	REQ_HOST_CONNECTION
} REQUEST_TO_USB_MANGER;

struct usb_accessory_s {
	bool accPermission;

	char *manufacturer;
	char *model;
	char *description;
	char *version;
	char *uri;
	char *serial;
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

int ipc_request_client_init();
int request_to_usb_server(int sockRemote, int request, void *data, char *answer, int answerLen);
char *get_app_id();
void accessory_status_changed_cb(keynode_t *key, void* data);
bool get_acc_list(struct usb_accessory_list **accList);
void free_accessory(struct usb_accessory_s *accessory);
void free_acc_list(struct usb_accessory_list *accList);
int ipc_noti_client_init(void);
gboolean ipc_noti_client_cb(GIOChannel *gioCh, GIOCondition condition, gpointer data);
bool is_emul_bin();

#endif /* __TIZEN_SYSTEM_USB_ACCESSORY_PRIVATE_H__ */


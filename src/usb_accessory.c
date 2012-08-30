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

#include "usb_accessory.h"
#include "usb_accessory_private.h"

struct AccCbData *accCbData;

int usb_accessory_clone(usb_accessory_h handle, usb_accessory_h* cloned_handle)
{
	__USB_FUNC_ENTER__ ;
	if (!handle) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	if (!cloned_handle || *cloned_handle) return USB_ERROR_INVALID_PARAMETER;
	*cloned_handle = (usb_accessory_h)malloc(sizeof(struct usb_accessory_s));
	snprintf((*cloned_handle)->manufacturer, strlen(handle->manufacturer), "%s", handle->manufacturer);
	snprintf((*cloned_handle)->model, strlen(handle->model), "%s", handle->model);
	snprintf((*cloned_handle)->description, strlen(handle->description), "%s", handle->description);
	snprintf((*cloned_handle)->version, strlen(handle->version), "%s", handle->version);
	snprintf((*cloned_handle)->uri, strlen(handle->uri), "%s", handle->uri);
	snprintf((*cloned_handle)->serial, strlen(handle->serial), "%s", handle->serial);

	(*cloned_handle)->accPermission = false;

	__USB_FUNC_EXIT__ ;
    return USB_ERROR_NONE;
}

int usb_accessory_destroy(usb_accessory_h handle)
{
    __USB_FUNC_ENTER__ ;
	if (!handle) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	FREE(handle);
	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_foreach_attached(usb_accessory_attached_cb callback, void *user_data)
{
	__USB_FUNC_ENTER__ ;
	if (callback == NULL) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	struct usb_accessory_list *accList = NULL;
	struct usb_accessory_list *tmpList = NULL;
	bool ret = false;
	ret = getAccList(&accList);
	um_retvm_if(ret == false, -1, "FAIL: getAccList(accList)\n");
	um_retvm_if(accList == NULL, -1, "ERROR: accList == NULL\n");

	ret = true;
	tmpList = accList;
	while (ret) {
		if (tmpList == NULL || tmpList->accessory == NULL) {
			break;
		}
		ret = callback(tmpList->accessory, user_data);
		if (ret) {
			tmpList = tmpList->next;
		}
	}

	ret = freeAccList(accList);
	um_retvm_if(ret == false, USB_ERROR_OPERATION_FAILED, "FAIL: freeAccList(accList)\n");

	__USB_FUNC_EXIT__ ;
    return USB_ERROR_NONE;
}


int usb_accessory_set_connection_changed_cb(usb_accessory_connection_changed_cb callback, void* user_data)
{
	__USB_FUNC_ENTER__ ;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	int ret = -1;
	accCbData = (struct AccCbData *)malloc(sizeof(struct AccCbData));
	accCbData->user_data = user_data;
	accCbData->connection_cb_func = callback;
	ret = vconf_notify_key_changed(VCONFKEY_USB_ACCESSORY_STATUS, accessory_status_changed_cb, accCbData);
	um_retvm_if(ret < 0, USB_ERROR_OPERATION_FAILED, "FAIL: vconf_notify_key_changed(VCONFKEY_USB_ACCESSORY_STATUS)\n");
	__USB_FUNC_EXIT__ ;
    return USB_ERROR_NONE;
}
 

int usb_accessory_connection_unset_cb()
{
	__USB_FUNC_ENTER__ ;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	if (accCbData != NULL) {
		FREE(accCbData);
		int ret = vconf_ignore_key_changed(VCONFKEY_USB_ACCESSORY_STATUS, accessory_status_changed_cb);
		um_retvm_if(ret < 0, USB_ERROR_OPERATION_FAILED, "FAIL: vconf_ignore_key_changed(VCONFKEY_USB_ACCESSORY_status");
	}
	__USB_FUNC_EXIT__ ;

    return USB_ERROR_NONE;
}
 

int usb_accessory_has_permission(usb_accessory_h accessory, bool* is_granted)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	if (accessory->accPermission == true) {
		__USB_FUNC_EXIT__ ;
		return USB_ERROR_NONE;
	} else {
		int ret = -1;
		int ipc_result = -1;
		int sock_remote;
		char buf[SOCK_STR_LEN];
		char *app_id = get_app_id();
		if(app_id == NULL) {
			USB_LOG("FAIL: get_app_id()\n");
			*is_granted = false;
			return USB_ERROR_NONE;
		}

		ret = ipc_request_client_init(&sock_remote);
		um_retvm_if(ret < 0, USB_ERROR_PERMISSION_DENIED, "FAIL: ipc_request_client_init()\n");

		ret = request_to_usb_server(sock_remote, HAS_ACC_PERMISSION, buf, app_id);
		um_retvm_if(ret < 0, USB_ERROR_PERMISSION_DENIED, "FAIL: request_to_usb_server()\n");

		ret = ipc_request_client_close(&sock_remote);
		um_retvm_if(ret < 0, USB_ERROR_PERMISSION_DENIED, "FAIL: ipc_request_client_close()\n");

		FREE(app_id);

		USB_LOG("Permission: %s\n", buf);
		ipc_result = atoi(buf);
		if (IPC_SUCCESS == ipc_result) {
			accessory->accPermission = true;
			*is_granted = true;
		} else {
			accessory->accPermission = false;
			*is_granted = false;
		}
		__USB_FUNC_EXIT__ ;
		return USB_ERROR_NONE;
	}
}


int usb_accessory_open(usb_accessory_h accessory, FILE **fd)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	if (accessory->accPermission == true) {
		*fd = fopen(USB_ACCESSORY_NODE, "r+");
		USB_LOG("file pointer: %d", *fd);
	} else {
		USB_LOG("Permission is not allowed");
		*fd = NULL;
	}
	__USB_FUNC_EXIT__ ;
    return USB_ERROR_NONE;
}


int usb_accessory_get_description(usb_accessory_h accessory, char** description)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	*description = strdup(accessory->description);
	__USB_FUNC_ENTER__ ;
    return USB_ERROR_NONE;
}


int usb_accessory_get_manufacturer(usb_accessory_h accessory, char** manufacturer)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	*manufacturer = strdup(accessory->manufacturer);
	__USB_FUNC_ENTER__ ;
    return USB_ERROR_NONE;
}


int usb_accessory_get_model(usb_accessory_h accessory, char** model)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	*model = strdup(accessory->model);
	__USB_FUNC_ENTER__ ;
    return USB_ERROR_NONE;
}


int usb_accessory_get_serial(usb_accessory_h accessory, char** serial)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	*serial = strdup(accessory->serial);
	__USB_FUNC_ENTER__ ;
    return USB_ERROR_NONE;
}


int usb_accessory_get_version(usb_accessory_h accessory, char** version)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	*version = strdup(accessory->version);
	__USB_FUNC_ENTER__ ;
    return USB_ERROR_NONE;
}


int usb_accessory_is_connected(usb_accessory_h accessory, bool* is_connected)
{
	__USB_FUNC_ENTER__ ;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	int ret = -1;
	int val = -1;
	ret = vconf_get_int(VCONFKEY_USB_ACCESSORY_STATUS, &val);
	um_retvm_if(ret < 0, USB_ERROR_OPERATION_FAILED, "FAIL: vconf_get_int(VCONFKEY_USB_ACCESSORY_STATUS)\n");
	switch (val) {
	case VCONFKEY_USB_ACCESSORY_STATUS_CONNECTED:
		*is_connected = true;
		break;
	case VCONFKEY_USB_ACCESSORY_STATUS_DISCONNECTED:
		*is_connected = false;
		break;
	}
	__USB_FUNC_EXIT__ ;
    return USB_ERROR_NONE;
}

int usb_accessory_request_permission(usb_accessory_h accessory, usb_accessory_permission_response_cb callback, void* user_data)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory) return USB_ERROR_INVALID_PARAMETER;
	if (!callback) return USB_ERROR_INVALID_PARAMETER;
	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}
	int ret = -1;
	guint g_ret = 0;
	int sock_remote;
	char buf[SOCK_STR_LEN];
	char *app_id = get_app_id();
	accCbData->user_data = accessory;
	accCbData->request_perm_cb_func = callback;
	accCbData->accessory = accessory;
	GError *err;
	GIOStatus gio_ret;

	int fd = ipc_noti_client_init();
	GIOChannel *g_io_ch = g_io_channel_unix_new(fd);
	g_ret = g_io_add_watch(g_io_ch, G_IO_IN, ipc_noti_client_cb, (gpointer)accCbData);
	um_retvm_if (0 == g_ret, USB_ERROR_PERMISSION_DENIED, "FAIL: g_io_add_watch(g_io_ch, G_IO_IN)\n");

	ret = ipc_request_client_init(&sock_remote);
	if(ret < 0) {
		USB_LOG("FAIL: ipc_request_client_init(&sock_remote)\n");
		ret = ipc_request_client_close(&sock_remote);
		if (ret < 0) USB_LOG("FAIL: ipc_request_client_close(&sock_remote)\n");
		ret = ipc_noti_client_close(&fd);
		if (ret < 0) USB_LOG("FAIL: ipc_noti_client_close(&fd)\n");
		gio_ret = g_io_channel_shutdown(g_io_ch, TRUE, &err);
		if (G_IO_STATUS_ERROR == gio_ret) USB_LOG("ERROR: g_io_channel_shutdown(g_io_ch)\n");
		g_io_channel_unref(g_io_ch);
		FREE(app_id);
		return USB_ERROR_PERMISSION_DENIED;
	}

	ret = request_to_usb_server(sock_remote, REQ_ACC_PERMISSION, buf, app_id);
	if(ret < 0) {
		USB_LOG("FAIL: request_to_usb_server(REQ_ACC_PERMISSION)\n");
		ret = ipc_request_client_close(&sock_remote);
		if (ret < 0) USB_LOG("FAIL: ipc_request_client_close(&sock_remote)\n");
		ret = ipc_noti_client_close(&fd);
		if (ret < 0) USB_LOG("FAIL: ipc_noti_client_close(&fd)\n");
		gio_ret = g_io_channel_shutdown(g_io_ch, TRUE, &err);
		if (G_IO_STATUS_ERROR == gio_ret) USB_LOG("ERROR: g_io_channel_shutdown(g_io_ch)\n");
		g_io_channel_unref(g_io_ch);
		FREE(app_id);
		return USB_ERROR_PERMISSION_DENIED;
	}

	FREE(app_id);

	ret = ipc_request_client_close(&sock_remote);
	if (ret < 0) {
		USB_LOG("FAIL: ipc_request_client_close(&sock_remote)\n");
		ret = ipc_noti_client_close(&fd);
		if (ret < 0) USB_LOG("FAIL: ipc_noti_client_close(&fd)\n");
		gio_ret = g_io_channel_shutdown(g_io_ch, TRUE, &err);
		if (G_IO_STATUS_ERROR == gio_ret) USB_LOG("ERROR: g_io_channel_shutdown(g_io_ch)\n");
		g_io_channel_unref(g_io_ch);
		return USB_ERROR_PERMISSION_DENIED;
	}

	__USB_FUNC_EXIT__ ;
    return USB_ERROR_NONE;
}

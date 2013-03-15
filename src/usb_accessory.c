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

#include "usb_accessory.h"
#include "usb_accessory_private.h"

struct AccCbData *accCbData;

int usb_accessory_clone(usb_accessory_h handle, usb_accessory_h* cloned_handle)
{
	__USB_FUNC_ENTER__ ;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!handle)
		return USB_ERROR_INVALID_PARAMETER;

	if (!cloned_handle || *cloned_handle)
		return USB_ERROR_INVALID_PARAMETER;

	*cloned_handle = (usb_accessory_h)malloc(sizeof(struct usb_accessory_s));
	if (!(*cloned_handle)) {
		USB_LOG("FAIL: malloc()");
		return USB_ERROR_OPERATION_FAILED;
	}

	(*cloned_handle)->manufacturer = strdup(handle->manufacturer);
	(*cloned_handle)->model = strdup(handle->model);
	(*cloned_handle)->description = strdup(handle->description);
	(*cloned_handle)->version = strdup(handle->version);
	(*cloned_handle)->uri = strdup(handle->uri);
	(*cloned_handle)->serial = strdup(handle->serial);
	(*cloned_handle)->accPermission = false;

	__USB_FUNC_EXIT__ ;
    return USB_ERROR_NONE;
}

int usb_accessory_destroy(usb_accessory_h handle)
{
	__USB_FUNC_ENTER__ ;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!handle)
		return USB_ERROR_INVALID_PARAMETER;

	free_accessory(handle);

	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_foreach_attached(usb_accessory_attached_cb callback, void *user_data)
{
	__USB_FUNC_ENTER__ ;

	struct usb_accessory_list *accList;
	struct usb_accessory_list *tmpList;
	bool ret;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!callback)
		return USB_ERROR_INVALID_PARAMETER;

	accList = (struct usb_accessory_list *)malloc(sizeof(struct usb_accessory_list));
	if (!accList) {
		USB_LOG("FAIL: malloc()");
		return USB_ERROR_OPERATION_FAILED;
	}

	ret = get_acc_list(&accList);
	if (!ret) {
		USB_LOG("FAIL: getAccList()");
		free_acc_list(accList);
		return USB_ERROR_OPERATION_FAILED;
	}

	ret = true;
	tmpList = accList;
	while (ret) {
		if (!tmpList || !(tmpList->accessory)) {
			break;
		}
		ret = callback(tmpList->accessory, user_data);
		if (ret) {
			tmpList = tmpList->next;
		}
	}

	free_acc_list(accList);

	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_set_connection_changed_cb(usb_accessory_connection_changed_cb callback,
						void* user_data)
{
	__USB_FUNC_ENTER__ ;

	int ret;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!callback)
		return USB_ERROR_INVALID_PARAMETER;

	accCbData = (struct AccCbData *)malloc(sizeof(struct AccCbData));
	if (!accCbData) {
		USB_LOG("FAIL: malloc()");
		return USB_ERROR_OPERATION_FAILED;
	}

	accCbData->user_data = user_data;
	accCbData->connection_cb_func = callback;

	ret = vconf_notify_key_changed(VCONFKEY_USB_ACCESSORY_STATUS,
					accessory_status_changed_cb, accCbData);
	if (ret < 0) {
		USB_LOG("FAIL: vconf_notify_key_changed()");
		return USB_ERROR_OPERATION_FAILED;
	}

	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_connection_unset_cb()
{
	__USB_FUNC_ENTER__ ;

	int ret;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	FREE(accCbData);

	ret = vconf_ignore_key_changed(VCONFKEY_USB_ACCESSORY_STATUS, accessory_status_changed_cb);
	if (ret < 0) {
		USB_LOG("FAIL: vconf_ignore_key_changed()");
		return USB_ERROR_OPERATION_FAILED;
	}

	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_has_permission(usb_accessory_h accessory, bool* is_granted)
{
	__USB_FUNC_ENTER__ ;

	int ret;
	int ipcResult;
	int sockRemote;
	char buf[SOCK_STR_LEN];
	char *appId;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;

	if (accessory->accPermission) {
		__USB_FUNC_EXIT__ ;
		return USB_ERROR_NONE;
	}

	appId = get_app_id();
	if(!appId) {
		USB_LOG("FAIL: get_app_id()");
		*is_granted = false;
		return USB_ERROR_NONE;
	}

	sockRemote = ipc_request_client_init();
	if (sockRemote < 0) {
		USB_LOG("FAIL: ipc_request_client_init()");
		FREE(appId);
		return USB_ERROR_IPC_FAILED;
	}

	ret = request_to_usb_server(sockRemote, HAS_ACC_PERMISSION, appId, buf, sizeof(buf));
	if (ret < 0) {
		USB_LOG("FAIL: request_to_usb_server()");
		close(sockRemote);
		FREE(appId);
		return USB_ERROR_IPC_FAILED;
	}

	close(sockRemote);
	FREE(appId);

	USB_LOG("Permission: %s", buf);
	ipcResult = atoi(buf);
	if (IPC_SUCCESS == ipcResult) {
		accessory->accPermission = true;
		*is_granted = true;
	} else {
		accessory->accPermission = false;
		*is_granted = false;
	}
	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_open(usb_accessory_h accessory, FILE **fd)
{
	__USB_FUNC_ENTER__ ;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;

	if (accessory->accPermission) {
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

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;

	*description = strdup(accessory->description);

	__USB_FUNC_ENTER__ ;
	return USB_ERROR_NONE;
}


int usb_accessory_get_manufacturer(usb_accessory_h accessory, char** manufacturer)
{
	__USB_FUNC_ENTER__ ;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;

	*manufacturer = strdup(accessory->manufacturer);

	__USB_FUNC_ENTER__ ;
	return USB_ERROR_NONE;
}


int usb_accessory_get_model(usb_accessory_h accessory, char** model)
{
	__USB_FUNC_ENTER__ ;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;

	*model = strdup(accessory->model);

	__USB_FUNC_ENTER__ ;
	return USB_ERROR_NONE;
}


int usb_accessory_get_serial(usb_accessory_h accessory, char** serial)
{
	__USB_FUNC_ENTER__ ;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;

	*serial = strdup(accessory->serial);

	__USB_FUNC_ENTER__ ;
	return USB_ERROR_NONE;
}


int usb_accessory_get_version(usb_accessory_h accessory, char** version)
{
	__USB_FUNC_ENTER__ ;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;

	*version = strdup(accessory->version);

	__USB_FUNC_ENTER__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_is_connected(usb_accessory_h accessory, bool* is_connected)
{
	__USB_FUNC_ENTER__ ;

	int ret;
	int val;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	ret = vconf_get_int(VCONFKEY_USB_ACCESSORY_STATUS, &val);
	if (ret < 0) {
		USB_LOG("FAIL: vconf_get_int()");
		return USB_ERROR_OPERATION_FAILED;
	}

	switch (val) {
	case VCONFKEY_USB_ACCESSORY_STATUS_CONNECTED:
		*is_connected = true;
		break;
	case VCONFKEY_USB_ACCESSORY_STATUS_DISCONNECTED:
		*is_connected = false;
		break;
	default:
		USB_LOG("vconf key value is improper");
		return USB_ERROR_OPERATION_FAILED;
	}

	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;
}

int usb_accessory_request_permission(usb_accessory_h accessory,
				usb_accessory_permission_response_cb callback,
				void* user_data)
{
	__USB_FUNC_ENTER__ ;

	int ret;
	guint gret;
	int fd;
	int sockRemote;
	char buf[SOCK_STR_LEN];
	char *appId;
	GError *err;
	GIOStatus gioret;
	GIOChannel *gioCh;

	if (is_emul_bin()) {
		USB_LOG("FAIL:USB Accessory is not available with emulator.");
		return USB_ERROR_NOT_SUPPORTED;
	}

	if (!accessory)
		return USB_ERROR_INVALID_PARAMETER;
	if (!callback)
		return USB_ERROR_INVALID_PARAMETER;
	if (!accCbData)
		return USB_ERROR_INVALID_PARAMETER;

	accCbData->user_data = accessory;
	accCbData->request_perm_cb_func = callback;
	accCbData->accessory = accessory;

	fd = ipc_noti_client_init();
	if (fd < 0) {
		USB_LOG("FAIL: ipc_noti_client_init()");
		goto out_ipc_fail;
	}

	gioCh = g_io_channel_unix_new(fd);
	if (!gioCh) {
		USB_LOG("FAIL: g_io_channel_unix_new()");
		goto out_ipc_noti;
	}

	gret = g_io_add_watch(gioCh, G_IO_IN, ipc_noti_client_cb, (gpointer)accCbData);
	if (gret == 0) {
		USB_LOG("FAIL: g_io_add_watch(g_io_ch, G_IO_IN)");
		goto out_gio_unref;
	}

	sockRemote = ipc_request_client_init();
	if(sockRemote < 0) {
		USB_LOG("FAIL: ipc_request_client_init(&sock_remote)");
		goto out_gio_unref;
	}

	appId = get_app_id();
	USB_LOG("App ID: %s", appId);
	if (!appId) {
		USB_LOG("FAIL: get_app_id()");
		goto out_ipc_request;
	}

	ret = request_to_usb_server(sockRemote, REQ_ACC_PERMISSION, appId, buf, sizeof(buf));
	if(ret < 0) {
		USB_LOG("FAIL: request_to_usb_server(REQ_ACC_PERMISSION)");
		goto out_app_id;
	}

	FREE(appId);
	close(sockRemote);

	__USB_FUNC_EXIT__ ;
	return USB_ERROR_NONE;

out_app_id:
	FREE(appId);
out_ipc_request:
	close(sockRemote);
out_gio_unref:
	gioret = g_io_channel_shutdown(gioCh, TRUE, &err);
	if (G_IO_STATUS_ERROR == gioret)
		USB_LOG("ERROR: g_io_channel_shutdown()");
	g_io_channel_unref(gioCh);
out_ipc_noti:
	close(fd);
out_ipc_fail:
	__USB_FUNC_EXIT__ ;
	return USB_ERROR_IPC_FAILED;
}

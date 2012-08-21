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

#include "usb_accessory_private.h"

/* This function initializes socket for ipc with usb-server */
int ipc_request_client_init(int *sock_remote)
{
	__USB_FUNC_ENTER__ ;
	if (!sock_remote) return -1;
	int len;
	struct sockaddr_un remote;

	if (((*sock_remote) = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		USB_LOG("FAIL: socket(AF_UNIX, SOCK_STREAM, 0)");
		return -1;;
	}
	remote.sun_family = AF_UNIX;
	strncpy(remote.sun_path, SOCK_PATH, strlen(SOCK_PATH)+1);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if (connect((*sock_remote), (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		USB_LOG("FAIL: connect((*sock_remote), (struct sockaddr *)&remote, len)");
		return -1;
	}
	__USB_FUNC_EXIT__ ;
	return 0;
}

/* This function closes socket for ipc with usb-server */
int ipc_request_client_close(int *sock_remote)
{
	__USB_FUNC_ENTER__ ;
	if (!sock_remote) return -1;
	close (*sock_remote);
	__USB_FUNC_EXIT__ ;
	return 0;
}

/* This function requests something to usb-server by ipc with socket and gets the results */
int request_to_usb_server(int sock_remote, int request, char *answer, char *pkgName)
{
	__USB_FUNC_ENTER__ ;
	int t;
	char str[SOCK_STR_LEN];

	USB_LOG("request: %d, pkgName: %s\n", request, pkgName);
	snprintf(str, SOCK_STR_LEN, "%d|%s", request, pkgName);
	if (send (sock_remote, str, strlen(str)+1, 0) == -1) {
		USB_LOG("FAIL: send (sock_remote, str, strlen(str)+1, 0)\n");
		return -1;
	}
	if ((t = recv(sock_remote, answer, SOCK_STR_LEN, 0)) > 0) {
		answer[t] = '\0';
		USB_LOG("[CLIENT] Received value: %s\n", answer);
	} else {
		USB_LOG("FAIL: recv(sock_remote, str, SOCK_STR_LEN, 0)\n");
		return -1;
	}
	__USB_FUNC_EXIT__ ;
	return 0;
}

int handle_input_to_server(void *data, char *buf)
{
	__USB_FUNC_ENTER__ ;
	if (!data) return -1;
	if (!buf) return -1;
	struct AccCbData *permCbData = (struct AccCbData *)data;
	int input = atoi(buf);
	USB_LOG("Input: %d\n", input);

	switch (input) {
	case REQ_PERM_NOTI_YES_BTN:
		permCbData->accessory->accPermission = true;
		permCbData->request_perm_cb_func((struct usb_accessory_s*)(permCbData->user_data), true);
		break;
	case REQ_PERM_NOTI_NO_BTN:
		permCbData->accessory->accPermission = false;
		permCbData->request_perm_cb_func((struct usb_accessory_s*)(permCbData->user_data), false);
		break;
	default:
		break;
	}
	__USB_FUNC_EXIT__ ;
	return 0;
}

static int read_message(int fd, char *str, int len)
{
	__USB_FUNC_ENTER__;
	int ret = -1;
	while(1) {
		ret = read(fd, str, len);
		if (ret < 0) {
			if (EINTR == errno) {
				USB_LOG("Re-read for error(EINTR)\n");
				continue;
			} else {
				USB_LOG("FAIL: read(fd, str, len)\n");
				return -1;
			}
		} else {
			__USB_FUNC_EXIT__;
			return 0;
		}
	}
}

int ipc_noti_client_init(void)
{
	__USB_FUNC_ENTER__ ;
	int sock_local;
	int ret = -1;
	int len;
	struct sockaddr_un serveraddr;

	sock_local = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_local < 0) {
		perror("socket");
		USB_LOG("FAIL: socket(AF_UNIX, SOCK_STREAM, 0)\n");
		return -1;
	}
	serveraddr.sun_family = AF_UNIX;
	strncpy(serveraddr.sun_path, ACC_SOCK_PATH, strlen(ACC_SOCK_PATH)+1);
	USB_LOG("socket file name: %s\n", serveraddr.sun_path);
	unlink(serveraddr.sun_path);
	len = strlen(serveraddr.sun_path) + sizeof(serveraddr.sun_family);

	if (bind (sock_local, (struct sockaddr *)&serveraddr, len) < 0) {
		perror("bind");
		USB_LOG("FAIL: bind (sock_local, (struct sockaddr_un *)serveraddr)\n");
		return -1;
	}

	ret = chown(ACC_SOCK_PATH, 5000, 5000);
	if (ret < 0) USB_LOG("FAIL: chown(ACC_SOCK_PATH, 5000, 5000)");
	ret = chmod(ACC_SOCK_PATH, 0777);
	if (ret < 0) USB_LOG("FAIL: chmod(ACC_SOCK_PATH, 0777);");

	if (listen (sock_local, 5) == -1) {
		perror("listen");
		USB_LOG("FAIL: listen (sock_local, 5)\n");
		return -1;
	}

	__USB_FUNC_EXIT__ ;
	return sock_local;
}

int ipc_noti_client_close(int *sock_remote)
{
	__USB_FUNC_ENTER__ ;
	close(*sock_remote);
	__USB_FUNC_EXIT__ ;
	return 0;
}

gboolean ipc_noti_client_cb(GIOChannel *g_io_ch, GIOCondition condition, gpointer data)
{
	__USB_FUNC_ENTER__ ;
	if (!data) return FALSE;
	if (!g_io_ch) return FALSE;
	int fd;
	int ret = -1;
	struct sockaddr_un client_address;
	int client_sockfd;
	int client_len;
	char input_buf[SOCK_STR_LEN];
	char output_buf[SOCK_STR_LEN];
	GError *err;
	GIOStatus gio_ret;

	fd = g_io_channel_unix_get_fd(g_io_ch);
	if (fd < 0) {
		USB_LOG("FAIL: g_io_channel_unix_get_fd(g_io_ch)\n");
		gio_ret = g_io_channel_shutdown(g_io_ch, TRUE, &err);
		if (G_IO_STATUS_ERROR == gio_ret) USB_LOG("ERROR: g_io_channel_shutdown(g_io_ch)\n");
		g_io_channel_unref(g_io_ch);
		return FALSE;
	}

	client_len = sizeof(client_address);
	client_sockfd = accept(fd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
	if (client_sockfd == -1) {
		perror("accept");
		USB_LOG("FAIL: accept(fd, (struct sockaddr *)&client_address, (socklen_t *)&client_len)\n");
		gio_ret = g_io_channel_shutdown(g_io_ch, TRUE, &err);
		if (G_IO_STATUS_ERROR == gio_ret) USB_LOG("ERROR: g_io_channel_shutdown(g_io_ch)\n");
		g_io_channel_unref(g_io_ch);
		ret = ipc_noti_client_close(&client_sockfd);
		if (ret < 0) USB_LOG("FAIL: ipc_noti_client_close(client_sockfd)\n");
		return FALSE;
	}

	if(read_message(client_sockfd, input_buf, sizeof(input_buf)) < 0) {
		USB_LOG("FAIL: read_message(client_sockfd, &buf)\n");
		snprintf(output_buf, strlen("FAIL"), "%d", IPC_FAIL);
		ret = write(client_sockfd, &output_buf, sizeof(output_buf));
		if (ret < 0) USB_LOG("FAIL: write(client_sockfd, &output_buf, sizeof(output_buf))\n");
		gio_ret = g_io_channel_shutdown(g_io_ch, TRUE, &err);
		if (G_IO_STATUS_ERROR == gio_ret) USB_LOG("ERROR: g_io_channel_shutdown(g_io_ch)\n");
		g_io_channel_unref(g_io_ch);
		ret = ipc_noti_client_close(&client_sockfd);
		if (ret < 0) USB_LOG("FAIL: ipc_noti_client_close(client_sockfd)\n");
		return FALSE;
	}
	USB_LOG("read(): %s\n", input_buf);
	snprintf(output_buf, SOCK_STR_LEN, "%d", IPC_SUCCESS);
	ret = write(client_sockfd, &output_buf, sizeof(output_buf));
	if (ret < 0) USB_LOG("FAIL: write(client_sockfd, &output_buf, sizeof(output_buf))\n");
	gio_ret = g_io_channel_shutdown(g_io_ch, TRUE, &err);
	if (G_IO_STATUS_ERROR == gio_ret) USB_LOG("ERROR: g_io_channel_shutdown(g_io_ch)\n");
	g_io_channel_unref(g_io_ch);
	ret = ipc_noti_client_close(&client_sockfd);
	if (ret < 0) USB_LOG("FAIL: ipc_noti_client_close(client_sockfd)\n");

	ret = handle_input_to_server((void *)data, input_buf);
	if (ret < 0) USB_LOG("FAIL: handle_input_to_server(input_buf, data)\n");

	__USB_FUNC_EXIT__ ;
	return FALSE;
}

/* This function returns the app id.
 * After calling this function, the memory space should be freed */
char *get_app_id()
{
	__USB_FUNC_ENTER__ ;
	int pid = getpid();
	USB_LOG("pid: %d\n", pid);
	char appId[APP_ID_LEN];
	int ret = aul_app_get_appid_bypid(getpid(), appId, APP_ID_LEN);
	um_retvm_if(AUL_R_OK != ret, NULL, "FAIL: aul_app_get_appid_bypid(getpid(), appId)\n");
	__USB_FUNC_EXIT__ ;
	return strdup(appId);
}

/* This function find an accessory attached just now
 * Currently This function supports just one accessory */
static int getChangedAcc (struct usb_accessory_s **attAcc)
{
	__USB_FUNC_ENTER__ ;
	if (attAcc == NULL) return -1;
	struct usb_accessory_list *accList = NULL;
	bool ret = getAccList(&accList);
	um_retvm_if(ret == false, -1, "FAIL: getAccList(&accList)\n");
	um_retvm_if(accList == NULL, -1, "accList == NULL\n");
	*attAcc = (struct usb_accessory_s *)malloc(sizeof(struct usb_accessory_s));
	snprintf((*attAcc)->manufacturer, ACC_ELEMENT_LEN, "%s", accList->accessory->manufacturer);
	snprintf((*attAcc)->model, ACC_ELEMENT_LEN, "%s", accList->accessory->model);
	snprintf((*attAcc)->description, ACC_ELEMENT_LEN, "%s", accList->accessory->description);
	snprintf((*attAcc)->version, ACC_ELEMENT_LEN, "%s", accList->accessory->version);
	snprintf((*attAcc)->uri, ACC_ELEMENT_LEN, "%s", accList->accessory->uri);
	snprintf((*attAcc)->serial, ACC_ELEMENT_LEN, "%s", accList->accessory->serial);
	ret = freeAccList(accList);
	um_retvm_if(ret == false, -1, "FAIL: freeAccList(accList)\n");

	__USB_FUNC_EXIT__ ;
	return 0;
}

/* This func release memory of an accessory attached just now */
static int freeChangedAcc (struct usb_accessory_s **attAcc)
{
	__USB_FUNC_ENTER__ ;
	if (attAcc == NULL) return -1;
	FREE(*attAcc);
	__USB_FUNC_EXIT__ ;
	return 0;
}

/* Callback function which is called when accessory vconf key is changed */
void accessory_status_changed_cb(keynode_t *in_key, void* data)
{
	__USB_FUNC_ENTER__ ;
	if (!data)  return ;
	struct AccCbData *conCbData = (struct AccCbData *)data;
	struct usb_accessory_s *changedAcc = NULL;
	int ret = -1;
	int val = -1;
	ret = vconf_get_int(VCONFKEY_USB_ACCESSORY_STATUS, &val);
	um_retm_if(ret < 0, "FAIL: vconf_get_int(VCONFKEY_USB_ACCESSORY_STATUS)\n");

	ret = getChangedAcc(&changedAcc);
	um_retm_if(ret < 0, "FAIL: getChangedAcc(&changedAcc)\n");

	switch (val) {
	case VCONFKEY_USB_ACCESSORY_STATUS_DISCONNECTED:
		conCbData->connection_cb_func(changedAcc, false, conCbData->user_data);
		break;
	case VCONFKEY_USB_ACCESSORY_STATUS_CONNECTED:
		conCbData->connection_cb_func(changedAcc, true, conCbData->user_data);
		break;
	default:
		USB_LOG("ERROR: The value of VCONFKEY_USB_ACCESSORY_STATUS is invalid\n");
		break;
	}
	ret = freeChangedAcc(&changedAcc);
	um_retm_if(ret < 0, "FAIL: freeChangedAcc(&changedAcc)\n");

	__USB_FUNC_EXIT__ ;
}

/* Get an element from a string which has all information of an accessory */
static bool getAccElement(char *totalInfo[], char accInfo[])
{
	__USB_FUNC_ENTER__ ;
	if (!totalInfo) return false;
	if (!accInfo) return false;

	char *finder = *totalInfo;

	if (*totalInfo) {
		while (1) {
			if (finder == NULL) {
				USB_LOG("ERROR: finder == NULL");
				__USB_FUNC_EXIT__ ;
				return false;
			}
			if (*finder == '|' || *finder == '\0') {
				*finder = '\0';
				snprintf(accInfo, ACC_ELEMENT_LEN, "%s", *totalInfo);
				USB_LOG("Info: %s", accInfo);
				*totalInfo = ++finder;
				__USB_FUNC_EXIT__ ;
				return true;
			}
			finder++;
		}
	}
	__USB_FUNC_EXIT__ ;
	return false;
}

/* Get all element separated from a string which has all information of an accessory */
static int getAccInfo(char *totalInfo[], struct usb_accessory_s **accessory)
{
	__USB_FUNC_ENTER__ ;
	bool ret = getAccElement(totalInfo, (*accessory)->manufacturer);
	um_retvm_if(ret == false, -1, "FAIL: getAccElement(totalInfo, manufacturer)\n");
	ret = getAccElement(totalInfo, (*accessory)->model);
	um_retvm_if(ret == false, -1, "FAIL: getAccElement(totalInfo, model)\n");
	ret = getAccElement(totalInfo, (*accessory)->description);
	um_retvm_if(ret == false, -1, "FAIL: getAccElement(totalInfo, description)\n");
	ret = getAccElement(totalInfo, (*accessory)->version);
	um_retvm_if(ret == false, -1, "FAIL: getAccElement(totalInfo, version)\n");
	ret = getAccElement(totalInfo, (*accessory)->uri);
	um_retvm_if(ret == false, -1, "FAIL: getAccElement(totalInfo, uri)\n");
	ret = getAccElement(totalInfo, (*accessory)->serial);
	um_retvm_if(ret == false, -1, "FAIL: getAccElement(totalInfo, serial)\n");
	__USB_FUNC_EXIT__ ;
	return 0;
}

/* This function finds a list which contain all accessories attached
 * Currently, Tizen usb accessory is designed for just one accessory */
//bool getAccList(struct usb_accessory_s **accList)
bool getAccList(struct usb_accessory_list **accList)
{
	__USB_FUNC_ENTER__ ;
	if (*accList != NULL) return false;
	*accList = (struct usb_accessory_list *)malloc(sizeof(struct usb_accessory_list));
	struct usb_accessory_s *accessory = NULL;
	accessory = (struct usb_accessory_s *)malloc(sizeof(struct usb_accessory_s));
	(*accList)->next = NULL;
	(*accList)->accessory = accessory;
	accessory->accPermission = false;

	int ret = -1;
	int sock_remote;
	char buf[SOCK_STR_LEN];
	ret = ipc_request_client_init(&sock_remote);
	um_retvm_if(ret < 0, false, "FAIL: ipc_request_client_init(&sock_remote)\n");

	ret = request_to_usb_server(sock_remote, GET_ACC_INFO, buf, NULL);
	um_retvm_if(ret < 0, false, "FAIL: request_to_usb_server(GET_ACC_INFO)\n");
	USB_LOG("GET_ACC_INFO: %s\n", buf);

	ret = ipc_request_client_close(&sock_remote);
	um_retvm_if(ret < 0, false, "FAIL: ipc_request_client_close(&sock_remote)\n");

	char *tempInfo = buf;
	ret = getAccInfo(&tempInfo, &((*accList)->accessory));
	um_retvm_if(ret < 0, false, "FAIL: getAccInfo(&tempInfo, accList)\n");

	USB_LOG("Acc manufacturer: %s\n", (*accList)->accessory->manufacturer);
	USB_LOG("Acc model: %s\n", (*accList)->accessory->model);
	USB_LOG("Acc description: %s\n", (*accList)->accessory->description);
	USB_LOG("Acc version: %s\n", (*accList)->accessory->version);
	USB_LOG("Acc uri: %s\n", (*accList)->accessory->uri);
	USB_LOG("Acc serial: %s\n", (*accList)->accessory->serial);

	__USB_FUNC_EXIT__ ;
	return true;
}

/* Release memory of accessory list */
bool freeAccList(struct usb_accessory_list *accList)
{
	__USB_FUNC_ENTER__ ;
	if (accList == NULL) return true;
	struct usb_accessory_list *tmpList = NULL;
	while (accList) {
		tmpList = accList;
		accList = accList->next;
		FREE(tmpList->accessory);
		FREE(tmpList);
	}
	__USB_FUNC_EXIT__ ;
	return true;
}

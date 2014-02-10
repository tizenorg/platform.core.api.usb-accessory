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

#include "usb_accessory_private.h"

#include <tzplatform_config.h>

#define NUM_ACC_INFO_SEPARATOR 5

static void show_acc_info(struct usb_accessory_s *accessory)
{
	__USB_FUNC_ENTER__ ;
	if (!accessory)
		return;

	USB_LOG("*******************************");
	USB_LOG("** Acc manufacturer: %s", accessory->manufacturer);
	USB_LOG("** Acc model       : %s", accessory->model);
	USB_LOG("** Acc description : %s", accessory->description);
	USB_LOG("** Acc version     : %s", accessory->version);
	USB_LOG("** Acc uri         : %s", accessory->uri);
	USB_LOG("** Acc serial      : %s", accessory->serial);
	USB_LOG("*******************************");
	__USB_FUNC_EXIT__ ;
}

void free_accessory(struct usb_accessory_s *accessory)
{
	__USB_FUNC_ENTER__ ;

	if (accessory) {
		FREE(accessory->manufacturer);
		FREE(accessory->model);
		FREE(accessory->description);
		FREE(accessory->version);
		FREE(accessory->uri);
		FREE(accessory->serial);
		FREE(accessory);
	}
	__USB_FUNC_EXIT__ ;
}

/* Release memory of accessory list */
void free_acc_list(struct usb_accessory_list *accList)
{
	__USB_FUNC_ENTER__ ;

	struct usb_accessory_list *tmpList;
	struct usb_accessory_s *accessory;

	if (!accList)
		return;

	while (accList) {
		tmpList = accList;
		accList = accList->next;
		accessory = tmpList->accessory;

		free_accessory(accessory);
		FREE(tmpList);
	}

	__USB_FUNC_EXIT__ ;
}

/* This function initializes socket for ipc with usb-server */
int ipc_request_client_init()
{
	__USB_FUNC_ENTER__ ;

	int sock;
	struct sockaddr_un remote;

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		USB_LOG("FAIL: socket(AF_UNIX, SOCK_STREAM, 0)");
		return -1;
	}

	remote.sun_family = AF_UNIX;
	strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path));

	if (connect(sock, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
		USB_LOG("FAIL: connect(sock, (struct sockaddr *)&remote, len)");
		close(sock);
		return -1;
	}

	__USB_FUNC_EXIT__ ;
	return sock;
}

/* This function requests something to usb-server by ipc with socket and gets the results */
int request_to_usb_server(int sockRemote, int request, void *data, char *answer, int answerLen)
{
	__USB_FUNC_ENTER__ ;

	int t;
	char str[SOCK_STR_LEN];
	char *pkgName;

	if (!answer)
		return -1;

	switch (request) {
	case REQ_ACC_PERMISSION:
	case HAS_ACC_PERMISSION:
		pkgName = (char *)data;
		USB_LOG("request: %d, pkgName: %s", request, pkgName);
		snprintf(str, sizeof(str), "%d|%s", request, pkgName);
		break;
	default:
		USB_LOG("request: %d", request);
		snprintf(str, sizeof(str), "%d|", request);
		break;
	}

	if (send (sockRemote, str, strlen(str)+1, 0) == -1) {
		USB_LOG("FAIL: send (sock_remote, str, strlen(str)+1, 0)");
		return -1;
	}

	if ((t = recv(sockRemote, answer, answerLen, 0)) > 0) {
		if (t < answerLen) {
			answer[t] = '\0';
		} else { /* t == answerLen */
			answer[answerLen-1] = '\0';
		}
		USB_LOG("[CLIENT] Received value: %s", answer);

		__USB_FUNC_EXIT__ ;
		return 0;
	}

	USB_LOG("FAIL: recv(sock_remote, str, answerLen, 0)");

	__USB_FUNC_EXIT__ ;
	return -1;
}

static int handle_input_to_server(void *data, char *buf)
{
	__USB_FUNC_ENTER__ ;

	struct AccCbData *permCbData;
	int input;

	if (!data)
		return -1;
	if (!buf)
		return -1;

	permCbData = (struct AccCbData *)data;
	input = atoi(buf);
	USB_LOG("Input: %d", input);

	switch (input) {
	case REQ_ACC_PERM_NOTI_YES_BTN:
		permCbData->accessory->accPermission = true;
		permCbData->request_perm_cb_func(
				(struct usb_accessory_s*)(permCbData->user_data), true);
		break;
	case REQ_ACC_PERM_NOTI_NO_BTN:
		permCbData->accessory->accPermission = false;
		permCbData->request_perm_cb_func(
				(struct usb_accessory_s*)(permCbData->user_data), false);
		break;
	default:
		USB_LOG("FAIL: buf (%s) is improper", buf);
		return -1;
	}
	__USB_FUNC_EXIT__ ;
	return 0;
}

static int read_message(int fd, char *str, int len)
{
	__USB_FUNC_ENTER__;
	int ret;

	if (!str)
		return -1;

	while(1) {
		ret = read(fd, str, len);
		if (ret >= 0) {
			__USB_FUNC_EXIT__;
			return 0;
		}

		if (EINTR == errno) {
			USB_LOG("Re-read for error(EINTR)");
			continue;
		}

		USB_LOG("FAIL: read(fd, str, len)");
		return -1;
	}
}

int ipc_noti_client_init(void)
{
	__USB_FUNC_ENTER__ ;
	int sock_local;
	int ret;
	struct sockaddr_un serveraddr;

	sock_local = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_local < 0) {
		USB_LOG("FAIL: socket(AF_UNIX, SOCK_STREAM, 0)");
		return -1;
	}

	serveraddr.sun_family = AF_UNIX;
	strncpy(serveraddr.sun_path, ACC_SOCK_PATH, strlen(ACC_SOCK_PATH)+1);
	USB_LOG("socket file name: %s", serveraddr.sun_path);

	unlink(serveraddr.sun_path);

	if (bind (sock_local, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		USB_LOG("FAIL: bind (sock_local, (struct sockaddr_un *)serveraddr)");
		close(sock_local);
		return -1;
	}

	ret = chown(ACC_SOCK_PATH,tzplatform_getuid(TZ_USER_NAME),tzplatform_getgid(TZ_SYS_USER_GROUP));
	if (ret < 0) {
		USB_LOG("FAIL: chown(ACC_SOCK_PATH, tzplatform_getuid(TZ_USER_NAME),tzplatform_getgid(TZ_SYS_USER_GROUP))");
		close(sock_local);
		return -1;
	}

	ret = chmod(ACC_SOCK_PATH, 0777);
	if (ret < 0) {
		USB_LOG("FAIL: chmod(ACC_SOCK_PATH, 0777);");
		close(sock_local);
		return -1;
	}

	if (listen (sock_local, 5) == -1) {
		USB_LOG("FAIL: listen (sock_local, 5)");
		close(sock_local);
		return -1;
	}

	__USB_FUNC_EXIT__ ;
	return sock_local;
}

gboolean ipc_noti_client_cb(GIOChannel *gioCh, GIOCondition condition, gpointer data)
{
	__USB_FUNC_ENTER__ ;

	int fd;
	int ret;
	int clientlen;
	struct sockaddr_un clientaddr;
	int sockfd;
	char inbuf[SOCK_STR_LEN];
	char outbuf[SOCK_STR_LEN];
	GError *err;
	GIOStatus gret;

	if (!data)
		return FALSE;
	if (!gioCh)
		return FALSE;

	fd = g_io_channel_unix_get_fd(gioCh);
	if (fd < 0) {
		USB_LOG("FAIL: g_io_channel_unix_get_fd()");
		goto out_gio_unref;
	}

	clientlen = sizeof(clientaddr);
	sockfd = accept(fd, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
	if (sockfd == -1) {
		USB_LOG("FAIL: accept()");
		goto out_gio_unref;
	}

	if(read_message(sockfd, inbuf, sizeof(inbuf)) < 0) {
		USB_LOG("FAIL: read_message()");
		snprintf(outbuf, sizeof(outbuf), "%d", IPC_FAIL);
		if (0 > write(sockfd, &outbuf, sizeof(outbuf)))
			USB_LOG("FAIL: write()");
		goto out_ipc_fd;
	}

	USB_LOG("read(): %s", inbuf);
	snprintf(outbuf, sizeof(outbuf), "%d", IPC_SUCCESS);
	ret = write(sockfd, &outbuf, sizeof(outbuf));
	if (ret < 0)
		USB_LOG("FAIL: write()");

	ret = handle_input_to_server((void *)data, inbuf);
	if (ret < 0)
		USB_LOG("FAIL: handle_input_to_server()");

out_ipc_fd:
	close(sockfd);
out_gio_unref:
	gret = g_io_channel_shutdown(gioCh, TRUE, &err);
	if (G_IO_STATUS_ERROR == gret)
		USB_LOG("ERROR: g_io_channel_shutdown(gioCh)");
	g_io_channel_unref(gioCh);

	__USB_FUNC_EXIT__ ;
	return FALSE;
}

/* This function returns the app id.
 * After calling this function, the memory space should be freed */
char *get_app_id()
{
	__USB_FUNC_ENTER__ ;

	int ret;
	char appId[APP_ID_LEN];

	ret = aul_app_get_appid_bypid(getpid(), appId, sizeof(appId));
	if (ret != AUL_R_OK) {
		USB_LOG("FAIL: aul_app_get_appid_bypid()");
		return NULL;
	}

	USB_LOG("appId: %s", appId);

	__USB_FUNC_EXIT__ ;
	return strdup(appId);
}

/* Get an element from a string which has all information of an accessory */
static bool get_acc_element(char **totalInfo, char **info)
{
	__USB_FUNC_ENTER__ ;

	char *finder;

	if (!totalInfo || !(*totalInfo))
		return false;
	if (!info)
		return false;

	finder = *totalInfo;

	while (finder) {
		if (*finder != '|' && *finder != '\0') {
			finder++;
			continue;
		}

		*finder = '\0';
		*info = strdup(*totalInfo);
		USB_LOG("Info: %s", *info);
		*totalInfo = ++finder;
		__USB_FUNC_EXIT__ ;
		return true;
	}

	__USB_FUNC_EXIT__ ;
	return false;
}

/* Get all element separated from a string which has all information of an accessory */
static int get_acc_info(char *totalInfo, struct usb_accessory_s **accessory)
{
	__USB_FUNC_ENTER__ ;

	bool ret;
	char *tmpInfo;
	int numSepBar;

	if (!totalInfo)
		goto out_fail;
	if (!accessory || !(*accessory))
		goto out_fail;

	/* Check whether or not the format of the accessory info is correct to parse */
	tmpInfo = totalInfo;
	numSepBar = 0;
	while(*tmpInfo != '\0') {
		if (*tmpInfo == '|')
			numSepBar++;
		tmpInfo++;
	}
	if (numSepBar != NUM_ACC_INFO_SEPARATOR) {
		USB_LOG("FAIL: Format of string (%s) is improper", totalInfo);
		goto out_fail;
	}

	tmpInfo = totalInfo;

	ret = get_acc_element(&tmpInfo, &((*accessory)->manufacturer));
	if (!ret)
		goto out_fail;

	ret = get_acc_element(&tmpInfo, &((*accessory)->model));
	if (!ret)
		goto out_release_manufacturer;

	ret = get_acc_element(&tmpInfo, &((*accessory)->description));
	if (!ret)
		goto out_release_model;

	ret = get_acc_element(&tmpInfo, &((*accessory)->version));
	if (!ret)
		goto out_release_description;

	ret = get_acc_element(&tmpInfo, &((*accessory)->uri));
	if (!ret)
		goto out_release_version;

	ret = get_acc_element(&tmpInfo, &((*accessory)->serial));
	if (!ret)
		goto out_release_uri;

	(*accessory)->accPermission = false;

	__USB_FUNC_EXIT__ ;
	return 0;

out_release_uri:
	FREE((*accessory)->uri);
out_release_version:
	FREE((*accessory)->version);
out_release_description:
	FREE((*accessory)->description);
out_release_model:
	FREE((*accessory)->model);
out_release_manufacturer:
	FREE((*accessory)->manufacturer);
out_fail:
	__USB_FUNC_EXIT__ ;
	return -1;
}

/* This function finds a list which contain all accessories attached
 * Currently, Tizen usb accessory is designed for just one accessory */
bool get_acc_list(struct usb_accessory_list **accList)
{
	__USB_FUNC_ENTER__ ;

	struct usb_accessory_s *accessory;
	int ret;
	int sock_remote;
	char buf[SOCK_STR_LEN];

	if (!accList || !(*accList))
		return false;

	sock_remote = ipc_request_client_init();
	if (sock_remote < 0) {
		USB_LOG("FAIL: ipc_request_client_init()");
		return false;
	}

	ret = request_to_usb_server(sock_remote, GET_ACC_INFO, NULL, buf, sizeof(buf));
	if (ret < 0) {
		USB_LOG("FAIL: request_to_usb_server(GET_ACC_INFO)");
		close(sock_remote);
		return false;
	}
	USB_LOG("GET_ACC_INFO: %s", buf);

	close(sock_remote);

	accessory = (struct usb_accessory_s *)malloc(sizeof(struct usb_accessory_s));
	if (!accessory) {
		USB_LOG("FAIL: malloc()");
		return false;
	}

	ret = get_acc_info(buf, &accessory);
	if (0 > ret) {
		USB_LOG("FAIL: get_acc_info()");
		return false;
	}

	(*accList)->next = NULL;
	(*accList)->accessory = accessory;

	show_acc_info((*accList)->accessory);

	__USB_FUNC_EXIT__ ;
	return true;
}

/* This function find an accessory attached just now
 * Currently This function supports just one accessory */
static int get_changed_acc (struct usb_accessory_s *attAcc)
{
	__USB_FUNC_ENTER__ ;

	struct usb_accessory_list *accList;
	bool ret;

	if (!attAcc)
		return -1;

	accList = (struct usb_accessory_list *)malloc(sizeof(struct usb_accessory_list));
	if (!accList) {
		USB_LOG("FAIL: malloc()");
		return -1;
	}

	ret = get_acc_list(&accList);
	if (!ret) {
		USB_LOG("FAIL: get_acc_list()");
		free_acc_list(accList);
		return -1;
	}

	if (!(accList->accessory)) {
		USB_LOG("FAIL: accList->accessory == NULL");
		free_acc_list(accList);
		return -1;
	}

	attAcc->manufacturer = strdup(accList->accessory->manufacturer);
	attAcc->model = strdup(accList->accessory->model);
	attAcc->description = strdup(accList->accessory->description);
	attAcc->version = strdup(accList->accessory->version);
	attAcc->uri = strdup(accList->accessory->uri);
	attAcc->serial = strdup(accList->accessory->serial);

	free_acc_list(accList);

	__USB_FUNC_EXIT__ ;
	return 0;
}

/* Callback function which is called when accessory vconf key is changed */
void accessory_status_changed_cb(keynode_t *key, void* data)
{
	__USB_FUNC_ENTER__ ;

	struct AccCbData *conCbData;
	struct usb_accessory_s *changedAcc;
	int ret;
	int val;

	if (!data)
		return;

	conCbData = (struct AccCbData *)data;

	ret = vconf_get_int(VCONFKEY_USB_ACCESSORY_STATUS, &val);
	if (0 > ret) {
		USB_LOG("FAIL: vconf_get_int()");
		return ;
	}

	switch (val) {
	case VCONFKEY_USB_ACCESSORY_STATUS_DISCONNECTED:
		conCbData->connection_cb_func(NULL, false, conCbData->user_data);
		break;

	case VCONFKEY_USB_ACCESSORY_STATUS_CONNECTED:
		changedAcc = (struct usb_accessory_s *)malloc(sizeof(struct usb_accessory_s));
		if (!changedAcc) {
			USB_LOG("FAIL: malloc()");
			return;
		}

		ret = get_changed_acc(changedAcc);
		if (ret < 0) {
			USB_LOG("FAIL: get_changed_acc()");
			free_accessory(changedAcc);
			return;
		}

		conCbData->connection_cb_func(changedAcc, true, conCbData->user_data);

		free_accessory(changedAcc);

		break;
	default:
		USB_LOG("ERROR: The value of VCONFKEY_USB_ACCESSORY_STATUS is invalid");
		break;
	}
	__USB_FUNC_EXIT__ ;
}

bool is_emul_bin()
{
	__USB_FUNC_ENTER__ ;
	int ret = -1;
	struct utsname name;
	ret = uname(&name);
	if (ret < 0) {
		__USB_FUNC_EXIT__ ;
		return true;
	}

	USB_LOG("Machine name: %s", name.machine);
	if (strcasestr(name.machine, "emul")) {
		__USB_FUNC_EXIT__ ;
		return true;
	} else {
		__USB_FUNC_EXIT__ ;
		return false;
	}
}

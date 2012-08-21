
#include "usb_accessory.h"
#include <Elementary.h>
#include <dlog.h>

#define USB_TAG "USB_ACC_TEST"

#define ACC_LOG(format, args...) \
	LOG(LOG_VERBOSE, USB_TAG, "[%s][Ln: %d] " format, \
					(char*)(strrchr(__FILE__, '/')+1), __LINE__, ##args)

#define ACC_LOG_ERROR(format, args...) \
	LOG(LOG_ERROR, USB_TAG, "[%s][Ln: %d] " format, \
					(char*)(strrchr(__FILE__, '/')+1), __LINE__, ##args)

#define __ACC_FUNC_ENTER__ \
			ACC_LOG("Entering: %s()\n", __func__)

#define __ACC_FUNC_EXIT__ \
			ACC_LOG("Exit: %s()\n", __func__)

#define FREE(arg) \
	do { \
		if(arg) { \
			free((void *)arg); \
			arg = NULL; \
		} \
	} while (0);

usb_accessory_h usbAcc;
unsigned char input[3];
unsigned char output[3];

Evas_Object *win = NULL;
Evas_Object *bg = NULL;
Evas_Object *btn1 = NULL;
Evas_Object *btn2 = NULL;
Evas_Object *btn3 = NULL;
Evas_Object *btn4 = NULL;
Evas_Object *popup = NULL;

static void unload_popup();

static void _block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: _block_clicked_cb()");
	unload_popup();
	ACC_LOG("EXIT: _block_clicked_cb()");
}

static void _timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: _timeout_cb()");
	unload_popup();
	ACC_LOG("EXIT: _timeout_cb()");
}

static void unload_popup()
{
	ACC_LOG("ENTER: unload_popup()");
	evas_object_smart_callback_del(popup, "block,clicked", _block_clicked_cb);
	evas_object_smart_callback_del(popup, "timeout", _timeout_cb);
	if (popup) {
		evas_object_del(popup);
		popup = NULL;
	}
	ACC_LOG("EXIT: unload_popup()");
}

static void launch_popup(char *str)
{
	ACC_LOG("ENTER: launch_popup(char *str)");
	if (!win) return;
	if (popup) {
		evas_object_del(popup);
		popup = NULL;
	}
	popup = elm_popup_add(win);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, str);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	elm_popup_timeout_set(popup, 3.0);
	evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);
	evas_object_show(popup);
	ACC_LOG("EXIT: launch_popup(char *str)");
}

void connect_acc_cb(void *data)
{
	ACC_LOG("ENTER: connect_acc_cb()\n");
	ACC_LOG("EXIT: connect_acc_cb()\n");
}

void disconnect_acc_cb(void *data)
{
	ACC_LOG("ENTER: disconnect_acc_cb()\n");
	int ret = usb_accessory_connection_unset_cb();
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_connection_unset_cb()\n");
	}
	ret = usb_accessory_destroy(usbAcc);
	if (ret < 0){
		ACC_LOG("FAIL: usb_accessory_destroy(&usbAcc)\n");
	}
	elm_exit();
	ACC_LOG("EXIT: disconnect_acc_cb()\n");
}

void connection_cb_func(usb_accessory_h accessory, bool isConnected, void *data)
{
	ACC_LOG("ENTER: connection_cb_func()\n");
	if (isConnected == true) {
		connect_acc_cb(accessory);
	} else {
		disconnect_acc_cb(accessory);
	}
	ACC_LOG("EXIT: connection_cb_func()\n");
}

static void send_request()
{
	ACC_LOG("ENTER: send_request()\n");
	char str[64];
	FILE *fp = NULL;
	int ret = usb_accessory_open(usbAcc, &fp);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_open(usbAcc, fp)\n");
		return ;
	}
	ACC_LOG("fd : %d\n", (int)fp);
	if (fp <= 0) {
		disconnect_acc_cb(usbAcc);
	}

	ret = fwrite(input, 3, 1, fp);
	if (ret < 0) {
		ACC_LOG("FAIL: fwrite(input, 1, 3, fp)\n");
		return ;
	}
	ret = fread(output, 3, 1, fp);
	if (ret < 0) {
		ACC_LOG("FAIL: fread(output, 1, 3, fp)\n");
		return ;
	}
	ACC_LOG("OUTPUT: %d, %d, %d\n", output[0], output[1], output[2]);
	snprintf(str, 64, "Input: Type %d<br>Output: %d, %d, %d", input[1], output[0], output[1], output[2]);
	launch_popup(str);
	ret = fclose(fp);
	if (0 != ret) {
		ACC_LOG("FAIL: fclose(fp)\n");
		return ;
	}
	ACC_LOG("EXIT: send_request()\n");
}

static void on_click_1(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_1()\n");
	input[0] = 0x2;
	input[1] = 0x1;
	input[2] = 0x1;
	send_request();
	ACC_LOG("EXIT: on_click_1()\n");
}

static void on_click_2(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_2()\n");
	input[0] = 0x2;
	input[1] = 0x2;
	input[2] = 0x1;
	send_request();
	ACC_LOG("EXIT: on_click_2()\n");
}

static void on_click_3(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_3()\n");
	input[0] = 0x2;
	input[1] = 0x3;
	input[2] = 0x1;
	send_request();
	ACC_LOG("EXIT: on_click_3()\n");
}

static void on_click_4(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_4()\n");
	disconnect_acc_cb(usbAcc);
	ACC_LOG("EXIT: on_click_4()\n");
}

static void register_btn_cb(usb_accessory_h accessory, bool isGranted)
{
	ACC_LOG("ENTER: register_btn_cb()\n");
	if (isGranted != true) {
		disconnect_acc_cb(usbAcc);
		return ;
	}
	evas_object_smart_callback_add(btn1, "clicked", on_click_1, NULL);
	evas_object_smart_callback_add(btn2, "clicked", on_click_2, NULL);
	evas_object_smart_callback_add(btn3, "clicked", on_click_3, NULL);
	evas_object_smart_callback_add(btn4, "clicked", on_click_4, NULL);
	ACC_LOG("EXIT: register_btn_cb()\n");
}

bool foreach_cb(usb_accessory_h handle, void *data)
{
	ACC_LOG("ENTER: foreach_cb()\n");
	if (!handle) return false;
	bool ret_val = true;
	int ret = -1;
	char *manufacturer = NULL;
	char *model = NULL;
	char *version = NULL;
	ret = usb_accessory_get_manufacturer(handle, &manufacturer);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_get_manufacturer(handle, &manufacturer)\n");
		return true;
	}
	ret = usb_accessory_get_model(handle, &model);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_get_model(handle, &model)\n");
		return true;
	}
	ret = usb_accessory_get_version(handle, &version);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_get_version(handle, &version)");
		return true;
	}

	if (!strncmp(manufacturer, "Google, Inc.", strlen("Google, Inc."))
				&& !strncmp(model, "DemoKit", strlen("DemoKit"))
				&& !strncmp(version, "1.0", strlen("1.0"))) {
		ret = usb_accessory_clone(handle, &usbAcc);
		if (ret != USB_ERROR_NONE) {
			ACC_LOG("FAIL: usb_accessory_clone(handle, &usbAcc)\n");
			ret_val = true;
		} else {
			ret_val = false;
		}
	} else {
		ret_val = true;
	}
	FREE(manufacturer);
	FREE(model);
	FREE(version);

	ACC_LOG("EXIT: foreach_cb()\n");
	return ret_val;
}

int main(int argc, char **argv)
{
	ACC_LOG("ENTER: MAIN()");
	bool ret = false;
	usbAcc = NULL;

	ret = usb_accessory_set_connection_changed_cb(connection_cb_func, usbAcc);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_set_connection_changed_cb()\n");
		return -1;
	}
	/* Check whether or not accessory is connected */
	bool connected;
	ret = usb_accessory_is_connected(NULL, &connected);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_is_connected(NULL, &connected)\n");
		return -1;
	}
	if (connected == false) {
		ACC_LOG("Accessory is not connected\n");
		disconnect_acc_cb(NULL);
		exit(0);
	}

	/* Retrieve an accessory which is one of the accessories connected and matches to this app */
	ret = usb_accessory_foreach_attached(foreach_cb, &usbAcc);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_foreach_attached(foreach_cb, usbAcc)\n");
		disconnect_acc_cb(NULL);
		return -1;
	}
	if (usbAcc == NULL) {
		ACC_LOG("Accessory is NULL\n");
		disconnect_acc_cb(NULL);
		return -1;
	}

	/* Check whether or not this app has permission to open the accessory */
	bool perm;
	ret = usb_accessory_has_permission(usbAcc, &perm);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_has_permission(usbAcc)\n");
		return -1;
	}

	elm_init(argc, argv);
	win = elm_win_add(NULL, "Accessory", ELM_WIN_BASIC);
	elm_win_title_set(win, "Usb Accessory Test");
	elm_win_autodel_set(win, EINA_TRUE);
	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
	evas_object_resize(win, 240, 60);
	evas_object_show(win);

	bg = elm_bg_add(win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	btn1 = elm_button_add(win);
	elm_object_text_set(btn1, "Type 1");
	evas_object_resize(btn1, 300, 200);
	evas_object_move(btn1, 100, 200);
	evas_object_show(btn1);

	btn2 = elm_button_add(win);
	elm_object_text_set(btn2, "Type 2");
	evas_object_resize(btn2, 300, 200);
	evas_object_move(btn2, 100,450);
	evas_object_show(btn2);

	btn3 = elm_button_add(win);
	elm_object_text_set(btn3, "Type 3");
	evas_object_resize(btn3, 300, 200);
	evas_object_move(btn3, 100, 700);
	evas_object_show(btn3);

	btn4 = elm_button_add(win);
	elm_object_text_set(btn4, "Exit");
	evas_object_resize(btn4, 300, 200);
	evas_object_move(btn4, 100, 950);
	evas_object_show(btn4);

	if (perm == true) {
		register_btn_cb(usbAcc, true);
	} else {
		ret = usb_accessory_request_permission(usbAcc, register_btn_cb, NULL);
		if (ret < 0) {
			disconnect_acc_cb(NULL);
		}
	}

	elm_run();

	elm_shutdown();
	return 0;
}

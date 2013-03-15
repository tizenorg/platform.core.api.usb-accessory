
#include <usb_accessory.h>
#include <Elementary.h>
#include <dlog.h>

#undef LOG_TAG
#define LOG_TAG "USB_ACC_TEST"

#define ACC_LOG(fmt, args...)   SLOGD(fmt, ##args)

#define FREE(arg) \
	do { \
		if(arg) { \
			free((void *)arg); \
			arg = NULL; \
		} \
	} while (0);

usb_accessory_h usbAcc;

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
	ACC_LOG("ENTER: connect_acc_cb()");
	ACC_LOG("EXIT: connect_acc_cb()");
}

void disconnect_acc_cb(void *data)
{
	ACC_LOG("ENTER: disconnect_acc_cb()");
	int ret = usb_accessory_connection_unset_cb();
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_connection_unset_cb()");
	}
	ret = usb_accessory_destroy(usbAcc);
	if (ret < 0){
		ACC_LOG("FAIL: usb_accessory_destroy(&usbAcc)");
	}
	elm_exit();
	ACC_LOG("EXIT: disconnect_acc_cb()");
}

void connection_cb_func(usb_accessory_h accessory, bool isConnected, void *data)
{
	ACC_LOG("ENTER: connection_cb_func()");
	if (isConnected == true) {
		connect_acc_cb(accessory);
	} else {
		disconnect_acc_cb(accessory);
	}
	ACC_LOG("EXIT: connection_cb_func()");
}

static void send_request(unsigned char input[], int len)
{
	ACC_LOG("ENTER: send_request()");
	char str[64];
	FILE *fp;
	int ret;
	unsigned char output[3];

	ret = usb_accessory_open(usbAcc, &fp);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_open(usbAcc, fp)");
		return ;
	}
	if (!fp) {
		disconnect_acc_cb(usbAcc);
		return ;
	}

	ACC_LOG("Before fwrite(), len: %d", len);

	ret = fwrite(input, len, 1, fp);
	if (ret < 0) {
		ACC_LOG("FAIL: fwrite()");
		return ;
	}

	ACC_LOG("After fwrite()");

	ret = fread(output, sizeof(output), 1, fp);
	if (ret < 0) {
		ACC_LOG("FAIL: fread()");
		return ;
	}

	ACC_LOG("OUTPUT: %d, %d, %d", output[0], output[1], output[2]);
	snprintf(str, sizeof(str), "Input: Type %d<br>Output: %d, %d, %d",
				input[1], output[0], output[1], output[2]);
	launch_popup(str);

	ret = fclose(fp);
	if (0 != ret) {
		ACC_LOG("FAIL: fclose()");
		return ;
	}

	ACC_LOG("EXIT: send_request()");
}

static void on_click_1(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_1()");
	unsigned char input[3];

	input[0] = 0x2;
	input[1] = 0x1;
	input[2] = 0x1;

	send_request(input, sizeof(input));
	ACC_LOG("EXIT: on_click_1()");
}

static void on_click_2(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_2()");
	unsigned char input[3];

	input[0] = 0x2;
	input[1] = 0x2;
	input[2] = 0x1;

	send_request(input, sizeof(input));
	ACC_LOG("EXIT: on_click_2()");
}

static void on_click_3(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_3()");
	unsigned char input[3];

	input[0] = 0x2;
	input[1] = 0x3;
	input[2] = 0x1;

	send_request(input, sizeof(input));
	ACC_LOG("EXIT: on_click_3()");
}

static void on_click_4(void *data, Evas_Object *obj, void *event_info)
{
	ACC_LOG("ENTER: on_click_4()");
	disconnect_acc_cb(usbAcc);
	ACC_LOG("EXIT: on_click_4()");
}

static void register_btn_cb(usb_accessory_h accessory, bool isGranted)
{
	ACC_LOG("ENTER: register_btn_cb()");
	if (!isGranted) {
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
	ACC_LOG("ENTER: foreach_cb()");
	int ret;
	char *manufacturer;
	char *model;
	char *version;

	if (!handle)
		return false;

	ret = usb_accessory_get_manufacturer(handle, &manufacturer);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_get_manufacturer()");
		goto out_manufacturer;
	}

	ret = usb_accessory_get_model(handle, &model);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_get_model()");
		goto out_model;
	}

	ret = usb_accessory_get_version(handle, &version);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_get_version()");
		goto out_version;
	}

	ACC_LOG("Manufacturer(%s), model(%s), version(%s)", manufacturer, model, version);

	if (!strncmp(manufacturer, "Tizen", strlen("Tizen"))
			&& !strncmp(model, "DemoKit", strlen("DemoKit"))
			&& !strncmp(version, "1.0", strlen("1.0"))) {
		ret = usb_accessory_clone(handle, &usbAcc);
		if (ret == USB_ERROR_NONE) {
			FREE(version);
			FREE(model);
			FREE(manufacturer);
			ACC_LOG("EXIT: foreach_cb()");
			return false;
		}
	}

out_version:
	FREE(version);
out_model:
	FREE(model);
out_manufacturer:
	FREE(manufacturer);

	ACC_LOG("EXIT: foreach_cb()");
	return true;
}

int main(int argc, char **argv)
{
	ACC_LOG("ENTER: MAIN()");
	bool ret;
	bool connected;
	bool perm;
	usbAcc = NULL;

	ret = usb_accessory_set_connection_changed_cb(connection_cb_func, usbAcc);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_set_connection_changed_cb()");
		return -1;
	}

	/* Check whether or not accessory is connected */
	ret = usb_accessory_is_connected(NULL, &connected);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_is_connected(NULL, &connected)");
		return -1;
	}
	if (!connected) {
		ACC_LOG("Accessory is not connected");
		disconnect_acc_cb(NULL);
		exit(0);
	}

	/* Retrieve an accessory which is one of the accessories connected and matches to this app */
	ret = usb_accessory_foreach_attached(foreach_cb, &usbAcc);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_foreach_attached(foreach_cb, usbAcc)");
		disconnect_acc_cb(NULL);
		return -1;
	}
	if (!usbAcc) {
		ACC_LOG("Accessory is NULL");
		disconnect_acc_cb(NULL);
		return -1;
	}

	/* Check whether or not this app has permission to open the accessory */
	ret = usb_accessory_has_permission(usbAcc, &perm);
	if (ret < 0) {
		ACC_LOG("FAIL: usb_accessory_has_permission(usbAcc)");
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

	if (perm) {
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

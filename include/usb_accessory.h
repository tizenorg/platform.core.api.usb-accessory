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

#ifndef __TIZEN_SYSTEM_USB_ACCESSORY_H__
#define __TIZEN_SYSTEM_USB_ACCESSORY_H__

#include <stdio.h>
#include <tizen.h>

/**
 * @addtogroup CAPI_SYSTEM_USB_ACCESSORY_MODULE
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerations of error code for usb accessory.
 */
typedef enum
{
    USB_ERROR_NONE              = TIZEN_ERROR_NONE,
    USB_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,
    USB_ERROR_NOT_CONNECTED     = TIZEN_ERROR_ENDPOINT_NOT_CONNECTED,
    USB_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED,
    USB_ERROR_OPERATION_FAILED  = TIZEN_ERROR_SYSTEM_CLASS | 0x62,
    USB_ERROR_IPC_FAILED        = TIZEN_ERROR_PROTOCOL_NOT_AVALIABLE,
    USB_ERROR_NOT_SUPPORTED     = TIZEN_ERROR_NOT_SUPPORT_API
} usb_error_e;

/**
 * @brief The USB Accessory handle.
 */
typedef struct usb_accessory_s* usb_accessory_h;

/**
 * @brief Called when the usb accessory is connected or disconnected.
 *
 * @remark
 * the handle of accessory will be free after end of usb_accessory_connection_cb()
 *
 * @param[in] accessory     The handle of the attached usb accessory.
 * @param[in] is_connected   True when connected or False when connection is lost.
 * @param[in] data          The user data passed from the register function.
 *
 * @see usb_accessory_connection_set_cb()
 */
typedef void (*usb_accessory_connection_changed_cb)(usb_accessory_h accessory, bool is_connected, void *data);

/**
 * @brief Called when the permission of usb accessory is granted.
 *
 * @remark
 * the handle of accessory will be free after end of usb_accessory_request_permission_cb()
 *
 * @param[in] accessory     The handle of the attached usb accessory.
 * @param[in] is_granted     The permission of usb accessory.
 *
 * @see usb_accessory_request_permission()
 */
typedef void (*usb_accessory_permission_response_cb)(usb_accessory_h accessory, bool is_granted);

/**
 * @brief Called to retrieve the handles of usb accessory.
 *
 * @remark
 * the handle of accessory will be free after end of usb_accessory_attached_cb()
 *
 * @param[in] handle        The handle of the attached usb accessory.
 * @param[in] data          The user data passed from the foreach function.
 *
 * @return                  @c true to continue with the next iteration of the loop, \n
 *                          @c false to break out of the loop.
 *
 * @see usb_accessory_foreach_attached()
 */
typedef bool (*usb_accessory_attached_cb)(usb_accessory_h handle, void *data);

/**
 * @brief Clone the handle of usb accessory.
 * 
 * @remark
 * the cloned handle must be destroyed by #usb_accessory_destroy()
 *
 * @param[in]  handle           The usb accessory handle that want to copy.
 * @param[out] cloned_handle    The cloned usb accessory handle.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 *
 * @see usb_accessory_destroy()
 */
int usb_accessory_clone(usb_accessory_h handle, usb_accessory_h* cloned_handle);

/**
 * @brief Destroy the handle of usb accessory.
 * 
 * @param[in] handle        The handle of the attached usb accessory.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_destroy(usb_accessory_h handle);

/**
 * @brief Retrieves every attached usb accessory handles.
 * @details 
 * usb_accessory_attached_cb() will be called once for each handle of attached usb accessory.
 * if usb_accessory_attached_cb() callback function returns false, then iteration will be finished.
 *
 * @remark
 * the handle of accessory will be free after end of usb_accessory_attached_cb().
 *
 * @param[in] callback      The iteration callback function.
 * @param[in] user_data     The user data to be passed to the callback function.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_foreach_attached(usb_accessory_attached_cb callback, void *user_data);

/**
 * @brief Register callback function to be invoked when usb accessory connected or disconnected.
 * @details
 * The handle of attached usb accessory handle will be passed to usb_accessory_connection_cb().
 * And status of connection will be also passed to callback function.
 * 
 * @param[in] accessory     The attached usb accessory handle.
 * @param[in] callback      The callback function to register.
 * @param[in] user_data     The user data to be passed to the callback function.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 * 
 * @see usb_accessory_connection_unset_cb()
 * 
 */
int usb_accessory_set_connection_changed_cb(usb_accessory_connection_changed_cb callback, void* user_data); 

/**
 * @brief Unregister the usb accessory connection callback function.
 * 
 * @param[in] accessory     The attached usb accessory handle.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 * 
 * @see usb_accessory_connection_set_cb()
 */
int usb_accessory_connection_unset_cb(void); 

/**
 * @brief Check whether or not the accessory has permission to access to the host.
 *
 * @param[in]  accessory     The attached usb accessory handle.
 * @param[out] is_granted    The permission to access to the host.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_has_permission(usb_accessory_h accessory, bool *is_granted);

/**
 * @brief opens a file descriptor for reading and writing data to the usb accessory.
 *
 * @param[in]  accessory     The attached usb accessory handle.
 * @param[out] fd            opened File descriptor for usb accessory.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_open(usb_accessory_h accessory, FILE **fd);

/**
 * @brief Get description of the accessory.
 *
 * @param[in]  accessory     The attached usb accessory handle.
 * @param[out] description   The description of the usbaccessory.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_get_description(usb_accessory_h accessory, char** description);

/**
 * @brief Get manufacturer name of the accessory.
 *
 * @param[in]  accessory     The attached usb accessory handle.
 * @param[out] manufacturer  The name of manufacturer.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_get_manufacturer(usb_accessory_h accessory, char** manufacturer);

/**
 * @brief Get model name of the accessory.
 *
 * @param[in]  accessory     The attached usb accessory handle.
 * @param[out] model         The model name of usb accessory.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_get_model(usb_accessory_h accessory, char** model);

/**
 * @brief Get unique serial number of the accessory.
 *
 * @param[in]  accessory     The attached usb accessory handle.
 * @param[out] serial        The serial of usb accessory.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_get_serial(usb_accessory_h accessory, char** serial);

/**
 * @brief Get version of the accessory.
 *
 * @param[in]  accessory     The attached usb accessory handle.
 * @param[out] version       The version of usb accessory.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_get_version(usb_accessory_h accessory, char** version);

/**
 * @brief Check whether or not the connection of the usb accessory.
 *
 * @param[in]  accessory     The usb accessory handle to check.
 * @param[out] is_connected  The connection status.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 */
int usb_accessory_is_connected(usb_accessory_h accessory, bool *is_connected);

/**
 * @brief Request permission to user for usb accessory.
 * @details
 * The usb accessory handle and boolean passed to callback function that indicating whether permission was granted by the user.
 * 
 * @param[in] accessory     The attached usb accessory handle.
 * @param[in] callback      The callback function to register.
 * @param[in] user_data     The user data to be passed to the callback function.
 *
 * @return                  0 on success, otherwise a negative error value
 * @retval                  #USB_ERROR_NONE                 Successful
 * @retval                  #USB_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval                  #USB_ERROR_OPERATION_FAILED     Operation failed
 *
 */
int usb_accessory_request_permission(usb_accessory_h accessory, usb_accessory_permission_response_cb callback, void* user_data);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // __TIZEN_SYSTEM_USB_ACCESSORY_H__


/**
 * \file            esp_mqtt_client.h
 * \brief           MQTT client
 */

/*
 * Copyright (c) 2018 Tilen Majerle
 *  
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of ESP-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#ifndef __ESP_APP_MQTT_CLIENT_H
#define __ESP_APP_MQTT_CLIENT_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "esp/esp.h"
#include "esp/apps/esp_mqtt_client_evt.h"
    
/**
 * \ingroup         ESP_APPS
 * \defgroup        ESP_APP_MQTT_CLIENT MQTT client
 * \brief           MQTT client
 * \{
 */

/**
 * \brief           Maximal number of open connections at a time
 * 
 * \note            This is default value. To change it, override value in `esp_config.h` configuration file
 */
#ifndef ESP_CFG_MQTT_MAX_REQUESTS
#define ESP_CFG_MQTT_MAX_REQUESTS       8
#endif

/**
 * \brief           Quality of service enumeration
 */
typedef enum {
    ESP_MQTT_QOS_AT_MOST_ONCE = 0x00,           /*!< Delivery is not guaranteed to arrive, but can arrive `up to 1 time` = non-critical packets where losses are allowed */
    ESP_MQTT_QOS_AT_LEAST_ONCE = 0x01,          /*!< Delivery is quaranteed `at least once`, but it may be delivered multiple times with the same content */
    ESP_MQTT_QOS_EXACTLY_ONCE = 0x02,           /*!< Delivery is quaranteed `exactly once` = very critical packets such as billing informations or similar */
} esp_mqtt_qos_t;

struct esp_mqtt_client;

/**
 * \brief           Pointer to \ref esp_mqtt_client_t structure
 */
typedef struct esp_mqtt_client* esp_mqtt_client_p;

/**
 * \brief           State of MQTT client
 */
typedef enum {
    ESP_MQTT_CONN_DISCONNECTED = 0x00,          /*!< Connection with server is not established */
    ESP_MQTT_CONN_CONNECTING,                   /*!< Client is connecting to server */
    ESP_MQTT_CONN_DISCONNECTING,                /*!< Client connection is disconnecting from server */
    ESP_MQTT_CONNECTING,                        /*!< MQTT client is connecting... CONNECT command has been sent to server */
    ESP_MQTT_CONNECTED,                         /*!< MQTT is fully connected and ready to send data on topics */
} esp_mqtt_state_t;

/**
 * \brief           MQTT client information structure
 */
typedef struct {
    const char* id;                             /*!< Client unique identifier. It is required and must be set by user */
    
    const char* user;                           /*!< Authentication username. Set to `NULL` if not required */
    const char* pass;                           /*!< Authentication password, set to `NULL` if not required */
    
    uint16_t keep_alive;                        /*!< Keep-alive parameter in units of seconds.
                                                    When set to `0`, functionality is disabled (not recommended) */
    
    const char* will_topic;                     /*!< Will topic */
    const char* will_message;                   /*!< Will message */
    esp_mqtt_qos_t will_qos;                    /*!< Will topic quality of service */
} esp_mqtt_client_info_t;

/**
 * \brief           MQTT request object
 */
typedef struct {
    uint8_t status;                             /*!< Entry status flag for in use or pending bit */
    uint16_t packet_id;                         /*!< Packet ID generated by client on publish */
    
    void* arg;                                  /*!< User defined argument */
    uint32_t expected_sent_len;                 /*!< Number of total bytes which must be sent
                                                    on connection before we can say "packet was sent". */
    
    uint32_t timeout_start_time;                /*!< Timeout start time in units of milliseconds */
} esp_mqtt_request_t;

/**
 * \brief           MQTT event types
 */
typedef enum {
    ESP_MQTT_EVT_CONNECT,                       /*!< MQTT client connect event */
    ESP_MQTT_EVT_SUBSCRIBE,                     /*!< MQTT client subscribed to specific topic */
    ESP_MQTT_EVT_UNSUBSCRIBE,                   /*!< MQTT client unsubscribed from specific topic */
    ESP_MQTT_EVT_PUBLISH,                       /*!< MQTT client publish message to server event.
                                                    \note   When publishing packet with quality of service \ref ESP_MQTT_QOS_AT_MOST_ONCE,
                                                            you may not receive event, even if packet was successfully sent,
                                                            thus do not rely on this event for packet with `qos = ESP_MQTT_QOS_AT_MOST_ONCE` */
    ESP_MQTT_EVT_PUBLISH_RECV,                  /*!< MQTT client received a publish message from server */
    ESP_MQTT_EVT_DISCONNECT,                    /*!< MQTT client disconnected from MQTT server */
    ESP_MQTT_EVT_KEEP_ALIVE,                    /*!< MQTT keep-alive sent to server and reply received */
} esp_mqtt_evt_type_t;

/**
 * \brief           List of possible results from MQTT server when executing connect command
 */
typedef enum {
    ESP_MQTT_CONN_STATUS_ACCEPTED =                 0x00,   /*!< Connection accepted and ready to use */
    ESP_MQTT_CONN_STATUS_REFUSED_PROTOCOL_VERSION = 0x01,   /*!< Connection Refused, unacceptable protocol version */
    ESP_MQTT_CONN_STATUS_REFUSED_ID =               0x02,   /*!< Connection refused, identifier rejected  */
    ESP_MQTT_CONN_STATUS_REFUSED_SERVER =           0x03,   /*!< Connection refused, server unavailable */
    ESP_MQTT_CONN_STATUS_REFUSED_USER_PASS =        0x04,   /*!< Connection refused, bad user name or password */
    ESP_MQTT_CONN_STATUS_REFUSED_NOT_AUTHORIZED =   0x05,   /*!< Connection refused, not authorized */
    ESP_MQTT_CONN_STATUS_TCP_FAILED =               0x100,  /*!< TCP connection to server was not successful */
} esp_mqtt_conn_status_t;

/**
 * \brief           MQTT event structure for callback function
 */
typedef struct {
    esp_mqtt_evt_type_t type;                   /*!< Event type */
    union {
        struct {
            esp_mqtt_conn_status_t status;      /*!< Connection status with MQTT */
        } connect;                              /*!< Event for connecting to server */
        struct {
            uint8_t is_accepted;                /*!< Status if client was accepted to MQTT prior disconnect event */
        } disconnect;
        struct {
            void* arg;                          /*!< User argument for callback function */
            espr_t res;                         /*!< Response status */
        } sub_unsub_scribed;                    /*!< Event for (un)subscribe to/from topics */
        struct {
            void* arg;                          /*!< User argument for callback function */
            espr_t res;                         /*!< Response status */
        } publish;                              /*!< Published event */
        struct {
            const uint8_t* topic;               /*!< Pointer to topic identifier */
            size_t topic_len;                   /*!< Length of topic */
            const void* payload;                /*!< Topic payload */
            size_t payload_len;                 /*!< Length of topic payload */
            uint8_t dup;                        /*!< Duplicate flag if message was sent again */
            esp_mqtt_qos_t qos;                 /*!< Received packet quality of service */
        } publish_recv;                         /*!< Publish received event */
    } evt;                                      /*!< Event data parameters */
} esp_mqtt_evt_t;

/**
 * \brief           MQTT event callback function
 * \param[in]       client: MQTT client
 * \param[in]       evt: MQTT event with type and related data
 */
typedef void        (*esp_mqtt_evt_fn)(esp_mqtt_client_p client, esp_mqtt_evt_t* evt);

esp_mqtt_client_p   esp_mqtt_client_new(size_t tx_buff_len, size_t rx_buff_len);
void                esp_mqtt_client_delete(esp_mqtt_client_p client);

espr_t              esp_mqtt_client_connect(esp_mqtt_client_p client, const char* host, esp_port_t port, esp_mqtt_evt_fn evt_fn, const esp_mqtt_client_info_t* info);
espr_t              esp_mqtt_client_disconnect(esp_mqtt_client_p client);
uint8_t             esp_mqtt_client_is_connected(esp_mqtt_client_p client);

espr_t              esp_mqtt_client_subscribe(esp_mqtt_client_p client, const char* topic, esp_mqtt_qos_t qos, void* arg);
espr_t              esp_mqtt_client_unsubscribe(esp_mqtt_client_p client, const char* topic, void* arg);

espr_t              esp_mqtt_client_publish(esp_mqtt_client_p client, const char* topic, const void* payload, uint16_t len, esp_mqtt_qos_t qos, uint8_t retain, void* arg);

void*               esp_mqtt_client_get_arg(esp_mqtt_client_p client);
void                esp_mqtt_client_set_arg(esp_mqtt_client_p client, void* arg);
    
/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __ESP_APP_MQTT_CLIENT_H */

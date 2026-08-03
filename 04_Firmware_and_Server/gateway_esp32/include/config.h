#pragma once
#include <Arduino.h>
#include <IPAddress.h>

#define GATEWAY_ID        "gateway01"
#define FIRMWARE_VERSION  "1.0.0-revA"
#define MESH_PREFIX       "kp_motor_monitor_mesh"
#define MESH_PASSWORD     "CHANGE_ME_FOR_DEPLOYMENT"
#define MESH_PORT         5555

#define ETH_CS_PIN       5
#define ETH_SCK_PIN     18
#define ETH_MISO_PIN    19
#define ETH_MOSI_PIN    23
#define ETH_RST_PIN     -1
static byte GATEWAY_MAC[] = {0xDE,0xAD,0xBE,0xEF,0xFE,0x01};
#define ETH_USE_DHCP true
static IPAddress STATIC_IP(192,168,1,50);
static IPAddress STATIC_GATEWAY(192,168,1,1);
static IPAddress STATIC_SUBNET(255,255,255,0);
static IPAddress STATIC_DNS(192,168,1,1);

static const char* MQTT_BROKER_HOST = "192.168.1.10";
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "esp32_gateway_01"
static const char* MQTT_USERNAME = "kp_mqtt_user";
static const char* MQTT_PASSWORD = "CHANGE_ME";
#define MQTT_TOPIC_GATEWAY_STATUS "kp/gateway/status"
#define MQTT_TOPIC_EDGE_PREFIX "kp/edge/"
#define MQTT_TOPIC_EDGE_SUFFIX "/features"
#define MQTT_RECONNECT_INTERVAL_MS 5000UL
#define GATEWAY_STATUS_INTERVAL_MS 30000UL
#define MQTT_BUFFER_BYTES 6144
#define QUEUE_CAPACITY 8

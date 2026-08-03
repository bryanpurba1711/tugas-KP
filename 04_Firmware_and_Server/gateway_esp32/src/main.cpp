#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include "config.h"

Scheduler scheduler;
painlessMesh mesh;
EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);

struct QueueItem {
    bool used = false;
    uint32_t meshFrom = 0;
    String nodeId;
    uint32_t seq = 0;
    String topic;
    String payload;
};
QueueItem queueItems[QUEUE_CAPACITY];
uint32_t relayedCount = 0, queuedCount = 0, rejectedCount = 0;
uint32_t lastReconnectAttempt = 0, lastStatusPublish = 0;

void sendAck(uint32_t destination, const String &nodeId, uint32_t seq, const char *status) {
    StaticJsonDocument<256> ack;
    ack["type"] = "ack";
    ack["gateway_id"] = GATEWAY_ID;
    ack["node_id"] = nodeId;
    ack["seq"] = seq;
    ack["status"] = status;
    String out;
    serializeJson(ack, out);
    mesh.sendSingle(destination, out);
}

bool queuePayload(uint32_t from, const String &nodeId, uint32_t seq,
                  const String &topic, const String &payload) {
    for (auto &item : queueItems) {
        if (!item.used) {
            item.used = true; item.meshFrom = from; item.nodeId = nodeId;
            item.seq = seq; item.topic = topic; item.payload = payload;
            ++queuedCount;
            return true;
        }
    }
    return false;
}

bool ethernetConnect() {
    SPI.begin(ETH_SCK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
    Ethernet.init(ETH_CS_PIN);
    bool ok = false;
    if (ETH_USE_DHCP) ok = Ethernet.begin(GATEWAY_MAC) != 0;
    if (!ok) {
        Ethernet.begin(GATEWAY_MAC, STATIC_IP, STATIC_DNS, STATIC_GATEWAY, STATIC_SUBNET);
        ok = true;
    }
    if (Ethernet.hardwareStatus() == EthernetNoHardware) return false;
    return ok;
}

bool mqttReconnect() {
    bool connected = strlen(MQTT_USERNAME) > 0
        ? mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                             MQTT_TOPIC_GATEWAY_STATUS, 1, true, "{\"status\":\"offline\"}")
        : mqttClient.connect(MQTT_CLIENT_ID);
    if (connected) {
        mqttClient.publish(MQTT_TOPIC_GATEWAY_STATUS,
                           "{\"status\":\"online\"}", true);
    }
    return connected;
}

void flushQueue() {
    if (!mqttClient.connected()) return;
    for (auto &item : queueItems) {
        if (!item.used) continue;
        if (!mqttClient.publish(item.topic.c_str(), item.payload.c_str())) return;
        ++relayedCount;
        item = QueueItem{};
    }
}

void onMeshReceive(uint32_t from, String &message) {
    StaticJsonDocument<6144> doc;
    if (deserializeJson(doc, message)) { ++rejectedCount; return; }
    if (strcmp(doc["type"] | "", "condition_features") != 0) { ++rejectedCount; return; }
    const char *node = doc["node_id"] | "";
    const uint32_t seq = doc["seq"] | 0;
    if (!strlen(node) || seq == 0) { ++rejectedCount; return; }
    const String nodeId(node);
    const String topic = String(MQTT_TOPIC_EDGE_PREFIX) + nodeId + MQTT_TOPIC_EDGE_SUFFIX;

    if (mqttClient.connected() && mqttClient.publish(topic.c_str(), message.c_str())) {
        ++relayedCount;
        sendAck(from, nodeId, seq, "published");
        return;
    }
    if (queuePayload(from, nodeId, seq, topic, message)) {
        sendAck(from, nodeId, seq, "queued");
    } else {
        ++rejectedCount; // no ACK: node will retry and retain sequence for next cycle
    }
}

void publishGatewayStatus() {
    StaticJsonDocument<384> doc;
    doc["status"] = "online";
    doc["gateway_id"] = GATEWAY_ID;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["uptime_ms"] = millis();
    doc["mesh_nodes"] = mesh.getNodeList().size();
    doc["messages_relayed"] = relayedCount;
    doc["messages_queued_total"] = queuedCount;
    doc["messages_rejected"] = rejectedCount;
    doc["eth_ip"] = Ethernet.localIP().toString();
    String out; serializeJson(doc, out);
    mqttClient.publish(MQTT_TOPIC_GATEWAY_STATUS, out.c_str(), true);
}

void setup() {
    Serial.begin(115200);
    delay(200);
    ethernetConnect();
    mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    mqttClient.setBufferSize(MQTT_BUFFER_BYTES);
    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &scheduler, MESH_PORT);
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    mesh.onReceive(&onMeshReceive);
}

void loop() {
    mesh.update();
    Ethernet.maintain();
    if (!mqttClient.connected()) {
        if (millis() - lastReconnectAttempt >= MQTT_RECONNECT_INTERVAL_MS) {
            lastReconnectAttempt = millis(); mqttReconnect();
        }
    } else {
        mqttClient.loop(); flushQueue();
    }
    if (millis() - lastStatusPublish >= GATEWAY_STATUS_INTERVAL_MS) {
        lastStatusPublish = millis();
        if (mqttClient.connected()) publishGatewayStatus();
    }
}

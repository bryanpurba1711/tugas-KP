#!/usr/bin/env python3
"""Validate MQTT schema 1.0 and write motor-condition features to InfluxDB."""
import json
import logging
import math
import os
import sys
import time
from typing import Any

import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s")
log = logging.getLogger("mqtt_to_influx")
MQTT_HOST = os.getenv("MQTT_HOST", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_USERNAME = os.getenv("MQTT_USERNAME", "")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "")
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "kp/edge/+/features")
MQTT_STATUS_TOPIC = os.getenv("MQTT_STATUS_TOPIC", "kp/gateway/status")
INFLUX_URL = os.getenv("INFLUX_URL", "http://localhost:8086")
INFLUX_TOKEN = os.getenv("INFLUX_TOKEN", "")
INFLUX_ORG = os.getenv("INFLUX_ORG", "kp-org")
INFLUX_BUCKET = os.getenv("INFLUX_BUCKET", "motor_condition_monitoring")
last_sequence: dict[str, int] = {}

influx_client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

def finite(value: Any, default: float = 0.0) -> float:
    try:
        x = float(value)
        return x if math.isfinite(x) else default
    except (TypeError, ValueError):
        return default

def validate(payload: dict[str, Any]) -> tuple[str, int]:
    required = ("schema_version", "type", "node_id", "seq", "vibration", "temperature")
    missing = [key for key in required if key not in payload]
    if missing: raise ValueError(f"missing fields: {missing}")
    if payload["schema_version"] != "1.0" or payload["type"] != "condition_features":
        raise ValueError("unsupported schema/type")
    node_id = str(payload["node_id"])
    seq = int(payload["seq"])
    if not node_id or seq <= 0: raise ValueError("invalid node_id/seq")
    return node_id, seq

def vibration_point(node_id: str, axis: str, data: dict[str, Any], ts: int) -> Point:
    p = (Point("vibration_features").tag("node_id", node_id).tag("axis", axis)
         .field("acceleration_rms_g", finite(data.get("accel_rms_g")))
         .field("peak_g", finite(data.get("peak_g")))
         .field("crest_factor", finite(data.get("crest_factor")))
         .field("kurtosis", finite(data.get("kurtosis")))
         .field("dominant_freq_hz", finite(data.get("dominant_freq_hz")))
         .field("velocity_rms_mm_s", finite(data.get("velocity_rms_mm_s")))
         .time(ts, WritePrecision.NS))
    for i, value in enumerate(data.get("band_rms_g", [])):
        p = p.field(f"band_{i}_rms_g", finite(value))
    return p

def handle_features(payload: dict[str, Any]) -> None:
    node_id, seq = validate(payload)
    if seq <= last_sequence.get(node_id, -1):
        log.warning("duplicate/out-of-order node=%s seq=%s", node_id, seq)
        return
    ts = time.time_ns()
    points: list[Point] = []
    points.append(Point("node_status").tag("node_id", node_id)
                  .field("sequence", seq)
                  .field("battery_mv", int(payload.get("battery_mv", 0)))
                  .field("acquisition_valid", bool(payload.get("acquisition_valid", False)))
                  .field("sample_rate_hz", finite(payload.get("sample_rate_hz")))
                  .field("sample_count", int(payload.get("sample_count", 0)))
                  .field("alarm_state", str(payload.get("alarm_state", "UNKNOWN")))
                  .time(ts, WritePrecision.NS))
    temp = payload.get("temperature", {})
    points.append(Point("temperature_features").tag("node_id", node_id)
                  .field("valid", bool(temp.get("valid", False)))
                  .field("object_c", finite(temp.get("object_c")))
                  .field("ambient_c", finite(temp.get("ambient_c")))
                  .field("delta_c", finite(temp.get("delta_c")))
                  .time(ts, WritePrecision.NS))
    vibration = payload.get("vibration", {})
    for axis in ("x", "y", "z"):
        if axis in vibration: points.append(vibration_point(node_id, axis, vibration[axis], ts))
    write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=points)
    last_sequence[node_id] = seq
    log.info("wrote node=%s seq=%s points=%s", node_id, seq, len(points))

def handle_gateway(payload: dict[str, Any]) -> None:
    ts = time.time_ns()
    p = (Point("gateway_status").tag("gateway_id", str(payload.get("gateway_id", "gateway01")))
         .field("online", payload.get("status") == "online")
         .field("mesh_nodes", int(payload.get("mesh_nodes", 0)))
         .field("messages_relayed", int(payload.get("messages_relayed", 0)))
         .field("messages_queued_total", int(payload.get("messages_queued_total", 0)))
         .field("messages_rejected", int(payload.get("messages_rejected", 0)))
         .field("uptime_ms", finite(payload.get("uptime_ms")))
         .time(ts, WritePrecision.NS))
    write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=p)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        client.subscribe([(MQTT_TOPIC, 1), (MQTT_STATUS_TOPIC, 1)])
        log.info("connected and subscribed")
    else: log.error("MQTT connect rc=%s", rc)

def on_message(client, userdata, message):
    try:
        payload = json.loads(message.payload.decode("utf-8"))
        if message.topic == MQTT_STATUS_TOPIC: handle_gateway(payload)
        else: handle_features(payload)
    except Exception:
        log.exception("rejected message topic=%s", message.topic)

def main() -> None:
    if not INFLUX_TOKEN:
        log.error("INFLUX_TOKEN is required"); sys.exit(1)
    client = mqtt.Client(client_id="kp_mqtt_subscriber_v1")
    if MQTT_USERNAME: client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.on_connect = on_connect; client.on_message = on_message
    while True:
        try:
            client.connect(MQTT_HOST, MQTT_PORT, keepalive=60)
            client.loop_forever()
        except OSError as exc:
            log.error("MQTT unavailable: %s", exc); time.sleep(5)

if __name__ == "__main__": main()

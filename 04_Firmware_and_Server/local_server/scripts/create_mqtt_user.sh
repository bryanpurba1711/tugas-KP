#!/usr/bin/env bash
# =====================================================================
# Membuat/mengganti password_file Mosquitto untuk user MQTT.
# Jalankan sekali sebelum `docker compose up` pertama kali, atau setiap
# kali ingin mengganti password.
# =====================================================================
set -e
cd "$(dirname "$0")/.."

USERNAME="${1:-kp_mqtt_user}"

echo "Membuat kredensial MQTT untuk user: $USERNAME"
docker run --rm -it \
  -v "$(pwd)/mosquitto/config:/mosquitto/config" \
  eclipse-mosquitto:2 \
  mosquitto_passwd -c /mosquitto/config/password_file "$USERNAME"

echo "Selesai. Pastikan MQTT_USERNAME/MQTT_PASSWORD di .env dan firmware"
echo "gateway_esp32/include/config.h sesuai dengan kredensial yang baru dibuat."

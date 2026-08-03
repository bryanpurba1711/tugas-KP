# Local server stack

Mosquitto, InfluxDB 2.7, Python subscriber, dan Grafana. Salin `.env.example` menjadi `.env`, ganti semua kredensial, buat password Mosquitto, lalu jalankan `docker compose up -d --build`.

Measurements: `vibration_features`, `temperature_features`, `node_status`, dan `gateway_status`. Data digunakan untuk trending 4-jam; bukan sistem proteksi trip real-time.

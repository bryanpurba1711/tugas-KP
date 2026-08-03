# Gateway ESP32 + W5500

Gateway selalu aktif sebagai root painlessMesh dan bridge Ethernet/MQTT. Payload valid dipublish langsung atau dimasukkan ke antrean RAM terbatas. Node menerima ACK `published` atau `queued`; ketika antrean penuh, gateway tidak memberi ACK agar node melakukan retry.

Catatan: antrean RAM tidak menggantikan penyimpanan persisten. Untuk deployment industri, tambahkan FRAM/flash queue atau gateway Linux bila kebutuhan kehilangan data sangat ketat.

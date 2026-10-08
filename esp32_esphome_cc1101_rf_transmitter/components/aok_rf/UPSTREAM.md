Source: https://github.com/nrambaud/ESPHOME_A-OK
Revision: 321bde40ba2e7687a6d423a6701dc0069fd81ad3

Local change: make receiver_id optional. Without it, do not instantiate or
register an AOKReceiver. Reject on_aok without receiver_id. Encoder, timings,
command values and C++ files are unchanged from upstream.

Use aok_rf: {} for transmit-only operation. Copy this entire components/aok_rf
folder alongside the YAML when deploying through ESPHome Device Builder.

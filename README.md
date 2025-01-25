# Rotex / Daikin - HPSU Serial (Erweiterung)
**[Daikin-Rotex-HPSU-CAN-Serial](https://github.com/Trunks1982/Daikin-Rotex-HPSU-CAN-Seriell)** ist eine flexible Standalone-Lösung zum Auslesen und Steuern von Rotex/Daikin Luftwärmepumpen über den **CAN-Bus** und/oder die **serielle Schnittselle**.
Die Steuerung und der Datenzugriff erfolgen wahlweise über den integrierten Webserver oder bequem über Home Assistant bzw. ioBroker.
Mehrsprachensupport ermöglicht es, alle Sensoren flexibel in der gewünschten Sprache zu definieren.

Dieses Projekt erweitert die Möglichkeit, die Luftwärmepumpe seriell auszulesen. Das Auslesen der CAN-Daten sowie die Steuerung der Wärmepumpe sind hier beschrieben: [Daikin-Rotex-HPSU-CAN-Seriell](https://github.com/Trunks1982/Daikin-Rotex-HPSU-CAN-Seriell). Dort findest du detaillierte Anleitungen (Standalone oder in Kombination mit Home Assistant), um die CAN-Komponente mit ESPHome auf einen ESP32 zu flashen. Das Flashen der seriellen Komponente erfolgt auf ähnliche Weise, wobei die [YAML-Datei](https://github.com/wrfz/esphome-components/blob/main/examples/full_de.yaml) aus diesem Projekt verwendet wird.

Für die serielle Lösung genügt ein zusätzlicher ESP32. Weitere Module sind nicht erforderlich.

Grundsätzlich ist es auch möglich, beide Komponenten (CAN und seriell) auf einem einzigen ESP32 zu betreiben. Aus mehreren Gründen wird jedoch empfohlen, zwei separate ESP32-Geräte zu verwenden.

## Homeassistant - Dashboard
[![Bild 1](images/ha-dashboard-thumb.png)](images/ha-dashboard.png)

Das [HPSU Dashboard für Home Assistant](https://github.com/wrfz/daikin-rotex-hpsu-dashboard) ist ein Add-on, das hilft, die Funktionsweise der Wärmepumpe leicht verständlich in Echtzeit nachzuvollziehen.

[![Bild 1](images/ha-uart-sensors-small.png)](images/ha-uart-sensors.png)

Die seriellen Werte des Kältekreises verbessern das Verständnis der Pumpe und erleichtern es, im Falle von Störungen die Ursachen schneller zu erkennen.
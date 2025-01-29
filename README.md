# Rotex / Daikin - HPSU / Altherma - Serial (Erweiterung)
**[Daikin-Rotex-HPSU-CAN-Serial](https://github.com/Trunks1982/Daikin-Rotex-HPSU-CAN-Seriell)** ist eine flexible Standalone-Lösung zum Auslesen und Steuern von Rotex/Daikin Luftwärmepumpen über den **CAN-Bus** und/oder die **serielle Schnittselle**.
Die Steuerung und der Datenzugriff erfolgen wahlweise über den integrierten Webserver oder bequem über Home Assistant bzw. ioBroker.
Mehrsprachensupport ermöglicht es, alle Sensoren flexibel in der gewünschten Sprache zu definieren.

Dieses Projekt erweitert die Möglichkeit, die Luftwärmepumpe seriell auszulesen und damit zusätzlich Informationen zum Kältekreis zu erhalten. Das Auslesen der CAN-Daten sowie die Steuerung der Wärmepumpe sind hier beschrieben: [Daikin-Rotex-HPSU-CAN-Seriell](https://github.com/Trunks1982/Daikin-Rotex-HPSU-CAN-Seriell). Dort findest du detaillierte Anleitungen (Standalone oder in Kombination mit Home Assistant), um die CAN-Komponente mit ESPHome auf einen ESP32 zu flashen. Das Flashen der seriellen Komponente erfolgt auf ähnliche Weise, wobei die [YAML-Datei](https://github.com/wrfz/esphome-components/blob/main/examples/full_de.yaml) aus diesem Projekt verwendet wird.

## Homeassistant - Dashboard
[![Bild 1](images/ha-dashboard-thumb.png)](images/ha-dashboard.png)

Das [HPSU Dashboard für Home Assistant](https://github.com/wrfz/daikin-rotex-hpsu-dashboard) ist ein Add-on, das hilft, die Funktionsweise der Wärmepumpe leicht verständlich in Echtzeit nachzuvollziehen.

[![Bild 1](images/ha-uart-sensors-small.png)](images/ha-uart-sensors.png)

Die seriellen Werte des Kältekreises verbessern das Verständnis der Pumpe und erleichtern es, im Falle von Störungen die Ursachen schneller zu erkennen.

Für die serielle Lösung genügt ein zusätzlicher ESP32. Weitere Module sind nicht erforderlich.

Grundsätzlich ist es möglich, beide Komponenten (CAN und Seriell) auf einem einzigen ESP32 zu betreiben. Aus mehreren Gründen wird jedoch empfohlen, zwei separate ESP32-Geräte zu verwenden.

## Benötigte Hardware
Da die serielle Schnittstelle der HPSU mit 5V arbeitet, wird ein ESP32 empfohlen, der an seinen GPIO-Pins 5V verträgt.
Hier zwei geeignete Modelle:
- [ATOM S3 Lite](https://docs.m5stack.com/en/core/AtomS3%20Lite)
- [AtomS3U](https://docs.m5stack.com/en/core/AtomS3U)

Alternativ können auch andere ESP32-Boards (z. B. **ESP32-S3-WROOM**) verwendet werden.
In dem Fall ist allerdings ein zusätzlicher Pegelwandler erforderlich, um die 3,3V-Pegel des ESP32 auf die 5V der HPSU anzuheben.

## ESP anschließen

Wichtig! Vor den nächsten Schritten unbedingt das Innen- und Außengerät stromlos machen! ⚠️

Der ESP32 wird mit der `X10A`-Schnittstelle verbunden, die auf der A1P-Platine der Luftwärmepumpe zu finden ist (auf der Platine beschriftet). Die genaue Position und der Anschluss sind in der Rotex bzw. Daikin Wartungsanleitung ausführlich beschrieben.

- Bei HPSU-Geräten befindet sich die A1P-Platine hinter der `RoCon BM1` Hauptplatine (im `RoCon B1` Bedienelement). Diese muss vorher ausgebaut werden.
  Um an den `X10A`-Anschluss zu gelangen, die schwarze Kunststoffabdeckung links mit zwei Clips lösen und anheben.

Je nach Gerät kann die `X10A`-Schnittstelle `5` oder `8` Pins haben.

Für die Kommunikation reichen grundsätzlich die drei Pins `RX`, `TX` und `GND`, sofern der ESP über ein separates Netzteil mit Strom versorgt wird.
In einigen Fällen können bestimmte ESP-Modelle direkt über die 5V der `X10A` betrieben werden.

Beispiel: Die HPSU Compact 508 kann problemlos zwei `ATOM S3 Lite` sowie ein `CAN-Modul` mit Strom versorgen.

### 5 Pin X10A Schnittstelle
|    | X10A  | ESP32
| ---| ----- | ------
| 1  | 5V    | 5V - Spannungsversurgung für den ESP
| 2  | TX    | RX
| 3  | RX    | TX
| 4  | ---   | ---
| 5  | GND   | GND

### 8 Pin X10A Schnittstelle
|    | X10A  | ESP32
| ---| ----- | ------
| 1  | 5V    | 5V - Spannungsversurgung für den ESP
| 2  | ---   | ---
| 3  | RX    | TX
| 4  | TX    | RX
| 4  | --    | ---
| 5  | --    | ---
| 6  | --    | ---
| 7  | --    | ---
| 8  | GND   | GND


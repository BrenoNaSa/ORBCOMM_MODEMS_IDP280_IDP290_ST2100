#include "orbcomm_modem.h"
#include "arduino.h"
#include <vector>

#define UM_Segundo (1000)
#define UM_Minuto (UM_Segundo * 60)
#define UMA_Hora (UM_Minuto * 60)

OrbcommModem *orbcommModem = new OrbcommModem;

int index_message = 9;

uint8_t timestamp[] = {0xAA, 0xBB, 0x0A, 65, 0xBB, 0xCC, 0xFF, 0xDD};
uint8_t hdw_pic[] = {66, 66, 66, 66};
uint8_t mac_address[] = {67, 67, 67, 67, 67, 67};
uint8_t version[] = {68, 68, 68, 68};

uint8_t telemetry[] = {0xFF, 0xFF, 0xAA, 0xBB, 0x0A, 65, 0xBB, 0xCC, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 84, 0x45, 0x4c, 69, 0x4d, 69, 0x54, 82, 0x49, 65};
uint8_t data_lebgth[]{69, 69};

uint8_t checksum[] = {70, 70, 70, 70};

std::vector<uint8_t> concatenated_raw_payload;
std::vector<std::vector<uint8_t>> raw_payload = {std::vector<uint8_t>(timestamp, timestamp + sizeof(timestamp)),
                                                 std::vector<uint8_t>(hdw_pic, hdw_pic + sizeof(hdw_pic)),
                                                 std::vector<uint8_t>(mac_address, mac_address + sizeof(mac_address)),
                                                 std::vector<uint8_t>(version, version + sizeof(version)),
                                                 std::vector<uint8_t>(telemetry, telemetry + sizeof(telemetry)),
                                                 std::vector<uint8_t>(data_lebgth, data_lebgth + sizeof(data_lebgth)),
                                                 std::vector<uint8_t>(checksum, checksum + sizeof(checksum))};

void mount_payload(void);

unsigned long start_time_base;

void setup()
{
  mount_payload();
  orbcommModem->setSIN(128);
  orbcommModem->setMIN(3);

  start_time_base = millis();

  orbcommModem->orbcommInitialize();
}

void loop()
{
  if ((millis() - start_time_base) >= (900000)) // 900000 = 15 minutos
  {
    start_time_base = millis();
    sprintf(orbcommModem->buffer, "%d", index_message);
    orbcommModem->orbcommRoutine(string(orbcommModem->buffer), concatenated_raw_payload.data(), concatenated_raw_payload.size());

    // PortSerial.printf("[Loop] - LATITUDE: %f\n", orbcommModem->getLatitude());
    // PortSerial.printf("[Loop] - LONGITUDE: %f\n", orbcommModem->getLongitude());
    // PortSerial.printf("[Loop] - TIMESTAMP: %f\n", orbcommModem->getTimeStamp());
    PortSerial.printf("[Loop] - Conteudo MSG To-Mobile: %s\n", orbcommModem->getMessageContentToMobile().c_str());
  
  }
}

void mount_payload(void)
{
  for (const auto &var : raw_payload)
  {
    concatenated_raw_payload.insert(concatenated_raw_payload.end(), var.begin(), var.end());
  }
}
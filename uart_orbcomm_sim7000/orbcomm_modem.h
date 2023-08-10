/**
 * orbcomm_modem.h
 *
 *  Created on: 17 de apr de 2023
 *      Author: Breno Nascimento Santos
 *      e-mail: breno.afaq@gmail.com
 */
#pragma once

#include <string>
#include <string.h>
#include <stdint.h>
#include "arduino.h"

#include <ctime>   // Include the ctime header for struct tm and mktime
#include <sstream> // Include the sstream header for std::stringstream

#define TIMEOUT_DELETION_MSG 30000
#define TIMEOUT_MENSSAGE_STATE 60000
#define TIMEOUT_ROUTINE_INITIALIZE 60000

#define PWRKEY 4

#define BOOL_PRINTF(x) ((x) ? "TRUE" : "FALSE")
#define PIN_STATE_PRINTF(x) ((x) ? "HIGH" : "LOW")

#define ORBCOMM_ERROR "ERROR"
#define ORBCOMM_OK "OK"
#define TIMEOUT_COMMAND 5000

#define PortSerial (Serial)
#define BAUDRATE (9600)
#define PIN_SERIAL_RX (26)
#define PIN_SERIAL_TX (27)
#define UART_PROTOCOL_REGISTER_USED_IN_ESP32_MODEL (0x800001c) // SERIAL_8N1
#define SERIAL_REGISTER (UART_PROTOCOL_REGISTER_USED_IN_ESP32_MODEL)

using namespace std;

enum PowerMode
{
  mobile_powered = 0,
  fixed_position_powered = 1,
  mobile_battery_operated = 2,
  fixed_position_battery_operated = 3,
  mobile_minimal_battery_operated = 4,
  mobile_parked_stationary = 5,
};

enum SleepSchedule
{
  five_seconds = 0,
  thirty_seconds = 1,
  one_minute = 2,
  three_minutes = 3,
  ten_minutes = 4,
  thirty_minutes = 5,
  sixty_minutes = 6,
  two_minutes = 7,
  five_minutes = 8,
  fifteen_minutes = 9,
  twenty_minutes = 10,
};

class OrbcommModem
{
public:
  OrbcommModem(void); // Construtor da classe

  char buffer[2];
  PowerMode powerMode;
  SleepSchedule sleepSchedule;

  void setSIN(uint8_t value);                                                                                  // Esse metodo faz SET no valor do parâmetro SIM
  void setMIN(uint8_t value);                                                                                  // Esse metodo faz SET no valor do parâmetro MIN
  uint8_t getSIN(void);                                                                                        // Esse metodo faz GET no valor do parâmetro SIM
  uint8_t getMIN(void);                                                                                        // Esse metodo faz GET no valor do parâmetro MIN
  float getLatitude(void);                                                                                     // Esse metodo faz GET no valor do parâmetro Latitude
  float getLongitude(void);                                                                                    // Esse metodo faz GET no valor do parâmetro Longitude
  double getTimeStamp(void);                                                                                   // Esse metodo faz GET no valor do parâmetro TimeStamp
  string getMessageContentToMobile(void);                                                                      // Esse metodo faz GET no valor do parâmetro Conteudo da mensagem To-Mobile
  void serialInitialize(uint32_t baud_rate, uint32_t serial_register, uint8_t rx_pin, uint8_t tx_pin);         // Essa função inicializa a serial utilizada pela modem da ORBCOMM
  bool checkSerialConnection(void);                                                                            // Essa função verifica o status da comunicação serial
  string executeAtCommand(string command, string result_expected, string error_expected, int command_timeout); // Essa função executa qualquer dos comanados ATs da ORBCOMM
  bool disableSim7000Serial(uint8_t pin_number, uint8_t mode_operation_pin, uint8_t pin_state);                // Essa função tem como objetivo exclusivo desabilitar uso da serial pela SIM7000.
  bool setPowerMode(string enum_mode);                                                                         // Essa função executa o comando que definir modo de energia.
  bool setSleepSchedule(string enum_tempo);                                                                    // Essa função executa o comando que definir periodo de ativacao para um modem de modo de baixo consumo de energia.
  bool orbcommInitialize(void);                                                                                // Essa função serve para inicializar o serviços ORBCOMM
  bool orbcommRoutine(string index_message, const uint8_t *raw_payload, size_t raw_payload_length);            // Essa função executa a rotinas dos serviços ORBCOMM

private:
  uint8_t SIN;
  uint8_t MIN;
  float latitude;
  float longitude;
  double timestamp;
  string message_content_ToMobile;

  bool deleteQueueMessageFromMobile(string index_message);                                       // Essa função deleta uma mensagem especifica da fila de mensagem.
  bool isConnectionAvailable(void);                                                              // Essa função verifica se a conexão está disponível chamando a função do comando AT.
  bool isSuccessfulMessageState(void);                                                           // Essa função verifica se o status de envio da mensagem foi bem sucedido.
  bool deleteMessagesOfToMobileQueue(string name_message);                                       // Essa função executa o comando que excluir mensagens da fila to-mobile.
  bool sendQueueForMessage(string index_message, string raw_payload, string sin, string min);    // Essa função executa o comando que enviar uma mensagem para fila mensagem From-Mobile.
  bool routineSendingMessageFromMobile(string index_message, string raw_payload_encoded_data);   // Essa função executa o a rotina de envio de uma mensagem para fila mensagem From-Mobile.
  bool routineDeletingMessageFromMobile(string index_message);                                   // Essa função executa uma rotina para deletar uma mensagem da fila From-Mobile.
  bool receivingRoutineMessageToMobile(void);                                                    // Essa função executa uma rotina para receber uma mensagem da fila To-Mobile.
  bool checkQueueMessagesFromMobile(void);                                                       // Essa função verifica a fila de mensagens From-Mobile.
  bool getGPS(void);                                                                             // Essa função executa o comando que retorna GPS.
  bool getDateTime(void);                                                                        // Essa função devera retorna o valor do timestamp ao inves da data e hora UTC.
  bool getContentOfSpecificMessagesOfQueueToMobile(string message_name);                         // Essa função executa o comando e retorna o centeudo da mensagem em string.
  string getQueueOfMessagesToMobile(void);                                                       // Essa função executa o comando que retorna as mensagem da fila To-Mobile.
  string getLastErrorCode(void);                                                                 // Essa função executa o comando que retorna o ultimo codigo de erro.
  string getStatusAntennaJamming(void);                                                          // Essa função executa o comando que deve obter status de interferência de antena.
  string encodeBinaryPacket(const uint8_t *raw_payload, size_t raw_payload_length);              // Essa função tem como objetivo montar cabeçalho do protocolo da aplicação que sera incluido na mensagem.
  string decimalArrayToHexadecimalString(const uint8_t *decimal_array, size_t length);           // Essa função converte um valor decimal ou hexadecimal para string.
  double unixTimestampConverter(string date, string time);                                       // Essa função retorna o timestamp em Unix-Timestamp
  uint16_t getUint16TimeFragment(string date_and_time, uint8_t substr_pos, uint8_t substr_size); // Essa função retorna os fragmento do timestamp em uint16_t
  string removeCharsAndSlashes(const string &input);                                             // Essa função remove alguns caracteres e os separadores de hexadecimais da string de entrada e retorna outra string.
};

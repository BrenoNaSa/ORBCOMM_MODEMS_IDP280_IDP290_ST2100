/**
 * orbcomm_modem.cpp
 *
 *  Created on: 17 de apr de 2023
 *      Author: Breno Nascimento Santos
 *      e-mail: breno.afaq@gmail.com
 */

#include "orbcomm_modem.h"

OrbcommModem::OrbcommModem(void) // Implementação do construtor da classe
{
  SIN = 128;
  MIN = 0;
  serialInitialize(BAUDRATE, SERIAL_REGISTER, PIN_SERIAL_RX, PIN_SERIAL_TX);

  bool isolated_serial_orbcommserial = disableSim7000Serial(PWRKEY, OUTPUT, HIGH);
  PortSerial.printf("ORBCOMM serial isolado BOOL: %s\n", BOOL_PRINTF(isolated_serial_orbcommserial));
  PortSerial.printf("ORBCOMM serial isolado INTEGER: %d\n", int(isolated_serial_orbcommserial));

  isConnectionAvailable(); // Essa função verifica se a conexão está disponível chamando a função do comando AT.
}

void OrbcommModem::setSIN(uint8_t value)
{
  SIN = value;
}

void OrbcommModem::setMIN(uint8_t value)
{
  MIN = value;
}

uint8_t OrbcommModem::getSIN(void)
{
  return SIN;
}

uint8_t OrbcommModem::getMIN(void)
{
  return MIN;
}

float OrbcommModem::getLatitude(void)
{
  return latitude;
}

float OrbcommModem::getLongitude(void)
{
  return longitude;
}

double OrbcommModem::getTimeStamp(void)
{
  return timestamp;
}

string OrbcommModem::getMessageContentToMobile(void)
{
  return removeCharsAndSlashes(message_content_ToMobile);
}

void OrbcommModem::serialInitialize(uint32_t baud_rate, uint32_t serial_register, uint8_t rx_pin, uint8_t tx_pin)
{
  PortSerial.begin(baud_rate, serial_register, rx_pin, tx_pin);
  powerMode = mobile_powered;
  // sleepSchedule = sixty_minutes;
  sleepSchedule = five_seconds;
}

bool OrbcommModem::checkSerialConnection(void)
{
  bool status_connection;
  if (PortSerial.available())
    status_connection = true;
  else
    status_connection = false;
  return status_connection;
}

string OrbcommModem::executeAtCommand(string command, string result_expected, string error_expected, int command_timeout = 5000)
{
  PortSerial.println(command.c_str());
  string result = "";

  while (checkSerialConnection())
    PortSerial.readString();

  unsigned long last_serial_read_instant = millis();
  while (millis() - last_serial_read_instant < command_timeout)
  {
    if (checkSerialConnection())
    {
      result = PortSerial.readString().c_str(); // #FIXME testar de forma cumulativa serializada
      PortSerial.println(result.c_str());
      last_serial_read_instant = millis();
      if (result.find(error_expected) != string::npos)
        return result;
      else if (result.find(result_expected) != string::npos)
        return result;
    }
  }
  return result;
}

bool OrbcommModem::disableSim7000Serial(uint8_t pin_number, uint8_t mode_operation_pin, uint8_t pin_state)
{
  bool result = false;
  uint32_t time = 0;

  if (result == false)
  {
    /** Define o numero do pino e seu modo de operação */
    pinMode(pin_number, mode_operation_pin);
    PortSerial.println("Iniciando rorina de desabilitação da serial da SIM7000...");

    PortSerial.println("DESLIGANDO LED.");
    executeAtCommand("AT+CNETLIGHT=0", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);

    PortSerial.printf("Pino power: %d\n", pin_number);

    if (mode_operation_pin == OUTPUT)
    {
      PortSerial.println("Modo de operação output.");
    }
    else
    {
      PortSerial.println("Modo de operação input.");
    }

    /** 1º Passo - pino fica em nivel logico baixo por 1.2 segundos e volta para nivel logico alto */
    PortSerial.println("1º Passo - pino fica em nivel logico baixo por 1.2 segundos e volta para nivel logico alto");
    pinMode(pin_number, OUTPUT);
    digitalWrite(pin_number, LOW);
    PortSerial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    PortSerial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    time = 1200;
    delay(1200);
    PortSerial.printf("Duracao: %d\n", time);

    digitalWrite(pin_number, HIGH);
    PortSerial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    PortSerial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    time = 5000;
    delay(5000);
    PortSerial.printf("Duracao: %d\n", time);

    /** 2º Passo - pino fica em nivel logico baixo por 1.4 segundos e volta para nivel logico alto */
    PortSerial.println("2º Passo - pino fica em nivel logico baixo por 1.4 segundos e volta para nivel logico alto");
    pinMode(pin_number, OUTPUT);
    digitalWrite(pin_number, LOW);
    PortSerial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    PortSerial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    time = 1400;
    delay(1400);
    PortSerial.printf("Duracao: %d\n", time);

    digitalWrite(pin_number, HIGH);
    PortSerial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    PortSerial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    if (digitalRead(pin_number) == HIGH)
    {
      PortSerial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
      PortSerial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));
      PortSerial.println("FIM rorina de desabilitação da serial da SIM7000...");
      result = true;
    }
    else
    {
      PortSerial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
      PortSerial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));
      result = false;
    }
  }
  return result;
}

bool OrbcommModem::setPowerMode(string enum_mode)
{
  string result = executeAtCommand("ATS50=" + enum_mode, ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  if (result.find("OK") != string::npos)
  {
    PortSerial.println("Configurado set power mode com sucesso!");
    return true;
  }
  else if (result.find("ERROR") != string::npos)
  {
    PortSerial.println("Error em configurado set power mode");
    return false;
  }
  else
  {
    PortSerial.println("[setPowerMode] - Problema na comunicacao!");
    return false;
  }
}

bool OrbcommModem::setSleepSchedule(string enum_tempo)
{
  string result = executeAtCommand("ATS51=" + enum_tempo, ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  if (result.find("OK") != string::npos)
  {
    PortSerial.println("Configurado set sleep schedule com sucesso!");
    return true;
  }
  else if (result.find("ERROR") != string::npos)
  {
    PortSerial.println("Error em configurado set sleep schedule");
    return false;
  }
  else
  {
    PortSerial.println("[setSleepSchedule] - Problema na comunicacao!");
    return false;
  }
}

bool OrbcommModem::orbcommInitialize(void)
{
  bool sleep_schedule_status = false;
  bool power_mode_status = false;
  bool gps_status = false;
  bool timestamp_status = false;

  uint64_t last_count_time = millis();
  while ((millis() - last_count_time) < TIMEOUT_ROUTINE_INITIALIZE)
  {
    if ((!power_mode_status))
    {
      // 1º Passo - Definir o modo de baixo consumo de energia.
      sprintf(buffer, "%d", powerMode);
      power_mode_status = setPowerMode(string(buffer));
      delay(500);
    }
    else if ((power_mode_status) && (!sleep_schedule_status))
    {
      // 2º Passo - Definir periodo de ativacao para um modem.
      sprintf(buffer, "%d", sleepSchedule);
      sleep_schedule_status = setSleepSchedule(string(buffer));
      delay(500);
    }
    else if ((power_mode_status) && (sleep_schedule_status) && (!timestamp_status))
    {
      // 3º Passo - Coleta dados de TimeStamp.
      timestamp_status = getDateTime();
      delay(500);
    }
    else if ((power_mode_status) && (sleep_schedule_status) && (timestamp_status) && (!gps_status))
    {
      // 4º Passo - Coleta dados de GPS.
      delay(500);
      gps_status = getGPS();
    }
    else if ((power_mode_status) && (sleep_schedule_status) && (timestamp_status) && (gps_status))
    {
      PortSerial.println("Inicializacao do servicos ORBCOMM executada com sucesso!!!");
      return true;
    }
    else
    {
      PortSerial.println("ERRRO na inicializacao do servicos ORBCOMM!!!");
      return false;
    }
  }
}

bool OrbcommModem::orbcommRoutine(string index_message, const uint8_t *raw_payload, size_t raw_payload_length)
{
  PortSerial.println("*********************");
  PortSerial.println(" INICIO DA ROTINA!!! ");
  PortSerial.println("*********************");

  /**************************************************************************************/
  /**************************************************************************************/
  /**************************************************************************************/

  uint64_t last_count_time = millis();
  while ((millis() - last_count_time) < TIMEOUT_MENSSAGE_STATE)
  {
    bool receiving_state = receivingRoutineMessageToMobile();
    if (receiving_state == true)
    {
      break;
    }
  }

  /**************************************************************************************/
  /**************************************************************************************/
  /**************************************************************************************/

  string raw_payload_encoded_data = encodeBinaryPacket(raw_payload, raw_payload_length);
  bool rotine_state = routineSendingMessageFromMobile(index_message, raw_payload_encoded_data);

  if (rotine_state == true)
  {
    bool message_state = isSuccessfulMessageState();
    if ((message_state == false))
    {
      uint64_t last_count_time = millis();
      while ((millis() - last_count_time) < TIMEOUT_MENSSAGE_STATE)
      {
        message_state = isSuccessfulMessageState();
        if (message_state == true)
        {
          PortSerial.println("*********************");
          PortSerial.println("  FIM DA ROTINA!!!  ");
          PortSerial.println("*********************");
          return true;
        }
      }
      PortSerial.printf("Tempo em segundos tentado verificar o state da mensagem da fila From-Mobile: %d\n", (millis() - last_count_time) / 1000);
      PortSerial.println("Timout na execucao da rotina de verificar o state da mensagem da fila From-Mobile!");
      PortSerial.println("Error na rotina de verificar o state da mensagem da fila From-Mobile!");
    }
    return true;
  }
  else
  {
    return false;
  }
}

bool OrbcommModem::deleteQueueMessageFromMobile(string index_message)
{
  string result = executeAtCommand("AT%MGRC=\"ZeusMsg" + index_message + "\"", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  if (result.find("OK") != string::npos)
  {
    PortSerial.println("Deletado mensagem com sucesso!");
    return true;
  }
  else if (result.find("ERROR") != string::npos)
  {
    PortSerial.println("Error em deletar mensagem!");
    return false;
  }
  else
  {
    PortSerial.println("[deleteQueueMessageFromMobile] - Problema na comunicacao!");
    return false;
  }
}

bool OrbcommModem::isConnectionAvailable(void)
{
  string result = executeAtCommand("AT", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);

  if (result.find("OK") != string::npos)
    return true;
  else
    return false;
}

bool OrbcommModem::isSuccessfulMessageState(void)
{
  string result = executeAtCommand("AT%MGRS", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  char *c_result;
  c_result = &result[0];
  if ((result.find("%MGRS") != string::npos) && (result.find("OK") != string::npos) && (result.length() > 26))
  {
    char *new_result;
    new_result = strtok(c_result, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");

    if (new_result[0] == '6')
    {
      PortSerial.print("Mensagem State: ");
      PortSerial.println(new_result);
      return true;
    }
    else
    {
      PortSerial.print("Mensagem State: ");
      PortSerial.println(new_result);
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool OrbcommModem::deleteMessagesOfToMobileQueue(string name_message)
{
  string result = executeAtCommand("AT%MGFM=\"" + name_message + "\"", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  if (result.find("OK") != string::npos)
    return true;
  else
    return false;
}

bool OrbcommModem::sendQueueForMessage(string index_message, string raw_payload, string sin, string min)
{
  string result = executeAtCommand("AT%MGRT=\"ZeusMsg" + index_message + "\",2," + sin + "." + min + ",1,\"" + raw_payload + "\"", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  if (result.find("OK") != string::npos)
    return true;
  else
    return false;
}

bool OrbcommModem::routineSendingMessageFromMobile(string index_message, string raw_payload_encoded_data)
{
  bool is_ready_to_send_message = false;
  uint8_t sin_u8 = getSIN();
  uint8_t min_u8 = getMIN();

  sprintf(buffer, "%d", sin_u8);
  string sin = string(buffer);
  sprintf(buffer, "%d", min_u8);
  string min = string(buffer);

  uint8_t queue_state = checkQueueMessagesFromMobile();
  if (queue_state == true)
  {
    bool send_message_queue_state = sendQueueForMessage(index_message, raw_payload_encoded_data, sin, min);
    if (send_message_queue_state == true)
    {
      PortSerial.println("Fila vazia uma mensagem foi adicionada na fila From-Mobile com sucesso!");
      return true;
    }
    else
    {
      PortSerial.println("Fila vazia ERRO em adicionar uma mensagem na fila From-Mobile");
      return false;
    }
  }
  else
  {
    is_ready_to_send_message = routineDeletingMessageFromMobile(index_message);
    if (is_ready_to_send_message == true)
    {
      sendQueueForMessage(index_message, raw_payload_encoded_data, sin, min);
      PortSerial.println("Adicionada de mensagem na fila From-Mobile apos a rotina de delecao de mensagem da fila From-Mobile!");
      return true;
    }
    else
    {
      PortSerial.println("Error na rotina de envio de mensagem da fila From-Mobile!");
      return false;
    }
  }
}

bool OrbcommModem::routineDeletingMessageFromMobile(string index_message)
{
  bool status_delete_queue_message = deleteQueueMessageFromMobile(index_message);
  PortSerial.println("Fila cheia executando rotina de delecao de mensagem fila From-Mobile...");

  if (status_delete_queue_message == true)
  {
    PortSerial.println("Fila vazia terminado rotina de delecao de mensagem da fila From-Mobile!");
    return true;
  }
  else
  {
    uint64_t last_count_time = millis();
    while ((millis() - last_count_time) < TIMEOUT_DELETION_MSG)
    {
      status_delete_queue_message = deleteQueueMessageFromMobile(index_message);
    }
    PortSerial.printf("Tempo em segundos tentado deletar a mensagem da fila From-Mobile: %d\n", (millis() - last_count_time) / 1000);
    PortSerial.println("Timout na execucao da rotina de delecao de mensagem da fila From-Mobile!");
    PortSerial.println("Error na rotina de deleção de mensagem da fila From-Mobile!");

    return false;
  }
}

bool OrbcommModem::receivingRoutineMessageToMobile(void)
{
  string message_name = getQueueOfMessagesToMobile();

  bool message_content_state = getContentOfSpecificMessagesOfQueueToMobile(message_name);
  if (message_content_state == true)
  {
    return true;
  }
}

bool OrbcommModem::checkQueueMessagesFromMobile(void)
{
  string result = executeAtCommand("AT%MGRS", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  char *c_result;
  c_result = &result[0];
  if ((result.find("%MGRS:") != string::npos) && (result.find("OK") != string::npos) && (result.length() == 17))
  {
    PortSerial.println("Fila Vazia!");
    return true;
  }
  else if ((result.find("%MGRS:") != string::npos) && (result.find("OK") != string::npos) && (result.length() > 17))
  {
    PortSerial.println("Fila Cheia!");
    return false;
  }
  else if (result.find("ERROR") != string::npos)
  {
    PortSerial.println("Error no comando!");
    return false;
  }
}

bool OrbcommModem::getGPS(void)
{
  string result = executeAtCommand("AT%GPS=15,1,\"RMC\"", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  char *c_result = &result[0];
  if ((result.find("%GPS:") != string::npos) && (result.find("OK") != string::npos) && (result.length() > 26))
  {
    char *new_result;
    new_result = strtok(c_result, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");

    float latitude_local = atof(new_result);
    new_result = strtok(NULL, ",");
    if (new_result[0] == 'S') // Direção da Latitude (N ou S)
    {
      latitude_local = -latitude_local;
    }
    new_result = strtok(NULL, ",");
    float longitude_local = atof(new_result);
    new_result = strtok(NULL, ",");
    if (new_result[0] == 'W') // Direção da Longitude (E ou W)
    {
      longitude_local = -longitude_local;
    }

    // Converter para decimal corretamente
    int degrees_latitude = int(latitude_local / 100.0);
    float minutes_latitude = latitude_local - degrees_latitude * 100.0;
    float latitude_decimal = degrees_latitude + minutes_latitude / 60.0;
    latitude = latitude_decimal;

    int degrees_longitude = int(longitude_local / 100.0);
    float minutes_longitude = longitude_local - degrees_longitude * 100.0;
    float longitude_decimal = degrees_longitude + minutes_longitude / 60.0;
    longitude = longitude_decimal;

    PortSerial.print("LATITUDE: ");
    PortSerial.println(latitude, 7); // Use 7 casas decimais para maior precisão
    PortSerial.print("LONGITUDE: ");
    PortSerial.println(longitude, 7); // Use 7 casas decimais para maior precisão

    return true;
  }
  else
  {
    return false;
  }
}

bool OrbcommModem::getDateTime(void)
{
  string result = executeAtCommand("AT%UTC", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);

  size_t utc_pos = result.find("%UTC: ");
  if (utc_pos == string::npos)
  {
    return false;
  }

  string date_and_time = result.substr(utc_pos + 6);

  string date = date_and_time.substr(0, 10); //(e.g., "2023-08-02")
  string time = date_and_time.substr(11, 8); //(e.g., "12:54:46")

  PortSerial.print("Data: ");
  PortSerial.println(date.c_str());

  PortSerial.print("Hora: ");
  PortSerial.println(time.c_str());

  timestamp = unixTimestampConverter(date, time);

  return true;
}

string OrbcommModem::getQueueOfMessagesToMobile(void)
{
  string result = executeAtCommand("AT%MGFN", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  char *c_result;
  c_result = &result[0];
  if ((result.find("%MGFN") != string::npos) && (result.find("OK") != string::npos) && (result.length() > 17))
  {
    char *new_result;
    new_result = strtok(c_result, ":");
    new_result = strtok(NULL, ",");
    PortSerial.println("*****************************************************");
    PortSerial.print("Nome da mensagem da fila To-Mobile: ");
    PortSerial.println(new_result);
    PortSerial.println("*****************************************************");
    return new_result;
  }
  else if ((result.find("%MGFN") != string::npos) && (result.find("OK") != string::npos) && (result.length() <= 17))
  {
    PortSerial.println("[getQueueOfMessagesToMobile] - Não existe mensagem na fila To-Mobile");
    return result;
  }
  else
  {
    PortSerial.println("[getQueueOfMessagesToMobile] - Erro na execução do comando AT%MGFN");
    return result;
  }
}

bool OrbcommModem::getContentOfSpecificMessagesOfQueueToMobile(string message_name)
{
  string result = executeAtCommand("AT%MGFG=" + message_name + ",1", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  char *c_result;
  c_result = &result[0];
  if ((result.find("%MGFG") != string::npos) && (result.find("OK") != string::npos) && (result.length() > 9))
  {
    char *new_result;
    new_result = strtok(c_result, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    new_result = strtok(NULL, ",");
    PortSerial.println("*****************************************************");
    PortSerial.print("Conteudo da mensagem da fila To-Mobile: ");
    PortSerial.println(new_result);
    message_content_ToMobile = new_result;
    PortSerial.println("*****************************************************");
    return true;
  }
  else if ((result.find("%MGFG") != string::npos) && (result.find("OK") != string::npos) && (result.length() <= 9))
  {
    PortSerial.println("[getContentOfSpecificMessagesOfQueueToMobile] - Não existe mensagem na fila To-Mobile");
    return false;
  }
  else
  {
    PortSerial.println("[getContentOfSpecificMessagesOfQueueToMobile] - Erro na execução do comando AT%MGFG=NomeMensagem");
    return false;
  }
}

string OrbcommModem::getLastErrorCode(void)
{
  string result = executeAtCommand("ATS80?", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  return result;
}

string OrbcommModem::getStatusAntennaJamming(void)
{
  string result = executeAtCommand("ATS56?", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  return result;
}

string OrbcommModem::encodeBinaryPacket(const uint8_t *raw_payload, size_t raw_payload_length)
{
  string encode_data = decimalArrayToHexadecimalString(raw_payload, raw_payload_length);
  // PortSerial.printf("MSG: %s\n", encode_data.c_str());

  return encode_data;
}

string OrbcommModem::decimalArrayToHexadecimalString(const uint8_t *decimal_array, size_t length)
{
  string hexadecimal_string = "";

  for (size_t i = 0; i < length; i++)
  {
    char hexadecimal_chars[3]; // Armazena 2 dígitos hexadecimais (incluindo '\0')
    sprintf(hexadecimal_chars, "%02X", decimal_array[i]);
    hexadecimal_string += hexadecimal_chars;
  }

  return hexadecimal_string;
}

double OrbcommModem::unixTimestampConverter(string date, string time)
{
  uint16_t years = getUint16TimeFragment(date, 0, 4);
  uint16_t months = getUint16TimeFragment(date, 5, 2);
  uint16_t days = getUint16TimeFragment(date, 8, 2);
  uint16_t hours = getUint16TimeFragment(time, 0, 2);
  uint16_t minutes = getUint16TimeFragment(time, 3, 2);
  uint16_t seconds = getUint16TimeFragment(time, 6, 2);

  struct tm tm;
  tm.tm_year = years - 1900;
  tm.tm_mon = months - 1;
  tm.tm_mday = days;
  tm.tm_hour = hours;
  tm.tm_min = minutes;
  tm.tm_sec = seconds;
  double total_seconds = mktime(&tm);

  PortSerial.print("TimeStamp:");
  PortSerial.println(total_seconds);

  return total_seconds;
}

uint16_t OrbcommModem::getUint16TimeFragment(string date_and_time, uint8_t substr_pos, uint8_t substr_size)
{
  string str_time_fragment = date_and_time.substr(substr_pos, substr_size);
  std::stringstream ss(str_time_fragment);
  uint16_t time_fragment_int;
  ss >> time_fragment_int;
  return time_fragment_int;
}

string OrbcommModem::removeCharsAndSlashes(const string &input)
{
  string output = "";
  for (size_t i = 0; i < input.length(); ++i)
  {
    if (i + 1 < input.length() && input[i] == '\\' && input[i + 1] == '0')
    {
      i += 1; // Skip the next character
      continue;
    }
    if (input[i] != '\\')
    {
      output += input[i];
    }
  }
  return output;
}

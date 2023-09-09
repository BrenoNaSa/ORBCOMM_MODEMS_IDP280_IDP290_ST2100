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
  Serial.printf("ORBCOMM serial isolado BOOL: %s\n", BOOL_PRINTF(isolated_serial_orbcommserial));
  Serial.printf("ORBCOMM serial isolado INTEGER: %d\n", int(isolated_serial_orbcommserial));

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

vector<uint8_t> OrbcommModem::getMessageContentToMobile(void)
{
  string msg_content_ToMobile = removeCharsAndSlashes(message_content_ToMobile);
  return hexStringToBytes(msg_content_ToMobile);
}

void OrbcommModem::serialInitialize(uint32_t baud_rate, uint32_t serial_register, uint8_t rx_pin, uint8_t tx_pin)
{
  PortSerial.begin(baud_rate, serial_register, rx_pin, tx_pin);
  powerMode = mobile_powered;
  // sleepSchedule = sixty_minutes;
  sleepSchedule = five_seconds;
  shippingTimePeriod = shipping_period_sixty_minutes;
}

bool OrbcommModem::disableSim7000Serial(uint8_t pin_number, uint8_t mode_operation_pin, uint8_t pin_state)
{
  bool result = false;
  uint32_t time_ms = 0;

  if (result == false)
  {
    /** Define o numero do pino e seu modo de operação */
    pinMode(pin_number, mode_operation_pin);
    Serial.println("Iniciando rorina de desabilitacao da serial da SIM7000...");

    Serial.printf("Pino power: %d\n", pin_number);

    if (mode_operation_pin == OUTPUT)
    {
      Serial.println("Modo de operacao output.");
    }
    else
    {
      Serial.println("Modo de operacao input.");
    }

    /** 1º Passo - pino fica em nivel logico baixo por 1.2 segundos e volta para nivel logico alto */
    Serial.println("1º Passo - pino fica em nivel logico baixo por 1.2 segundos e volta para nivel logico alto");
    pinMode(pin_number, OUTPUT);
    digitalWrite(pin_number, LOW);
    Serial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    Serial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    time_ms = 1200;
    delay(time_ms);
    Serial.printf("Duracao: %d\n", time_ms);

    digitalWrite(pin_number, HIGH);
    Serial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    Serial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    time_ms = 5000;
    delay(time_ms);
    Serial.printf("Duracao: %d\n", time_ms);

    /** 2º Passo - pino fica em nivel logico baixo por 1.4 segundos e volta para nivel logico alto */
    Serial.println("2º Passo - pino fica em nivel logico baixo por 1.4 segundos e volta para nivel logico alto");
    pinMode(pin_number, OUTPUT);
    digitalWrite(pin_number, LOW);
    Serial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    Serial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    time_ms = 1400;
    delay(time_ms);
    Serial.printf("Duracao: %d\n", time_ms);

    digitalWrite(pin_number, HIGH);
    Serial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
    Serial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));

    if (digitalRead(pin_number) == HIGH)
    {
      Serial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
      Serial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));
      Serial.println("FIM rorina de desabilitação da serial da SIM7000...");
      result = true;
    }
    else
    {
      Serial.printf("Pino state BOOL: %s\n", PIN_STATE_PRINTF(digitalRead(pin_number)));
      Serial.printf("Pino state INTEGER: %d\n", digitalRead(pin_number));
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
    Serial.println("Configurado set power mode com sucesso!");
    return true;
  }
  else if (result.find("ERROR") != string::npos)
  {
    Serial.println("Error em configurado set power mode");
    return false;
  }
  else
  {
    Serial.println("[setPowerMode] - Problema na comunicacao!");
    return false;
  }
}

bool OrbcommModem::setSleepSchedule(string enum_tempo)
{
  string result = executeAtCommand("ATS51=" + enum_tempo, ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  if (result.find("OK") != string::npos)
  {
    Serial.println("Configurado set sleep schedule com sucesso!");
    return true;
  }
  else if (result.find("ERROR") != string::npos)
  {
    Serial.println("Error em configurado set sleep schedule");
    return false;
  }
  else
  {
    Serial.println("[setSleepSchedule] - Problema na comunicacao!");
    return false;
  }
}

uint8_t OrbcommModem::getPeriodSize(vector<uint8_t> message_content_to_mobile)
{

  if (message_content_to_mobile.size() >= 3)
  {
    uint8_t byte1 = message_content_to_mobile[1];
    uint8_t byte2 = message_content_to_mobile[2];

    Serial.print("byte1: ");
    Serial.println(byte1, HEX); // Imprime byte1 em formato hexadecimal
    Serial.print("byte2: ");
    Serial.println(byte2, HEX); // Imprime byte2 em formato hexadecimal
    if (byte1 == PERIOD_CHANGE_MESSAGE_TYPE)
    {
      Serial.println("Tipo mensagem PERIODSIZE!");
      if (byte2 == SHIPPING_PERIOD_FIFTEEN_MINUTES)
      {
        Serial.println("Tempo de envio de mensagens From-Mobile sera de 15 minutios");
        shippingTimePeriod = shipping_period_fifteen_minutes;
        return shippingTimePeriod;
      }
      else if (byte2 == SHIPPING_PERIOD_SIXTY_MINUTES)
      {
        Serial.println("Tempo de envio de mensagens From-Mobile sera de 60 minutios");
        shippingTimePeriod = shipping_period_sixty_minutes;
        return shippingTimePeriod;
      }
      else
      {
        Serial.println("Tempo de envio de mensagens From-Mobile (RESERVADO) Ainda não implementado!");
        shippingTimePeriod = shipping_period_zero_minutes;
        return shippingTimePeriod;
      }
    }
    else
    {
      Serial.println("Tipo mensagem RESERVADO!");
      shippingTimePeriod = shipping_period_zero_minutes;
      return shippingTimePeriod;
    }
  }
  else
  {
    Serial.println("Não há dados suficientes no vetor.");
    shippingTimePeriod = shipping_period_zero_minutes;
    return shippingTimePeriod;
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
      Serial.println("Inicializacao do servicos ORBCOMM executada com sucesso!!!");
      return true;
    }
    else
    {
      Serial.println("ERRRO na inicializacao do servicos ORBCOMM!!!");
      return false;
    }
  }
}

bool OrbcommModem::orbcommRoutineSendMessage(string index_message, const uint8_t *raw_payload, size_t raw_payload_length)
{
  Serial.println("*********************");
  Serial.println(" INICIO DA ROTINA!!! ");
  Serial.println("*********************");

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
          Serial.println("*********************");
          Serial.println("  FIM DA ROTINA!!!  ");
          Serial.println("*********************");
          return true;
        }
      }
      Serial.printf("Tempo em segundos tentado verificar o state da mensagem da fila From-Mobile: %d\n", (millis() - last_count_time) / 1000);
      Serial.println("Timout na execucao da rotina de verificar o state da mensagem da fila From-Mobile!");
      Serial.println("Error na rotina de verificar o state da mensagem da fila From-Mobile!");
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool OrbcommModem::orbcommRoutineMessageReceiving(void)
{
  uint64_t last_count_time = millis();
  while ((millis() - last_count_time) < TIMEOUT_MENSSAGE_STATE)
  {
    bool receiving_state = receivingRoutineMessageToMobile();
    if (receiving_state == true)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool OrbcommModem::deleteQueueMessageFromMobile(string index_message)
{
  string result = executeAtCommand("AT%MGRC=\"ZeusMsg" + index_message + "\"", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  if (result.find("OK") != string::npos)
  {
    Serial.println("Deletado mensagem com sucesso!");
    return true;
  }
  else if (result.find("ERROR") != string::npos)
  {
    Serial.println("Error em deletar mensagem!");
    return false;
  }
  else
  {
    Serial.println("[deleteQueueMessageFromMobile] - Problema na comunicacao!");
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
      Serial.print("Mensagem State: ");
      Serial.println(new_result);
      return true;
    }
    else
    {
      Serial.print("Mensagem State: ");
      Serial.println(new_result);
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
      Serial.println("Fila vazia uma mensagem foi adicionada na fila From-Mobile com sucesso!");
      return true;
    }
    else
    {
      Serial.println("Fila vazia ERRO em adicionar uma mensagem na fila From-Mobile");
      return false;
    }
  }
  else
  {
    is_ready_to_send_message = routineDeletingMessageFromMobile(index_message);
    if (is_ready_to_send_message == true)
    {
      sendQueueForMessage(index_message, raw_payload_encoded_data, sin, min);
      Serial.println("Adicionada de mensagem na fila From-Mobile apos a rotina de delecao de mensagem da fila From-Mobile!");
      return true;
    }
    else
    {
      Serial.println("Error na rotina de envio de mensagem da fila From-Mobile!");
      return false;
    }
  }
}

bool OrbcommModem::routineDeletingMessageFromMobile(string index_message)
{
  bool status_delete_queue_message = deleteQueueMessageFromMobile(index_message);
  Serial.println("Fila cheia executando rotina de delecao de mensagem fila From-Mobile...");

  if (status_delete_queue_message == true)
  {
    Serial.println("Fila vazia terminado rotina de delecao de mensagem da fila From-Mobile!");
    return true;
  }
  else
  {
    uint64_t last_count_time = millis();
    while ((millis() - last_count_time) < TIMEOUT_DELETION_MSG)
    {
      status_delete_queue_message = deleteQueueMessageFromMobile(index_message);
    }
    Serial.printf("Tempo em segundos tentado deletar a mensagem da fila From-Mobile: %d\n", (millis() - last_count_time) / 1000);
    Serial.println("Timout na execucao da rotina de delecao de mensagem da fila From-Mobile!");
    Serial.println("Error na rotina de deleção de mensagem da fila From-Mobile!");

    return false;
  }
}

bool OrbcommModem::checkQueueMessagesFromMobile(void)
{
  string result = executeAtCommand("AT%MGRS", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);
  char *c_result;
  c_result = &result[0];
  if ((result.find("%MGRS:") != string::npos) && (result.find("OK") != string::npos) && (result.length() == 26))
  {
    Serial.println("Fila Vazia!");
    return true;
  }
  else if ((result.find("%MGRS:") != string::npos) && (result.find("OK") != string::npos) && (result.length() > 26))
  {
    Serial.println("Fila Cheia!");
    return false;
  }
  else if (result.find("ERROR") != string::npos)
  {
    Serial.println("Error no comando!");
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

bool OrbcommModem::getGPS(void)
{
  string result = executeAtCommand("AT%GPS=15,1,\"RMC\"", ORBCOMM_OK, ORBCOMM_ERROR, TIMEOUT_COMMAND);

  // Procurar o início da string RMC
  size_t rmcPos = result.find("$GPRMC");
  if (rmcPos != string::npos)
  {
    // Encontramos o início do campo RMC, avançamos para a posição correta
    rmcPos += 7; // Tamanho de "$GPRMC,"

    // Extrair os campos relevantes com sscanf
    int hour, minute, second;
    char latitude_dir, longitude_dir;
    float latitude_local, longitude_local, speed, course;

    if (sscanf(result.c_str() + rmcPos, "%02d%02d%02d.%*d,%*c,%f,%c,%f,%c,%f,%f,%*[^*]", &hour, &minute, &second, &latitude_local, &latitude_dir, &longitude_local, &longitude_dir, &speed, &course) == 9)
    {
      // Converter para decimal corretamente
      int degrees_latitude = int(latitude_local / 100);
      float minutes_latitude = fmod(latitude_local, 100);
      float latitude_decimal = degrees_latitude + (minutes_latitude / 60);

      if (latitude_dir == 'S')
      {
        latitude_decimal = -latitude_decimal;
      }

      int degrees_longitude = int(longitude_local / 100);
      float minutes_longitude = fmod(longitude_local, 100);
      float longitude_decimal = degrees_longitude + (minutes_longitude / 60);

      if (longitude_dir == 'W')
      {
        longitude_decimal = -longitude_decimal;
      }

      // Atualizar os valores de latitude e longitude
      latitude = latitude_decimal;
      longitude = longitude_decimal;

      // Debugging
      Serial.print("LATITUDE: ");
      Serial.println(latitude, 7); // Use 7 casas decimais para maior precisão.
      Serial.print("LONGITUDE: ");
      Serial.println(longitude, 7); // Use 7 casas decimais para maior precisão.

      return true;
    }
  }
  return false;
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

  Serial.print("Data: ");
  Serial.println(date.c_str());

  Serial.print("Hora: ");
  Serial.println(time.c_str());

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
    Serial.println("*****************************************************");
    Serial.print("Nome da mensagem da fila To-Mobile: ");
    Serial.println(new_result);
    Serial.println("*****************************************************");
    return new_result;
  }
  else if ((result.find("%MGFN") != string::npos) && (result.find("OK") != string::npos) && (result.length() <= 17))
  {
    Serial.println("[getQueueOfMessagesToMobile] - Não existe mensagem na fila To-Mobile");
    return result;
  }
  else
  {
    Serial.println("[getQueueOfMessagesToMobile] - Erro na execução do comando AT%MGFN");
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
    Serial.println("*****************************************************");
    Serial.print("Conteudo da mensagem da fila To-Mobile: ");
    Serial.println(new_result);
    message_content_ToMobile = new_result;
    Serial.println("*****************************************************");
    return true;
  }
  else if ((result.find("%MGFG") != string::npos) && (result.find("OK") != string::npos) && (result.length() <= 9))
  {
    Serial.println("[getContentOfSpecificMessagesOfQueueToMobile] - Não existe mensagem na fila To-Mobile");
    return false;
  }
  else
  {
    Serial.println("[getContentOfSpecificMessagesOfQueueToMobile] - Erro na execução do comando AT%MGFG=NomeMensagem");
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
  // Serial.printf("MSG: %s\n", encode_data.c_str());

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

  Serial.print("TimeStamp:");
  Serial.println(total_seconds);

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

vector<uint8_t> OrbcommModem::hexStringToBytes(const string &hexString)
{
  vector<uint8_t> bytes;

  for (size_t i = 0; i < hexString.length(); i += 2)
  {
    string byteString = hexString.substr(i, 2);
    uint8_t byte = strtol(byteString.c_str(), NULL, 16);
    bytes.push_back(byte);
  }

  return bytes;
}

string OrbcommModem::executeAtCommand(string command, string result_expected, string error_expected, int command_timeout = 5000)
{

  Serial.print("Comando a executar: ");
  Serial.println(command.c_str());
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
      Serial.println(result.c_str());
      last_serial_read_instant = millis();
      if (result.find(error_expected) != string::npos)
        return result;
      else if (result.find(result_expected) != string::npos)
        return result;
    }
  }
  return result;
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
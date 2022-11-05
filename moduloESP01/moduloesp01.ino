/*********************************************************************************************************************************
  Basic_Insert_ESP.ino
      
  Library for communicating with a MySQL or MariaDB Server
  
  Based on and modified from Dr. Charles A. Bell's MySQL_Connector_Arduino Library https://github.com/ChuckBell/MySQL_Connector_Arduino
  to support nRF52, SAMD21/SAMD51, SAM DUE, STM32F/L/H/G/WB/MP1, ESP8266, ESP32, etc. boards using W5x00, ENC28J60, LAM8742A Ethernet,
  WiFiNINA, ESP-AT, built-in ESP8266/ESP32 WiFi.
  The library provides simple and easy Client interface to MySQL or MariaDB Server.
  
  Built by Khoi Hoang https://github.com/khoih-prog/MySQL_MariaDB_Generic
  Licensed under MIT license
 **********************************************************************************************************************************/
/*
  MySQL Connector/Arduino Example : connect by wifi
  This example demonstrates how to connect to a MySQL server from an
  Arduino using an Arduino-compatible Wifi shield. Note that "compatible"
  means it must conform to the Ethernet class library or be a derivative
  with the same classes and methods.
  
  For more information and documentation, visit the wiki:
  https://github.com/ChuckBell/MySQL_Connector_Arduino/wiki.
  INSTRUCTIONS FOR USE
  1) Change the address of the server to the IP address of the MySQL server
  2) Change the user and password to a valid MySQL user and password
  3) Change the SSID and pass to match your WiFi network
  4) Connect a USB cable to your Arduino
  5) Select the correct board and port
  6) Compile and upload the sketch to your Arduino
  7) Once uploaded, open Serial Monitor (use 115200 speed) and observe
  If you do not see messages indicating you have a connection, refer to the
  manual for troubleshooting tips. The most common issues are the server is
  not accessible from the network or the user name and password is incorrect.
  Created by: Dr. Charles A. Bell
*/

#if ! (ESP8266 || ESP32 )
  #error This code is intended to run on the ESP8266/ESP32 platform! Please check your Tools->Board setting
#endif

#include "Credentials.h"

#define MYSQL_DEBUG_PORT      Serial

// Debug Level from 0 to 4
#define _MYSQL_LOGLEVEL_      1

#include <MySQL_Generic.h>

#define USING_HOST_NAME     false

#if USING_HOST_NAME
  // Optional using hostname, and Ethernet built-in DNS lookup
  char server[] = "db4free.net"; // change to your server's hostname/URL
#else
  IPAddress server(85,10,205,173);
#endif

uint16_t server_port = 3306;    //3306;

char default_database[] = "projetopi";           //"test_arduino";
char default_table[]    = "coletaDados";          //"test_arduino";

String INSERT_SQL;
// exemplo de string de inserção de dados
//String("INSERT INTO projetopi.coletaDados (chuva,vazao,nivelR1) VALUES (11.0,22.0,33.0)");

MySQL_Connection conn((Client *)&client);

MySQL_Query *query_mem;

void setup()
{
  //pinMode(LED_BUILTIN, OUTPUT);
   
  Serial.begin(9600);
  while (!Serial && millis() < 5000); // wait for serial port to connect

  MYSQL_DISPLAY1("\nIniciando Basic_Insert_ESP em", ARDUINO_BOARD);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  // Begin WiFi section
  MYSQL_DISPLAY1("Conectado a ", ssid);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    MYSQL_DISPLAY0(".");
  }

  // print out info about the connection:
  MYSQL_DISPLAY1("Conectado a Rede. Meu Endereço de IP é: ", WiFi.localIP());

  MYSQL_DISPLAY3("Conectando ao Servidor SQL @", server, ", Porta =", server_port);
  MYSQL_DISPLAY5("Usuario =", user, ", Senha =", password, ", BD =", default_database);
}

void runInsert()
{
  // Initiate the query class instance
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    MYSQL_DISPLAY(INSERT_SQL);
    
    // Execute the query
    // KH, check if valid before fetching
    if ( !query_mem.execute(INSERT_SQL.c_str()) )
    {
      MYSQL_DISPLAY("Erro ao inserir");
    }
    else
    {
      MYSQL_DISPLAY("Dados Inseridos");
    }
  }
  else
  {
    MYSQL_DISPLAY("Desconectado do Servidor. Não poi possível inserir os dados");
  }
}

// Função que lê o conteúdo da comunicação serial rx-tx carater a caracter e retorna uma string 
String leStringSerial(){
  String conteudo = "";
  char caractere;
  
  while(Serial.available() > 0) {
    caractere = Serial.read();
    if (caractere != '\n'){
      conteudo.concat(caractere);
    }
    delay(10);
  }
  return conteudo;
}

void loop()
{
    
      if (Serial.available() > 0)
      {
            INSERT_SQL = leStringSerial();
            if(INSERT_SQL.substring(0,11)=="INSERT INTO")  // Testa se o texto começa com "INSERT INTO" , sendo um comando SQL , executa 
            {
              if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
              {
                //digitalWrite(LED_BUILTIN, LOW);
                delay(500);
                  runInsert();
                conn.close();  // close the connection
                //digitalWrite(LED_BUILTIN, HIGH);
              } 
              else 
              {
                MYSQL_DISPLAY("\nFalha de conexão. Tentando novamento na próxima iteração.");
              }
            
              MYSQL_DISPLAY("\nAguardando...");
            }
     }
}

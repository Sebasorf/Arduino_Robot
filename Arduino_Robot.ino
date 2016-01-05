//Probar ya que esta versión no la probe. Debería andar con la aplicación

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

//WiFi Shield
#define led13 13
#define ADAFRUIT_CC3000_IRQ 3           
#define ADAFRUIT_CC3000_VBAT 5
#define ADAFRUIT_CC3000_CS 10        
#define WLAN_SSID "WIFIMobile"
#define WLAN_PASS "123456789"
#define WLAN_SSID1 "SpeedyHidden"
#define WLAN_PASS1 "qpWOeiRUty"
#define WLAN_SSID2 "WIFI4940"
#define WLAN_PASS2 "12345678"
#define WLAN_SECURITY WLAN_SEC_WPA2
#define LISTEN_PORT 8888           

//Motor driver
#define ENA 9
#define IN1 8
#define IN2 7
#define IN3 6
#define IN4 4
#define ENB 2
#define LUCES 23
         
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);
Adafruit_CC3000_Server server(LISTEN_PORT);
  
char m;
boolean adelante=true;
int velocidad=20; 
                                              
void setup() { 
  pinMode(led13, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT); 
  pinMode(LUCES, OUTPUT);
  digitalWrite(led13, LOW);
  Serial.begin(115200);
  inicializarMotoresPositivo(); 
  delay(500);
  Serial.println("Inicializando servidor...");
  if(!cc3000.begin())
  {
    Serial.println("No se puede inicializar la placa");
    while(1)
    {
      error();
    }
  }
  Serial.println("Intentando conectar a la red");   
  if(cc3000.connectToAP(WLAN_SSID1, WLAN_PASS1, WLAN_SECURITY, 2))
  {
    Serial.println("Se conecto correctamente a la red");
    digitalWrite(led13, HIGH);    
    while(!cc3000.checkDHCP())
    {
      delay(100);
    }
    while(!mostrarDatosDeConexion())
      delay(1000);
    server.begin();                                              
    Serial.println("Esperando una nueva conexion...");
    Serial.println();
  }                                                              
  else
  {
    Serial.println("No se puede conectar a la red");
    while(1)
      error();
  }
}
                                                                        
void loop() {
  
   Adafruit_CC3000_ClientRef cliente = server.available();
   //Apenas se conecta el sistema android debe enviar un caracter 
   if(cliente.available())
   {
     digitalWrite(LUCES, HIGH);
     m=cliente.read();
     switch(m){ 
     case 'm':  Serial.println(m);
                comprobarProtocolo(m, cliente);
                break;
     case 'a':  Serial.println(m);
                aumentarVelocidad();
                break;
     case 'd':  Serial.println(m);
                disminuirVelocidad();
                break;
     case 'p':  Serial.println(m);
                parar();
                break;
     case 'b':  Serial.println();
                acelerar();
                break;
     case 'r':  Serial.println();
                marchaAtras();
                break;
     default:   Serial.println("No se reconoce el comando");
                break;
     }
   }
}

//Funciones de usuario
void comprobarProtocolo(char m, Adafruit_CC3000_ClientRef cliente)
{
   if(m=='m')
     Serial.println("Se cumple el protocolo... Esperando comandos");
   else
   {
     Serial.println("No se cumple el protocolo, intente conectar nuevamente");
     cliente.stop();
     cliente.flush();
   }
}

boolean mostrarDatosDeConexion()
{                                                                      
  uint32_t dirIP, netMask, gateway, dhcpserv, dnsserv;
  if(!cc3000.getIPAddress(&dirIP, &netMask, &gateway, &dhcpserv, &dnsserv))
  {                                                              
    Serial.println("No se puede mostrar los detalles");
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(dirIP);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netMask);        
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);           
    Serial.println();
    return true;
   } 
}

void inicializarMotoresPositivo()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void inicializarMotoresNegativo()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW); 
}

void aumentarVelocidad()
{
  digitalWrite(LUCES, LOW);
  if(adelante)                              //Si está yendo para adelante  
  {
    if(velocidad<255)                          //Si la velocidad no es la mayor
    {  velocidad+=5;                                  //Aumento velocidad
      Serial.println("Aumento velocidad con sentido para adelante");
      Serial.println(velocidad);
    } 
  }
  else                                      //Sino, Osea, si esta yendo para atras
  {
    if(velocidad>20)                           //Si la velocidada es mayor a 19
    {
      digitalWrite(LUCES, HIGH);
      velocidad-=5;                                  //Disminuyo velocidad
      Serial.println("Disminuyo velocidad intentando ir para adelante");
      Serial.println(velocidad);
    }
    else                                       //Sino, si la velocidad es igual a 19
    {
      adelante=true;                           //Entonces voy a ir para adelante
      inicializarMotoresPositivo();            //Inicialializo el motor con el sentido positivo
      velocidad+=5;
      delay(500);
      Serial.println("Cambio el sentido para adelante e inicializo el motor positivo");
      Serial.println(velocidad);
    } 
  }
  actualizarVelocidad(velocidad);
}

void disminuirVelocidad()
{
  digitalWrite(LUCES, HIGH);
  if(adelante)
  {
    if(velocidad>19)
    {
      velocidad-=5;
      Serial.println("Bajo velocidad con sentido para adelante");
      Serial.println(velocidad);
    }
    else
    {
      adelante=false;
      inicializarMotoresNegativo();
      velocidad+=5;
      delay(500);
      Serial.println("Cambio el sentido para atras, inicializo motor negativo");
      Serial.println(velocidad);
    } 
  }
  else
  {
    if(velocidad<255)
    {
      velocidad+=5;
      Serial.println("Aumento velocidad con sentido para atras");
      Serial.println(velocidad);
    }
  }
  actualizarVelocidad(velocidad);
}

void parar()
{
  digitalWrite(LUCES, HIGH);
  velocidad=0;
  actualizarVelocidad(velocidad);
  Serial.println("El robot se detuvo");
}

void acelerar()
{
  digitalWrite(LUCES, LOW);
  if(velocidad<=40 || adelante == false)
  {
    parar();
    inicializarMotoresPositivo();
    adelante = true;
    velocidad=100;
    digitalWrite(LUCES, LOW);
    actualizarVelocidad(velocidad);
    Serial.println("Acelerando el robot");
    Serial.println(velocidad);
  }
}

void marchaAtras()
{
  digitalWrite(LUCES, HIGH);
  if(velocidad <= 40 || adelante==true)
  {
    parar();
    inicializarMotoresNegativo();
    adelante = false;
    velocidad=100;
    actualizarVelocidad(velocidad);   
    Serial.println("Auto corriendo en marcha atrás");
    Serial.println(velocidad);
  }
}

void actualizarVelocidad(int velocidad)
{
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);
  //Acá envió la velocidad
  server.write(velocidad);
}


void error(){
  delay(1000);
  digitalWrite(led13, HIGH);
  delay(1000);
  digitalWrite(led13, LOW);
}


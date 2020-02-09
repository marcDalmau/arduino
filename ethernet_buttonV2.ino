#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11
const int EchoPin = 5;
const int TriggerPin = 6;
DHT dht(DHTPIN, DHTTYPE);

byte mac[]={0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1,175);
EthernetServer server(80);

String HTTP_req; // Para guardar la petición
String heatIndexC;
String humidity;
String temperature;

//boolean LED2_status = 0;

void setup()
{   Ethernet.begin(mac, ip);  
    server.begin();
    dht.begin();             
    Serial.begin(9600);       
    //pinMode(4, OUTPUT);
    pinMode(DHTPIN, INPUT); 
    pinMode(TriggerPin, OUTPUT);
    pinMode(EchoPin, INPUT);
    delay(100);      
}

void loop()
{
  
   // Leemos la humedad relativa
    float h = dht.readHumidity();
    // Leemos la temperatura en grados centígrados (por defecto)
    float t = dht.readTemperature();
    // Comprobamos si ha habido algún error en la lectura
    if (isnan(h) || isnan(t))
    {
      Serial.println("Error obteniendo los datos del sensor DHT11");
      return;
    }
    // Calcular el índice de calor en grados centígrados
    float hic = dht.computeHeatIndex(t, h, false);
    humidity = h;
    temperature = t;
    //heatIndexC = hic; 
    Serial.print("Humidity:");
    Serial.print(humidity);
    Serial.print(" Temperatura:");
    Serial.println(temperature);
    
    //COMENÇA LECTURA CLIENT - HTTP REQUEST
    EthernetClient client = server.available();    // Comprobamos si hay peticiones
    if (client)
    {  
        boolean currentLineIsBlank = true;
        while (client.connected())
        { 
          if (client.available())
          {
            char c = client.read(); 
            HTTP_req += c;
            if(c=='\n' && currentLineIsBlank)
            { 
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connection: close");
             client.println();
             client.println("<!DOCTYPE html>");   
             client.println("<html>");
             client.println("<head>");
             client.println("<title>Control de LEDs en Arduino</title>");
             client.println("</head>");
             client.println("<body>");
             client.println("<h1>ESTATS</h1>");
             client.println("<form method=\"get\">");
             processCheckData(client, humidity, temperature);
             client.println("</form>");
             client.println("</body>");
             client.println("</html>");
          
             HTTP_req = "";
             break;
            }
            if(c=='\n')
                  currentLineIsBlank = true;
                else if (c != '\r')
                  currentLineIsBlank = false;
          } 
       }   // WHile
       delay(10);      // dar tiempo
       client.stop(); // Cierra conexion
     }  // If client
  }
  
void processCheckData(EthernetClient cl, String humidity, String temperature)
  {  
  String distancia;
  int cm;
     Serial.print(HTTP_req);
         
     if (HTTP_req.indexOf("favicon") > -1) //FAVICON AL APRETAR EL BOTON
     {
         Serial.println("favicon");
     
     }else if (HTTP_req.indexOf("ON= HTTP") > -1) {
             digitalWrite(4, HIGH);
             cm = ping(TriggerPin, EchoPin);
             distancia = cm;
             cl.println("<p>ELS VALORS SON:</p>");
             cl.println("<p>humitat(%):"+humidity+"</p>");    
             cl.println("<p>temperatura(C):"+temperature+"</p>");
             cl.println("<p>distancia(cm):"+distancia+"</p>");    
             cl.println("<button type=submit name=OFF>LED OFF");
             cl.println("<button type=submit name=ON>Actualitzar dades clima</button>");
             
     } else if (HTTP_req.indexOf("OFF= HTTP") > -1) {           
             digitalWrite(4, LOW);
             Serial.println("PONE EL LED A 0");
             cl.println("<button type=submit name=ON>LED ON");
     }else {
           digitalWrite(4, LOW);
           cl.println("<button type=submit name=ON>LED ON");
       //    cl.println("<button type=submit name=A_CL>ACTUALITZA CLIMA");
           Serial.println("ENVIA LED2 PARA QUE LO PONGA A UNO");
     }
   }

  int ping(int TriggerPin, int EchoPin) {
  long duration, distanceCm;
  
  digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  
  duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
  
  distanceCm = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
  return distanceCm;
}

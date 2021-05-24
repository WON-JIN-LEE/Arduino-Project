#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#define DHTTYPE DHT11   // DHT 11

#define dht_dpin 4
DHT dht(dht_dpin, DHTTYPE);

//먼지센서 고정 요소
#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000           


#include <ESP8266WiFi.h>

const char* ssid = "Ana"; //사용자의 무선 랜 ID 
const char* password = "0987654321a"; //무선 랜 패스워드

//const char* ssid = "hi"; //사용자의 무선 랜 ID 
//const char* password = "01035928294"; //무선 랜 패스워드

//LED 핀
const int led1Pin =  2; //핀 번호 GPIO16번을 씀

//먼지센서 핀, 변수선언
const int iled = 5;                                            //drive the led of sensor
const int vout = 0;                                            //analog input
float density, voltage;
int   adcvalue;

//DC모터 핀
int IN1 = D5; //GPIO 14
int IN2 = D6; //GPIO 12
int IN3 = D7; //GPIO 13
int IN4 = D8; //GPIO 15

int motor_power=0;
//int dust_Average = 70;

//먼지센서 함수
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80); // 와이파이 서버 (80)할당, 규격이 정해져있음

//setup() 초기상태 설정 한번만 실행함
void setup() {  

 // LED 핀 셋업
  pinMode(led1Pin, OUTPUT); //핀을 출력 상태로 설정
  digitalWrite(led1Pin, LOW); // 핀으로 LOW신호 출력
  
 //먼지센서 핀  ON
 // pinMode(iled, OUTPUT);
 // digitalWrite(iled, LOW);                                     //iled default closed
  pinMode(iled, INPUT);
  digitalWrite(iled, LOW);   


  pinMode(dht_dpin, INPUT); // 온습도 센서

 //DC모터
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  
  Serial.begin(115200);                                         //send and receive at 9600 baud
  Serial.print("*********************************** WaveShare ***********************************\n");

  //DHT11
  dht.begin(); //DHT 초기화
  
   
 //시리얼 시작과 함께 (속도) 지정
  Serial.println("Humidity and temperature\n\n");
  delay(10);
  
  // Connect to WiFi network
  Serial.println(); 
  Serial.println(); //엔터
  Serial.print("Connecting to ");
  Serial.println(ssid); 
  
  WiFi.begin(ssid, password); // 와이파이 시작
 
 //연결될 때까지 반복
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); //0.5초마다 “.” 출력
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); //연결 성공하면 시리얼모니터에 출력
  
  // Start the server
  server.begin(); //할당 받은 서버 시작
  Serial.println("Server started"); //시리얼 모니터에 서버시작 구문 출력

  // Print the IP address
  Serial.println(WiFi.localIP()); // 사용되고 있는 IP주소 출력

  
}

// 사용할동안 계속 반복하는 구문
void loop() {

//DHT 온습도 초기화
float humidity =0;
float temp = 0; 

  WiFiClient client = server.available();  //클라이언트로(사용자)로부터 응답을 받음
//응답이 없으면 server.available()는 0이고 응답이 있으면 1을 넘겨준다. 
if (!client) {
    return; //응답이 없으면 리턴함
  }



  // 새로운 응답이오면 시리얼에 출력
  Serial.println("new client");
  client.setTimeout(10000);
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');//‘\r’은 문장의 끝을 알려줌.  한문장씩 읽어들    임
  Serial.println(req);
  client.flush(); //받을거 다 받았으면 끊어줌

  while(client.available()) {
    client.read(); //전송된 데이터가 있을 경우 데이터를 읽어들인다.
  }


 //indexOf() 특정한 문자열의 위치(index)를 반환한다. 탐색하려는 문자열이 존재하지 않는다면 -1을 반환한다.
// 읽어온 문장에 "/?led1=ON" 이 있으면 위치 인덱스값 넘겨줌
  if (req.indexOf("/?LED=ON") > 0) {
    
    digitalWrite(led1Pin, HIGH); // LED ON
    pinMode(iled, OUTPUT); //먼지센서 ON
    
    humidity = dht.readHumidity(); //온습도
    temp = dht.readTemperature();   

    
    Serial.println(F("LED1 is ON")); 
    Serial.println(F("sensor ON")); // 데이터 메모리(SRAM)에 저장하지 않고  프로그램 메모리(Flash)에 저장할 수 있도록 F() 함수를 이용함

   
     motor_power=3;

     
        if (req.indexOf("/?LED=ON1") > 0) {
     
           motor_power=1;
        }
        else if (req.indexOf("/?LED=ON2") > 0) {
     
           motor_power=2;
        }
        else if (req.indexOf("/?LED=ON3") > 0) {
     
           motor_power=3;
        }


   // Density 계산식
 digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    density = voltage * COV_RATIO;
  }
  else
    density = 0;

         
  }
  else if (req.indexOf("/?LED=OFF") > 0) {
    
     digitalWrite(led1Pin, LOW);
     pinMode(iled, INPUT);//먼지센서 OFF
     
    // humidity =0;//온습도
     //temp = 0;   
   
     digitalWrite(IN1, HIGH); //모터1 정지
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, HIGH); //모터2 정지
     digitalWrite(IN4, HIGH);
     
     motor_power=0; 
     density = 0;
     
    Serial.println(F("LED1 is OFF"));  
    Serial.println(F("sensor off")); 
  }
  
/////////////////////////////////////온습도 계산
    
  //serial 모니터에 출력
  Serial.print("Humidity : ");
  Serial.print(humidity);
  Serial.print(" Temperature : ");
  Serial.print(temp);
  Serial.println(" ºC");



//모터속도 조건문
switch (motor_power) {
  case 1:
    analogWrite(IN1, 300);
    analogWrite(IN2, 0); 
    analogWrite(IN3, 300);
    analogWrite(IN4, 0); 
    break;
  case 2:
    analogWrite(IN1, 650);
    analogWrite(IN2, 0);
    analogWrite(IN3, 650);
    analogWrite(IN4, 0); 
    break;
  case 3:
    analogWrite(IN1, 1000);
    analogWrite(IN2, 0);
    analogWrite(IN3, 1000);
    analogWrite(IN4, 0); 
    break;
}

  // html 형식 Web 구성
          client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n");
          client.print("<head>"); 
          client.print("<meta charset=\"UTF-8\" http-equiv=\"refresh\" content=\"1\">");
          client.print("</head>");

          //스타일추가
          client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">");
          client.println("<style type=\"text/css\"><!--");
          client.println("body {color: #000000; font-size: 15pt;}");
          client.println("h3 {color: #008000; font-size: 30pt;}");
          client.println("h2 {color: #830000; font-size: 20pt;}");  
          //client.println("hr {color: #008000; size: 100px noshade;}");  
          //client.println("#footer {text-align: center; margin-top: 20px;}");
         // client.println("#footer p {font-size: 9pt; color: #85144b;}");
          client.println("//--></style>");

          
          client.println("<body>");
          client.println("<div class=\"container\"><h3>NodeMCU Webserver</h3>");
          client.println("<img src=\"https://image.chosun.com/sitedata/image/201908/23/2019082301404_0.png\" class=\"img-rounded\" alt=\"Cinque Terre\" width=\"400\">"); // 이미지 삽입코드
          client.println("<div><br>NodeMCU에서 돌아가는 웹서버입니다. <br>웹브라우저로 접속해서 각종 센서 데이터를 <br>출력하거나 디바이스를 컨트롤 할 수 있습니다.</div>");
          
          client.println("<hr width=\"40%\" align=\"left\" >"); //구분선
         
          client.println("<h2>센서 작동</h2>");
          
          //ON 버튼
          client.println(F("<form method=\"get\">")); 
          client.println(F("<input type=\"hidden\" name=\"LED\" value=\"ON\">")); 
          client.println(F("<input type=\"button\" name=\"button1\" value=\"ON\" class=\"btn btn-success\" onclick=\"submit();\" style=\"height:60px; width:120px;  font-size:30px; float: left;\" >")); 
          client.println(F("</form>")); 
          
          //OFF 버튼
          client.println(F("<form method=\"get\">")); 
          client.println(F("<input type=\"hidden\" name=\"LED\" value=\"OFF\">")); 
          client.println(F("<input type=\"button\" name=\"button1\" value=\"OFF\" class=\"btn btn-danger\" onclick=\"submit();\" style=\"height:60px; width:120px;  font-size:30px; float: left;\" >")); 
          client.println(F("</form>")); 
          client.println("<br />"); //줄바꿈 코드
          client.println("<br />"); //줄바꿈 코드
          
          client.println("<hr width=\"40%\" align=\"left\" >"); //구분선
          
          client.println("<h2>바람세기</h2>");
          
          client.print(" 풍속 : ");
          switch (motor_power) {
              case 1:
                client.print(" 약 ");
                break;
              case 2:
                client.print(" 중 ");
                break;
              case 3:
                client.print(" 강 ");
                break;
              default :
               client.print(" 꺼짐 ");
                
            }
          //약모터 버튼
          client.println(F("<form method=\"get\">")); 
          client.println(F(" <input type=\"hidden\" name=\"LED\" value=\"ON1\">")); 
          client.println(F(" <input type=\"button\" name=\"button3\" value=\"약\" onclick=\"submit();\" style=\"height:60px; width:120px;  font-size:30px; float: left;\" >")); 
          client.println(F("</form>"));
         
          //중모터 버튼
          client.println(F("<form method=\"get\">")); 
          client.println(F(" <input type=\"hidden\" name=\"LED\" value=\"ON2\">")); 
          client.println(F(" <input type=\"button\" name=\"button4\" value=\"중\" onclick=\"submit();\" style=\"height:60px; width:120px;  font-size:30px; float: left;\" >")); 
          client.println(F("</form>"));
        
          //강모터 버튼
          client.println(F("<form method=\"get\">")); 
          client.println(F(" <input type=\"hidden\" name=\"LED\" value=\"ON3\">")); 
          client.println(F(" <input type=\"button\" name=\"button5\" value=\"강\" onclick=\"submit();\" style=\"height:60px; width:120px;  font-size:30px; float: left;\" >")); 
          client.println(F("</form>")); 
          
          client.println("<br />"); //줄바꿈 코드
          client.println("<br />"); //줄바꿈 코드

          client.println("<hr width=\"40%\" align=\"left\" >"); //구분선
          
          client.println("<h2>온/습도, 미세먼지 센서</h2>");
          
          client.print("Temperature : ");
          client.print(temp);
          client.print(" °C");
          client.print("<br>");
          client.print("Humidity : ");
          client.print(humidity);
          client.print(" %");
          client.print("<br>");
          client.print(" density : ");
          client.print(density);
          client.print(" ug/m3\n");
          
  
          client.println("<br />");
    
          client.println("</body>");
          client.println("</html>");


          

  delay(1);
  Serial.println("Client disonnected");

}

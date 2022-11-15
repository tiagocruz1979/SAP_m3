
// Display de Cristal Líquido
// INCLUSÃO DE BIBLIOTECAS
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//Definições
#define endereco  0x27 // Endereços comuns: 0x27, 0x3F
#define colunas   16
#define linhas    2
LiquidCrystal_I2C lcd(endereco, colunas, linhas); // instancia objeto

// Variaveis e constantes para pluviometro
  const int portaREED = 2;   // porta digital para ligação do pluviômetro
  const int portaLEDPluv = 7; // portal digital com led indicador de dados lidos no pluviômetro;

// Variaceis e Constantes Sensos Vazao
  const int portaVazao = 3; // porta digital de instalação do pino amarelo do sensor de vazão
  const int portaLEDVazao = 8; // portal digital com led indicador de dados lidos no sensor de vazão
  
//Sensor UltraSonico
  #include <Ultrasonic.h>
  const byte pino_trigger = 4;
  const byte pino_echo = 5;
  Ultrasonic ultrasonic(pino_trigger, pino_echo);

// Constantes Gerais;
  const unsigned int intervalo = 20000; // intervalo de tempo entre cada leitura e envio ao banco de dados. 
  //const int pinLED_Atividade = 6;

class Pluviometro
{
  private:
    double fatorAlturaPulsos; // fator de conversão da leitura dos pulos do pluviômetro convertidos em mm (altura de chuva)
    volatile int contaPulsos;

  public:
    Pluviometro(double fatorAlturaPulsos)
    {
      this->contaPulsos = 0;
      this->fatorAlturaPulsos = fatorAlturaPulsos;
    }

    void addPulso(){this->contaPulsos++;}
    void reiniciaPulsos(){this->contaPulsos=0;}
    
    int getPulsos(){return this->contaPulsos;}
    
    double getChuva_mm()
    {
        double chuva;
        chuva = getPulsos() * fatorAlturaPulsos;
        return chuva;
    }
    
}; Pluviometro pluv(0.3423);

class SensorVazao
{
private:
  volatile int contaPulsos;
  double fatorConversaoPulsoLitro; // conversão do sensor de vazao em pulsos = litros
  int intervaloTempoMiliSeg;
  
public:
  SensorVazao(double fatorConversaoPulsoLitro, int intervaloTempoMiliSeg)
  {
    contaPulsos = 0;
    this->fatorConversaoPulsoLitro = fatorConversaoPulsoLitro;
    this->intervaloTempoMiliSeg = intervaloTempoMiliSeg;
  }

  void addPulso(){this->contaPulsos++;}
  void reiniciaPulsos(){this->contaPulsos=0;}

  int getPulsos(){return this->contaPulsos;}

  double getVolume_litros(){
    double volume;
    volume = getPulsos() * this->fatorConversaoPulsoLitro;
    return volume;
  }
  double getVazao_LitrosPorMinuto()
  {
    return getVolume_litros() * 60000 / this->intervaloTempoMiliSeg;
  }

}; SensorVazao vazao(0.002, intervalo);


class Reservatorio
{
  private:
  
  double alturaSensor;
  double larguraBase;
  double ComprimentoBase;
  double AlturaMaxima;
  Ultrasonic *us;

  public:

  Reservatorio(double AlturaSensor,double larguraBase, double comprimentoBase, double alturaMaxima, Ultrasonic *us)
  {
    this->alturaSensor = AlturaSensor;
    this->larguraBase = larguraBase;
    this->ComprimentoBase = comprimentoBase;
    this->AlturaMaxima = alturaMaxima;
    this->us = us;
  }
  double getCapacidade(){return larguraBase*ComprimentoBase*AlturaMaxima;}
  double getAreaBase(){return larguraBase*ComprimentoBase;}
  
  long getNivel()
  {
    long dist = long(getDistSensorUS());
    long n = alturaSensor - dist;
    if(n<0) return 0;
    return n;
  }
  double getVolume(){
    double areaBase = getAreaBase();
    long nivel = getNivel();
    return (areaBase * nivel / 1000);
  }

  float getDistSensorUS()
  {
    us->measure();
    return us->get_cm();
  }

};Reservatorio reservatorio(18.8,12.0,24.3,10.2, &ultrasonic);

  String reservatorio_teste(Reservatorio &r)
  {
      String str;
      str = "---Testes do Reservatorio--\n";
      str.concat("getCapacidade = "); str.concat(r.getCapacidade()); str.concat("\n"); str.concat("getAreaBase = "); str.concat(r.getAreaBase()); str.concat("\n");
      str.concat("getNivel = "); str.concat(r.getNivel()); str.concat("\n"); str.concat("getVolume = "); str.concat(r.getVolume()); str.concat("\n");
      str.concat("DistSensor = "); str.concat(r.getDistSensorUS()); str.concat("\n");      
      str.concat("---Fim do Teste---\n");
      return str;  
  }

 void setup() {
   // initializa a comunicaçao serial:
   Serial.begin(9600);
   
   lcd.init(); // INICIA A COMUNICAÇÃO COM O DISPLAY
   lcd.backlight(); // LIGA A ILUMINAÇÃO DO DISPLAY
   lcd.clear(); // LIMPA O DISPLAY
   atualizaDisplay(reservatorio.getVolume());

   // inicianliza configuração do pluviometro;
   pinMode(portaREED, INPUT);
   pinMode(portaLEDPluv,OUTPUT);
   attachInterrupt(digitalPinToInterrupt(portaREED),contaPulsoPluv,FALLING);

   // inicializa configuração do sensor de vazao;
   pinMode(portaVazao,INPUT);
   pinMode(portaLEDVazao, OUTPUT);
   attachInterrupt(digitalPinToInterrupt(portaVazao), contaPulsoVazao,RISING);

   // LED de atividade
   //pinMode(pinLED_Atividade,OUTPUT);
 }

// variaveis de controle para a opção de enviar ao banco de dados somente quando há novos dados atualizados
double LeituraAnteriorVazao = 0.0;
double LeituraAnteriorChuva = 0.0;
double LeituraAnteriorReservatorio = 0.0;

void loop() {

//digitalWrite(pinLED_Atividade,LOW);
digitalWrite(portaLEDPluv,LOW);
digitalWrite(portaLEDVazao,LOW);

sei();
  delay(intervalo);
cli();

//digitalWrite(pinLED_Atividade,HIGH);

//calcula o volume de chuva em mm no período;
double chuva = pluv.getChuva_mm();
pluv.reiniciaPulsos();

//calcula o a vazao no periodo em litros/minuto
double vazaoLitrosMinuto = vazao.getVazao_LitrosPorMinuto();
vazao.reiniciaPulsos();

//volume utilizado do reservatório
double VolumeReservatorio = reservatorio.getVolume();

//if((vazaoLitrosMinuto > 0.0 || chuva > 0.0 || LeituraAnteriorVazao>0.0 || LeituraAnteriorChuva > 0 || VolumeReservatorio!=LeituraAnteriorReservatorio) )
//{

  // exemplo de SQL de insersao de dados : INSERT INTO projetopi.coletaDados (chuva,vazao,nivelR1) VALUES (99.0,98.0,97.99)";
  
  String str_SQL;
  str_SQL  = "INSERT INTO projetopi.coletaDados (chuva,vazao,nivelR1) VALUES(";
  str_SQL += String(chuva);
  str_SQL += ",";
  str_SQL += String(vazaoLitrosMinuto);
  str_SQL += ",";
  str_SQL += String(VolumeReservatorio);
  str_SQL += ")\n";
  
  
  //str_SQL = "";
  //str_SQL = "INSERT INTO projetopi.coletaDados (chuva,vazao,nivelR1) VALUES(55.55,66.66,77.77)\n";

  Serial.write(str_SQL.c_str());
 
  // variaveis para controle de envio com a opção de enviar ao bd somente quando tiver dados atualizados
  //LeituraAnteriorVazao = vazaoLitrosMinuto;
  //LeituraAnteriorChuva = chuva;
  //LeituraAnteriorReservatorio = VolumeReservatorio;

//}
}


void atualizaDisplay(double volume)
{
   lcd.setCursor(4,0);
   lcd.print("SAPm3");
   lcd.setCursor(0,1);
   lcd.print("Volume: ");
   lcd.setCursor(8,1);
   lcd.print(volume);
   lcd.print(" l");
}

void contaPulsoPluv()
{
  const long espera = 300; // tempo de espera para considerar um novo pulso
  static long tempo = 0;

  if( (millis()-tempo) > espera )
  {
    pluv.addPulso();
    digitalWrite(portaLEDPluv,HIGH);
    tempo = millis();
  }
}

void contaPulsoVazao()
{
  const int espera = 30; // tempo de espera para considerar um novo pulso
  static long tempo = 0;

  if( (millis() - tempo) > espera )
  {
    vazao.addPulso();
    digitalWrite(portaLEDVazao,HIGH);
   tempo=millis();
  }
}

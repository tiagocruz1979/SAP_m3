// Display de Cristal Líquido
// INCLUSÃO DE BIBLIOTECAS
//#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
//Definições

#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <regex>
typedef int byte;

void atualizaDisplay(double);
void contaPulsoPluv();
void contaPulsoVazao();

#define OUTPUT 1
#define INPUT 0
void pinMode(int porta, int tipo)
{
    std::cout << "Pino " << porta << " configurado como " << (tipo==INPUT?"INPUT":"OUTPUT") << std::endl;
}

int digitalPinToInterrupt(int porta)
{
    if(porta==2) return 0;
    else if(porta==3) return 1;
    else return -1;
}

#define FALLING 1
#define RISING 2
void attachInterrupt(int numero, void(*funcao)(), int tipo)
{
    std::cout << "Interrupcao ativada " << numero << std::endl;
    funcao();
}

#define LOW 0
#define HIGH 1
void digitalWrite(int porta, int estado)
{
    std::cout << "Porta " << porta << " estado " << estado << std::endl;
}

void sei()
{
    std::cout << "sei\n";
}
void cli()
{
    std::cout << "cli\n";
}

void delay(long tempo)
{
    std::cout << "Delay " << tempo << std::endl;
    clock_t t = clock();
    while(clock()<t+tempo){}
}

long millis()
{
    clock_t t = clock();
    return (long)t;
}

class LiquidCrystal_I2C
{
    int endereco;
    int colunas;
    int linhas;
    int l,c;
public:
    LiquidCrystal_I2C(int endereco,int colunas, int linhas)
    {
        std::cout << "Instanciando objeto LCD (Endereco,Colunas,Linhas) (" << endereco << "," << colunas << "," << linhas << ")\n";
        this->endereco = endereco;
        this->colunas = colunas;
        this->linhas = linhas;
        this->l=this->c=0;
    }
    void init()
    {
        std::cout << "Inicializa LCD\n";
    }
    void backlight()
    {
        std::cout << "Liga Backlight\n";
    }
    void setCursor(int coluna, int linha)
    {
        std::cout << "LCD Cursor na coluna " << coluna << " e linha " << linha << std::endl;
        this->l = linha;
        this->c = coluna;
    }
    void clear()
    {
        std::cout << "Limpando LCD\n";
    }
    void print(std::string str)
    {
        std::cout << "LCD > " << str << std::endl;
    }
    void print(double valor)
    {
        std::cout << "LCD > " << std::to_string(valor) << std::endl;
    }
};

class Ultrasonic
{
    int trigger;
    int echo;
    float dist;
public:
    Ultrasonic(int trigger, int echo)
    {
        std::cout << "Instanciando objeto Ultrasonic...(trigger/echo) (" << trigger << "/" << echo << ") " << std::endl;
        this->trigger = trigger;
        this->echo = echo;
        this->dist = 0.0;
    }
    void measure()
    {
        std::cout << "Ultrasonic chamada de metodo 'measure'\n";
        srand(time(0));
        int t = 100+rand()%100;
        this->dist = (float)t/10;
    }
    float get_cm()
    {
        std::cout << "Ultrasonic chamada de metodo get_cm = " << this->dist << std::endl;
        return this->dist;
    }
};

class String{
    std::string str;
public:
    String(){};
    String(std::string str)
    {
        this->str = str;
    }
    String(double valor)
    {
        this->str = std::to_string(valor);
    }

    void concat(std::string str)
    {
        this->str.append(str);
    }
    void concat(double valor)
    {
        concat(std::to_string(valor));
    }
    void operator=(std::string str)
    {
        this->str = str;
    }
    void operator+=(std::string str)
    {
        concat(str);
    }
    void operator+=(double valor)
    {
        concat(valor);
    }
    void operator+=(String Str)
    {
        concat(Str.c_str());
    }
    std::string c_str()
    {
        return this->str;
    }

    std::string operator()()
    {
        return this->str;
    }


};

std::string arquivoTeste = "teste.txt";
std::ofstream resultado(arquivoTeste);

class SerialCom
{
    int velocidade;
public:
    SerialCom(){};
    void begin(int vel)
    {
        this->velocidade = vel;
    }
    void write(std::string str)
    {
        std::cout << "Saida Serial > " << str;
        if(!resultado.is_open())
            resultado.open(arquivoTeste,std::ios::app);

        if(resultado.is_open())
        {
            resultado << str;
        }
        else
        {
            std::cout << "Erro ao acessar arquivo" << std::endl;
        }
        resultado.close();
    }
    void write(String Str)
    {
        std::cout << Str.c_str();
    }
};


SerialCom Serial;


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
//  #include <Ultrasonic.h>
  const byte pino_trigger = 4;
  const byte pino_echo = 5;
  Ultrasonic ultrasonic(pino_trigger, pino_echo);

// Constantes Gerais;
  const int intervalo = 1000; // intervalo de tempo entre cada leitura e envio ao banco de dados.
  //const int pinLED_Atividade = 6;

class Pluviometro
{
  private:
    double fatorAlturaPulsos; // fator de conversão da leitura dos pulos do pluviômetro convertidos em mm (altura de chuva)
    volatile int contaPulsos = 0;

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
  double fatorConversaoPulsoLitro = 0.002; // conversão do sensor de vazao em pulsos = litros
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

}; SensorVazao vazao(0.02, intervalo);


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


    int aleatorio;

    aleatorio = rand() % 100;
    for(int i = 0 ; i < aleatorio;i++)
        pluv.addPulso();
    std::cout << "pulsos Pluviometro: " << pluv.getPulsos() << std::endl;

    aleatorio = rand() % 100;
    for(int i = 0 ; i < aleatorio;i++)
        vazao.addPulso();
    std::cout << "pulsos Vazao:" << vazao.getPulsos() << std::endl;

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


bool testeArqSaida(std::string arquivo)
{
    bool resultado = true;
    std::ifstream entrada;
    std::string sql;
    std::regex rx("INSERT INTO projetopi.coletaDados \\(chuva,vazao,nivelR1\\) VALUES\\([0-9]+.[0-9]+,[0-9]+.[0-9]+,[0-9]+.[0-9]+\\)");
    entrada.open(arquivo,std::ios::in);

    std::cout << "\nIniciando teste do arquivo de saida" << std::endl;
    if(entrada.is_open())
    {
        while(!entrada.eof())
        {
            getline(entrada,sql);
            if(sql=="") break;
            std::cout << "Saida: " << sql << "...";

            if(std::regex_match(sql,rx))
            {
                std::cout << "[OK]" << std::endl;
            }
            else
            {
                std::cout << "[FALHOU]" << std::endl;
                resultado = false;
            }
        }
    }
    return resultado;

}


int main()
{
    srand(time(NULL));

    setup();
    for(int i = 0 ; i < 5 ; i++)
    {
        loop();
    }

    if(testeArqSaida(arquivoTeste))
    {
        std::cout << "Concluido sem falhas" << std::endl;
    }
    else
    {
        std::cout << "Concluido com falhas" << std::endl;
    }

    return 0;
}



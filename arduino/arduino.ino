//Pinos para manipular as cores do led RGB, devem ser pinos PWM
const int pVerde = 10;
const int pAzul = 11;
const int pVermelho = 9;

//Variaveis de controle da intensidade das cores
int vermelho = 0;
int verde = 0;
int azul = 0;

//vetor responsavel por armazenar a comunicação serial
char buffer[18];

//função responsável por receber e tratar os comandos
void tratarComando(char* dados){
  
  //Serial.print("Comando recebido: ");
  //Serial.println(dados);
  
  //ponteiro: aloca espaço de memória dinamicamente
  char* parametro;
  
  //realiza a leitura do cache até o " " ou ","
  parametro = strtok(dados, " ,");
  
  //continua a leitura do cache até seu fim enviando os comandos
  //para a função ajustaLED
  while(parametro != NULL){
    ajustaLED(parametro);
    parametro = strtok(NULL, " ,"); //continua de onde parou
  }
  
  //após a leitura
  //limpa o buffer definindo como vazio as posições de memória
  for(int i=0;i<16;i++){
    buffer[i] = '\0';
  }
  
  //se a porta serial estiver ligada, realiza uma nova leitura
  while(Serial.available()){
    Serial.read();
  }
  
}

/*
funcao que recebe os dados tratados e atribui a intensidade
das cores a cada pino ligado aos contatos do led RGB
o comando recebido deverá ser composto de uma letra equivalente
a cor desejada, no caso R, G ou B, seguida da intensindade que
poderá variar de 0 a 255. Assim, por exemplo G128 equivale a
enviar ao led 50% da cor Verde.
*/
void ajustaLED(char* dados){
  
  if(dados[0]=='r' || dados[0]=='R'){
    //strtol filtra do comando somente a parte numérica 
    vermelho = strtol(dados+1,NULL,10);
    //constrain força os valores a se adequar a faixa indicada
    vermelho = constrain(vermelho,0,255);
    //grava o valor lido no pino equivalente a cor
    analogWrite(pVermelho, vermelho);
    
  }
  
  if(dados[0]=='g' || dados[0]=='G'){
    verde = strtol(dados+1,NULL,10);
    verde = constrain(verde,0,255);
    analogWrite(pVerde, verde);
  }

  if(dados[0]=='b' || dados[0]=='B'){
    azul = strtol(dados+1,NULL,10);
    azul = constrain(azul,0,255);
    analogWrite(pAzul, azul);
  }
}

void setup()
{
  //delay(2000);
  //inicia a comunicação serial com o computador através do
  //cabo USB
  Serial.begin(9600);
  
  while(Serial.available()){
    Serial.read();
  }
  
  //definição dos pinos PWM das cores como saída para o led RGB
  pinMode(pVerde, OUTPUT);
  pinMode(pAzul, OUTPUT);
  pinMode(pVermelho, OUTPUT);
}

void loop()
{
  
  //caso a comunicação serial estiver ativa inicia a leitura
  if(Serial.available()>0){
    int i=0;
    delay(100);//tempo para encher o buffer
    int tamanho = Serial.available();
    //corta a leitura em 15 caracteres para evitar estouro
    //do buffer
    if(tamanho>15){
      tamanho=15;
    }
    /*
    A comunicação serial é feita caracter a caracter. 
    Assim, cada caracter é inserido sequencialmente no buffer
    */
    while(tamanho--){
      buffer[i++]=Serial.read();
    }
    //com o buffer já carregado, seu conteúdo será tratado
    //pela função tratarComando
    tratarComando(buffer);
  }
}

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256
static char receive[BUFFER_LENGTH];


int main(){
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];

   printf("Ferramenta para comunicacao com o Arduino na porta serial\n\n");

   printf("Escreva o código no seguinte formato: R[0-255],G[0-255],B[0-255]\n\n");

   printf("Exemplo de cores: \n");

   printf("Vermelho: R255,G0,B0\n");
   printf("Amarelo: R255,G255,B0\n");
   printf("Verde: R0,G255,B0\n");
   printf("#####################\n\n");


   fd = open("/dev/meudriver_char", O_RDWR);
   if (fd < 0){
      perror("Erro ao abrir dispositivo!");
      return errno;
   }

   while(1){
      printf("Digite o codigo: ");
      scanf("%s", stringToSend);
      printf("Enviando codigo RGB para o Arduino: %s.\n", stringToSend);
      ret = write(fd, stringToSend, strlen(stringToSend));
      if (ret < 0){
         perror("Falha ao escrever mensagem no driver.");
         return errno;
      }

   }

   return 0;
}

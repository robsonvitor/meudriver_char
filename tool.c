/**
 * @file   testebbchar.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the ebbchar.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/ebbchar.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM


int main(){
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   printf("Ferramenta para escrever em ttyACM0\n");
   fd = open("/dev/meudriver_char", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   while(1){
      printf("Digite o codigo: ");
      scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)
      printf("Escrevendo mensagem na porta serial: [%s].\n", stringToSend);
      ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
      if (ret < 0){
         perror("Falha ao escrever mensagem no driver.");
         return errno;
      }

      // printf("Press ENTER to read back from the device...\n");
      // getchar();

      // printf("Reading from the device...\n");
      // ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
      // if (ret < 0){
      //    perror("Failed to read the message from the device.");
      //    return errno;
      // }
      // printf("The received message is: [%s]\n", receive);
   }

   printf("End of the program\n");
   return 0;
}

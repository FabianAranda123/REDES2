#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include"func.h"



int main ()
{
	/////////////////////////////////////////// CREACION DEL SERVIDOR MASTER ///////////////////////////////////////////

	int canal;             //Canal de comunicacion
	char server_addr[15];  //Direccion del servidor master
	int server_port;       //Puerto del servidor master
	int s;
	struct sockaddr_in client_info;   //Informacion del cliente 
	int clien_info_len;
	char flag[] = "f";

	strcpy(server_addr, "127.0.0.1");  //Direccion del servidor master
	server_port = 1111;                //Puerto del servidor master

	if((s = createServer(server_addr, server_port)) < 0 ) //Creando un servidor y devolviendo un canal de comunicacion con el cliente
	{
		printf("Error al creal el Master Server\n");
		return -1;
	}
	//listen()
	if((listen(s, 1)) < 0)   //Esperando conexiones
	{
		printf("Error en listen() del server\n");
		return -1;
	}
	printf("Server creado\n");

	

	////////////////////////////////////////// ETAPA DE COMUNICACIONES  ////////////////////////////////////////////////

	#define B_INI 1        //Byte inicial de la trama
	#define TAM_DATA 4     //Tamano del contenido de datos
	#define NAME_FILE 30   //Bytes que ocntienen el nombre del archivo que se desea guardar o recuperar de los servidores
	#define DATA 100       //Bytes que contienen los datos del archivo que se desea guardar o recuperar de los servidores

	char trama[B_INI + TAM_DATA + NAME_FILE + DATA];  //Cadena donde se almacena la trama recibida
	char byte_ini;

	while(1)  //Ciclo que permite estar siempre esperando peticiones del cliente
	{
		printf("\n\nEsperando conexiones * * *\n");
		//accept()
		clien_info_len = sizeof(client_info);
		if((canal = accept(s, (struct sockaddr *)&client_info, &clien_info_len)) < 0)  //Aceptando conexiones y creando el canal de comunicacion
		{
			printf("Error en accept() del server\n");
			return -1;
		}

	printf("Se ha conectado el cliente con la ip: %s, y el puerto %d\n", inet_ntoa(client_info.sin_addr), htons(client_info.sin_port));

	
		trama[0] = 10;
		recv(canal, trama, sizeof(trama), 0); //Recibimos la trama
		send(canal, &flag, sizeof(flag), 0);
		memcpy(&byte_ini, &trama[0], 1);      //Obtenemos el primer byte

		if((byte_ini == '1') || (byte_ini == '0')) //Cliente nos envia un archivo
		{
			saveFile(canal, trama); //Guardando archivo y dividiendolo entre los 3 servidores
		}
		else if(byte_ini == '2') //Cliente quiere un archivo
		{
			getFile(canal, trama); //Recuperando archivo de los 3 servidores
		}
		else
		{
			printf("Solicitud no valida\n");
		}
		//unsigned int microseconds = 2;
		//usleep(microseconds);
		close(canal);

	}




	return 0;
}

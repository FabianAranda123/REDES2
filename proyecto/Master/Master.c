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

////Definicion de la trama//////////
#define B_INI 1        //Byte inicial de la trama
#define TAM_DATA 4     //Tamano del contenido de datos
#define NAME_FILE 30   //Bytes que ocntienen el nombre del archivo que se desea guardar o recuperar de los servidores
#define DATA 100       //Bytes que contienen los datos del archivo que se desea guardar o recuperar de los servidores
#define SERV_ADDR "127.0.0.1"



int main()
{

	///////////////////////////////////////////////////// CREACION DE SERVIDOR /////////////////////////////////////////////////////////////////

	int canal;                         //Canal de comunicacion
	char server_addr[15];              //Direccion del servidor master
	int server_port;                   //Puerto del servidor master
	int s;                             //Descriptor del socket
	struct sockaddr_in client_info;    //Informacion del cliente 
	struct sockaddr_in server_info;    //Informacion del server Master
	int clien_info_len;                //Tamano de la estructura del cliente
	char flag = 'f';

	strcpy(server_addr, SERV_ADDR);  //Direccion del servidor master
	server_port = 1111;                //Puerto del servidor master

	//Lenando la estructura con los datos del servidor master
	bzero((char*)&server_info, sizeof(server_info));  //Llenando estructura de ceros
	server_info.sin_family = AF_INET;                 //Tipo de socket es para conexion en internet
	server_info.sin_port = htons(server_port);               //Puerto del servidor master
	server_info.sin_addr.s_addr = inet_addr(server_addr);     //Direccion del servidor master

	//socket()
	if((s = socket(AF_INET, SOCK_STREAM, 0))<0) //Creacion del socket de tipo SOCK_STREAM (Orientado a conexion)
	{
		printf("Error en socket() del server\n");
		return -1;
	}

	//bind() 
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));      //Forzando a usar la mima direccion 
	if(bind(s, (struct sockaddr*)&server_info, sizeof(server_info)) < 0)  //Ligando una direccion y puerto al socket
	{
		printf("Error en bind() del server\n");
		return -1;
	}
	//listen()
	if((listen(s, 100)) < 0)   //Esperando conexiones
	{
		printf("Error en listen() del server\n");
		return -1;
	}
	printf("Server creado\n");

	////////////////////////////////////////// ETAPA DE COMUNICACIONES  ////////////////////////////////////////////////

	while(1) // Ciclo que permite esperar petticiones de clientes
	{
		printf("\n\nEsperando conexiones * * *\n");
		//accept()
		clien_info_len = sizeof(client_info);
		if((canal = accept(s, (struct sockaddr *)&client_info, &clien_info_len)) < 0)  //Aceptando conexiones y creando el canal de comunicacion
		{
			printf("Error en accept() del server\n");
			return -1;
		}
		system("clear");
		printf("Se ha conectado el cliente con la ip: %s, y el puerto %d\n", inet_ntoa(client_info.sin_addr), htons(client_info.sin_port));

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/////////Comunicacion con el cliente/////////////////////
		char trama[B_INI + TAM_DATA + NAME_FILE + DATA];  //Cadena donde se almacena la trama recibida

		recv(canal, trama, sizeof(trama), 0);  //Recibimos la trama
		send(canal, &flag, sizeof(char), 0);   //Enviamos confirmacion
		recv(canal, &flag, sizeof(char), 0);

		if((trama[0] == '1') || (trama[0] == '0')) //Cliente nos envia un archivo
		{
			saveFile(canal, trama); //Guardando archivo y dividiendolo entre los 3 servidores
		}
		else if(trama[0] == '2') //Cliente quiere un archivo
		{
			getFile(canal, trama); //Recuperando archivo de los 3 servidores
		}
		else
		{
			printf("Solicitud no valida\n");
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



		close(canal); //Cerramos el canal de comunicaciones para esperar otra peticion
	}
	return 0;
}

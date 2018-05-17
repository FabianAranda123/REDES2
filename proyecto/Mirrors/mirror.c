
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

#include"FuncionesM.h"

#define B_INI 1        //Byte inicial de la trama
#define TAM_DATA 4     //Tamano del contenido de datos
#define NAME_FILE 30   //Bytes que contienen el nombre del archivo que se desea guardar o recuperar de los servidores
#define DATA 100       //Bytes que contienen los datos del archivo que se desea guardar o recuperar de los servidores


int main(int argc, char* argv[]){
	
	int id_mirror, tamFile;
	char mirror_addr[15];   //Direccion de algun worker para realizar la conexion
	int mirror_port;	    //Puerto de algun worker para realizar la conexion

	int canal;             //Canal de comunicacion
	char server_addr[15];  //Direccion del servidor worker
	int server_port;       //Puerto del servidor master
	char trama[B_INI + TAM_DATA + NAME_FILE + DATA];  //Cadena donde se almacena la trama recibida
	char byte_ini;
	struct sockaddr_in client_info;   //Informacion del cliente 
	int clien_info_len, s; 
	char cad ='a';
	
	
	
	if(argc < 3){
		printf( "                      " );
		printf( "Faltan argumentos: [ip] [puerto]" );
		exit(0);
	}
	
	/////CREACION DEL SERVIDOR PARA ESPERAR CONEXIÓN CON EL MASTER O CON EL WORKER//////////
	strcpy(server_addr, argv[1]);  //Direccion del servidor master
	server_port = atoi(argv[2]);                //Puerto del servidor master

	if((s = createServer(server_addr, server_port)) < 0 ) //Creando un servidor y devolviendo un canal de comunicacion con el cliente
	{
		printf("Error al creal el mirror-Server\n");
		return -1;
	}
	

	/////////////////////////////////// ETAPA DE COMUNICACIONES ///////////////////////////////////////////////////////////////////////

	
	while(1)  //Ciclo que permite estar siempre esperando peticiones del cliente
	{
		
		///////////////////////////////////////////////////////////////////////////////////////
		printf("\nServidor-Mirror creado\nEsperando conexiones ...\n");

		//accept()
		clien_info_len = sizeof(client_info);
		if((canal = accept(s, (struct sockaddr *)&client_info, &clien_info_len)) < 0)  //Aceptando conexiones y creando el canal de comunicacion
		{
			printf("Error en accept() del server\n");
			return -1;
		}
		printf("Se ha conectado el worker con la ip: %s, y el puerto %d\n", inet_ntoa(client_info.sin_addr), htons(client_info.sin_port));

		//////////////////////////////////////////////////////////////////////////////////////
		
		recv(canal, trama, sizeof(trama), 0); //Recibimos la trama
		send(canal, &cad, sizeof(char), 0); //Recibimos la trama
		//printf("El mirror recibio la primera trama\n");
		memcpy(&byte_ini, &trama[0], 1);      //Obtenemos el primer byte

		if((byte_ini == '1') || (byte_ini == '0')) //Cliente(Master) nos envia un archivo
		{
			printf("Opción 1\n");
			saveFile(canal,trama); //Guardando archivo y dividiendolo entre los 3 workers
			
		}
		else if(byte_ini == '2') //Master solicita un archivo
		{
			printf("Opción 2\n");
			getFile(canal,trama); //Recuperando parte del archivo del mirror
		}
		else
		{
			printf("Solicitud no valida\n");
		}
		close(canal);

	}


	return 0;
}

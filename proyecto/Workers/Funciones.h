

//Función que establece la conexión con los mirrors
int mirrorConection(char dir[], int port)
{

	int s;                           //Id del socket
	struct sockaddr_in mirror_info;  //Estructura con informacion del worker
	//socket()
	if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)  //CReando el socket para conectarnos
	{
		printf("Error en socket()\n");
		return -1;
	}

	//Llenando la estructura con los datos del worker 
	bzero((char*)&mirror_info, sizeof(mirror_info));  //Llenando estructura de ceros
	mirror_info.sin_family = AF_INET;                 //Diciendo que el tipo de conexion es de internet
	mirror_info.sin_port = htons(port);               //Puerto del worker al cual nos queremos conectar
	mirror_info.sin_addr.s_addr = inet_addr(dir);     //Direccion del worker al cual nos queremos conectar

	//connect()
	if((connect(s, (struct sockaddr*)&mirror_info, sizeof(mirror_info))) < 0) //intentando hacer la conexion con el socket
	{
		printf("Error en connect\n");
		return -1;
	}
	return s; //Conexion con el espejo exitosa

}

///////////////Funcion que crea el servidor y devuelve una conexion con el cliente que nos envia archivos///////////////////////////

int createServer(char dir[], int port)
{
	struct sockaddr_in server_info;   //Informacion del server Worker
	struct sockaddr_in client_info;   //Informacion del cliente 
	int s;                            //Identificador del socckket
	int canal;                        //Identificador del canal de comunicaciones
	int clien_info_len;               //Tamano de la estructura de la informacion del cliente

	//Lenando la estructura con los datos del servidor master
	bzero((char*)&server_info, sizeof(server_info));  //Llenando estructura de ceros
	server_info.sin_family = AF_INET;                 //Tipo de socket es para conexion en internet
	server_info.sin_port = htons(port);               //Puerto del servidor master
	server_info.sin_addr.s_addr = inet_addr(dir);     //Direccion del servidor master

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
	if((listen(s, 1)) < 0)   //Esperando conecxiones
	{
		printf("Error en listen() del server\n");
		return -1;
	}

	return s;

}


/////////Funcion que recibe parte del archivo que le corresponda guardar//////////////////////////////
int saveFile(int canal, char trama[])
{
	char datarecv[135];
	char b_ini;         //byte inicial que indica si es la ultima trama o no
	int fileSize;       //Tamano de los datos que se esta recibiendo
	char fileName[30];  //Nombre del archivo que se esta recibindo
	char fileData[100]; //Datos del archivo que se recibe
	FILE *fp;			//Apuntador al archivo para guardar los dtaos y leerlos
	int bytes_read;
	char cad='a';

	//Obtenemos los datos de la primer trama enviada
	memcpy(&b_ini, &trama[0], 1);       //Byte inicial 0 si es la ultima trama o 1 si aun faltan tramas por recibir
	memcpy(&fileSize, &trama[1], 4);    //Tamano de los datos que se enviaron
	memcpy(&fileName, &trama[5], 30);   //Nombre del archivo a guardar
	memcpy(&fileData, &trama[35], 100); //Datos del archivo
	
	

	fp = fopen(fileName, "w");  //Abriendo el archivo para guardar los datos que nos llegan;
	while(b_ini == '1')         //Minetras no sea la trama final
	{
		fwrite(&fileData, 1, fileSize, fp);          //Escribimos los datos en el archivo
					
		//recibimos la trama para guardar el archivo y envimos al mirror 
		recv(canal, datarecv, sizeof(datarecv), 0);  //Recibimos una nueva trama
		send(canal, &cad, sizeof(char), 0);
		
		//send(id_mirror, datarecv, sizeof(datarecv), 0);	
		//recv(id_mirror, &cad, sizeof(char), 0);
		//Obtenemos los datos de la primer trama enviada
		memcpy(&b_ini, &datarecv[0], 1);       //Byte inicial 0 si es la ultima trama o 1 si aun faltan tramas por recibir
		memcpy(&fileSize, &datarecv[1], 4);    //Tamano de los datos que se enviaron
		memcpy(&fileName, &datarecv[5], 30);   //Nombre del archivo a guardar
		memcpy(&fileData, &datarecv[35], 100); //Datos del archivo
		
     //Datos del archivo
	}
	//memcpy(&b_ini, &datarecv[0], 0);
	fwrite(&fileData, 1, fileSize, fp);  //Escribimos los datos de la ultima trama
	//send(id_mirror, datarecv, sizeof(datarecv), 0); //Se envía la última trama
	
	fclose(fp);//Cerramos el archivo


	
	
}



//Función que obtiene el archivo que se le ha solicitado
void getFile(int canal, char trama[])
{
	char dataSend[135];  //Trama para enviar y recibir datos
	char b_ini;         //byte inicial que indica si es la ultima trama o no
	int fileSize;       //Tamano de los datos que se esta recibiendo
	char fileName[30];  //Nombre del archivo que se esta recibindo
	char fileData[100]; //Datos del archivo que se recibe
	
	FILE *fp;
	int bytes_read = 100,tamBloque , i=0;
	long lSize;
	char cad;

	memcpy(&fileName, &trama[5], 30);	
	fp = fopen(fileName, "r");

	fseek (fp , 0 , SEEK_END);
  	lSize = ftell (fp);
  	printf("\n %d ", lSize);
 	rewind (fp);
 	
 	tamBloque = lSize/100;
 	
 	
 	//////////Emepzaos a construir las tramas para enviarlas
 	while(i < tamBloque)
 	{
 		b_ini = '1';
 		fread(fileData, 1, 100, fp);
		memcpy(&dataSend[0], &b_ini, 1);
		memcpy(&dataSend[1], &bytes_read, 4);
		memcpy(&dataSend[5], &fileName, 30);
		memcpy(&dataSend[35], &fileData, 100);
		send(canal, dataSend, sizeof(dataSend), 0);	
		recv(canal, &cad, sizeof(char), 0);	
		i++;
 	}
 	bytes_read = lSize % 100;
 	if(bytes_read != 0)
 	{
 		b_ini = '0';
 		fread(fileData, 1, bytes_read, fp);
 		
		memcpy(&dataSend[0], &b_ini, 1);
		memcpy(&dataSend[1], &bytes_read, 4);
		memcpy(&dataSend[5], &fileName, 30);
		memcpy(&dataSend[35], &fileData, 100);
		send(canal, dataSend, sizeof(dataSend), 0);
		recv(canal, &cad, sizeof(char), 0);	
 	
 	}
 	fclose(fp);
 
}





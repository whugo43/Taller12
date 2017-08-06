#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define MAX 1000000
#define MAXPALABRASBUSCAR 100
pthread_mutex_t mutex; 
int num_palabras[MAXPALABRASBUSCAR];
char *palabras[MAXPALABRASBUSCAR];
int numero_palabras=0;
//retorna el numero de lineas del archivo y guarda en tam_lineas el numero de caracteres de cada linea
int numero_lineas (char *ruta, int *tam_lineas){
	if (ruta != NULL){
		FILE* ar= fopen (ruta, "r");
		int lineas = 0;
		int tam_linea=0;
		while (!feof(ar)){
			tam_linea++;
			char c = getc(ar);
			if (c== '\n'){
				if (tam_lineas != NULL) {
					tam_lineas[lineas] = tam_linea;
				}
				lineas++;
				tam_linea =0;
			}
		}
		fclose (ar);
		return lineas;
	}
	return -1;
}

//estructura con la informacion necesario a enviar a cada hilo creado
typedef struct mi_estructuraTDA{
	int iniciocaracter;
	int inilinea;
	int finlinea;
	FILE * fp;
	int *tam_lineas;
} estructura;

int main(int argc, char *argv[]){
	if(argc>3){
		numero_palabras=argc-3;  //numero de palabras a buscar  
		//validando ruta de archivo
		char* pathfile=argv[1];
		FILE *fp = fopen(pathfile,"r");
		if(fp==NULL){ 
			printf("direccion invalida de archivo");
			return -1;
		}
		//validando numero de hilos
		int num_hilos=atoi(argv[2]);  //numero de hilos a utilizar para la busqueda
		if(num_hilos<1){
			printf("numero de hijo debe ser mayor o igual a 1");
			return -1;
		}
		
		for(int i=0;i<numero_palabras;i++){
			palabras[i]=argv[3+i];
			num_palabras[i]=0;
		}

		int *tam_lineas = (int *)malloc(MAX*sizeof(int));
		int totallineas = numero_lineas(pathfile,tam_lineas);
		printf(" total lineas de archivo: %d \n",totallineas);

		//determinando cantidad de caracteres y la seccion para cada hilo
		int totalcaracteres=0;
		for(int i=0;i<totallineas;i++)
      		totalcaracteres+=tam_lineas[i];
      	printf("total caracteres: %d \n", totalcaracteres);

      	int residuo=0,tamanobloque;
		pthread_t *hilos =malloc(num_hilos*sizeof(pthread_t));
		
		//validar tamano de bloque y determinando informacion a enviar en cada
		if(totalcaracteres%num_hilos!=0){
			tamanobloque=totalcaracteres/num_hilos;
			residuo=totalcaracteres%num_hilos;
		}
		tamanobloque=(totalcaracteres/num_hilos)+residuo;
		
		
	}else{
		printf("necesita enviar mas parametros: ./buscar dir_archivo num_hilos palabra1 palabra2..... palabraN.");
	}
}

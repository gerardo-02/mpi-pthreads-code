#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Vectores globales accesibles desde cualquier función
int *vec1;
int *vec2;

// Parámetros enviados a cada hebra. Rango del primer vector que tiene que
// calcular y el número de elementos del segundo vector necesario
typedef struct t_struct{
	int primer_elemento;
	int ultimo_elemento;
	int elementos_v2;
} rango;

// Función que realiza cada hebra. Recorre el rango del primer vector elemento
// por elemento y comprueba si se encuentra en el segundo vector. Si es así, el
// cardinal incrementa en 1. La variable devuelta es de tipo double aunque
// siempre es un número entero para poder luego calcular la distancia de jaccard,
// que si es un número real.
void* body(void* param){
	rango *MiRango = (rango*) param;
	double *card_interseccion = malloc(sizeof(double));
	*card_interseccion = 0.0;
	int encontrado;
	for (int i = MiRango->primer_elemento; i < MiRango->ultimo_elemento; i++){
		encontrado = 0;
		for (int j=0; j < MiRango->elementos_v2 && encontrado == 0; j++){
			if (vec1[i] == vec2[j]){
				*card_interseccion = *card_interseccion + 1;
				encontrado = 1;
			}
		}

	}
	return card_interseccion;
}

int main(int argc, char* argv[]){

	// Declaración de variables y asignación de valores solicitando por consola
	int NUM_ELEMENTOS_V1, NUM_ELEMENTOS_V2, NUM_THREADS, cociente, resto;
	double distancia_jaccard = 0.0;
	double card_union=0.0, card_interseccion=0.0;

	printf("Introduzca el número de elementos del vector 1:\n");
	do
		scanf("%d", &NUM_ELEMENTOS_V1);
	while (NUM_ELEMENTOS_V1 < 1);

	printf("Introduzca el número de elementos del vector 2:\n");
	do
		scanf("%d", &NUM_ELEMENTOS_V2);
	while (NUM_ELEMENTOS_V2 < 1);
	printf("Introduzca el número de hebras que se van a utilizar:\n");
	do
		scanf("%d", &NUM_THREADS);
	while (NUM_THREADS < 1);

	vec1 = (int*)malloc(NUM_ELEMENTOS_V1 * sizeof(int));
	vec2 = (int*)malloc(NUM_ELEMENTOS_V2 * sizeof(int));

	if (vec1 == NULL || vec2 == NULL ){
		printf("Memoria no reservada\n");
		exit(0);
	}

	for (int i=0; i<NUM_ELEMENTOS_V1; i++){
		vec1[i] = i;
	}
	for (int i=0; i<NUM_ELEMENTOS_V2; i++){
		vec2[i] = 2*i;
	}

	// Asignación de los rangos del primer vector de cada thread
	struct t_struct rangos[NUM_THREADS];
	cociente = NUM_ELEMENTOS_V1 / NUM_THREADS;
	resto = NUM_ELEMENTOS_V1 % NUM_THREADS;

	for (int i=0; i<NUM_THREADS; i++){
		rangos[i].elementos_v2 = NUM_ELEMENTOS_V2;
		rangos[i].primer_elemento = i * cociente;
		if (i == NUM_THREADS-1)
			rangos[i].ultimo_elemento = rangos[i].primer_elemento + cociente + resto;
		else
			rangos[i].ultimo_elemento = rangos[i].primer_elemento + cociente;

		//printf("Hebra %d\t Rangos: Primero %d Ultimo %d\n", i, rangos[i].primer_elemento, rangos[i].ultimo_elemento);
	}

	// Ejecución de las hebras
	pthread_t threads[NUM_THREADS];

	for(long int i = 0; i<NUM_THREADS; i++){
		pthread_create(&threads[i], 0, body, &(rangos[i]));
	}

	// Recepción de los resultados
	void* buffer = 0;
	for(long int i = 0; i<NUM_THREADS; i++){
		pthread_join(threads[i], &buffer);
		card_interseccion += *((double*) buffer);
		free(buffer);
	}

	// Calculo final
	card_union = NUM_ELEMENTOS_V1 + NUM_ELEMENTOS_V2 - card_interseccion;

	distancia_jaccard = card_interseccion / card_union;
	printf("La distancia de jaccard es %f\n", distancia_jaccard);

	pthread_exit(0);

}

#include "csv.h"
#include <stdio.h>
#include <string.h>

struct archivo_csv {
	size_t cant_lineas;
	size_t linea_actual;
	char separador;
	char **lineas;
};

struct archivo_csv *abrir_archivo_csv(const char *nombre_archivo,
				      char separador)
{
	FILE *archivo_pedido = fopen(nombre_archivo, "r");
	if (archivo_pedido == NULL) {
		return NULL;
	}
	char caracter_actual;
	size_t cantidad_caracteres_total = 0;
	size_t cantidad_caracteres_linea = 0;
	struct archivo_csv *archivo_procesado =
		malloc(sizeof(struct archivo_csv));
	archivo_procesado->cant_lineas = 0;
	archivo_procesado->linea_actual = 0;
	archivo_procesado->separador = separador;
	archivo_procesado->lineas = malloc(sizeof(char *));
	while ((caracter_actual = (char)fgetc(archivo_pedido)) != EOF) {
		cantidad_caracteres_total++;
		cantidad_caracteres_linea++;
		if (caracter_actual == '\n') {
			archivo_procesado->cant_lineas++;
			archivo_procesado->lineas =
				realloc(archivo_procesado->lineas,
					sizeof(char *) *
						archivo_procesado->cant_lineas);
			if (archivo_procesado->lineas == NULL) {
				return NULL;
			}
			archivo_procesado
				->lineas[archivo_procesado->cant_lineas - 1] =
				malloc(sizeof(char) *
				       (cantidad_caracteres_linea + 1));
			if (archivo_procesado
				    ->lineas[archivo_procesado->cant_lineas -
					     1] == NULL) {
				return NULL;
			}
			int moverse = fseek(archivo_pedido,
					    (long)(cantidad_caracteres_total -
						   cantidad_caracteres_linea),
					    SEEK_SET);
			if (moverse == -1) {
				return NULL;
			}
			archivo_procesado
				->lineas[archivo_procesado->cant_lineas - 1] =
				fgets(archivo_procesado->lineas
					      [archivo_procesado->cant_lineas -
					       1],
				      (int)cantidad_caracteres_linea + 1,
				      archivo_pedido);
			if (archivo_procesado
				    ->lineas[archivo_procesado->cant_lineas -
					     1] == NULL) {
				return NULL;
			}
			cantidad_caracteres_linea = 0;
		}
	}
	int cerrar_archivo = fclose(archivo_pedido);
	if (!(cerrar_archivo == 0)) {
		return NULL;
	}
	return archivo_procesado;
}

/* Dado un string, el separador y un numero size_t que representa el numero de
separador en el que se encuentra un bucle devuelve el numero de la aparicion de
ese separador anterior en ese string o -1 (en caso de buscar el anterior 
separador a la primera aparicion del separador, se devuelve 0) */
long int encontrar_separador_anterior(const char *string, const char separador,
				      size_t separador_actual)
{
	if (separador_actual == 1) {
		return 0;
	}
	long int ii = 0;
	long int encuentros_separador = 0;
	for (ii = 0; ii < strlen(string); ii++) {
		if ((string[ii] == separador) || (string[ii] == '\n') ||
		    (string[ii] == '\0') || (string[ii] == EOF)) {
			encuentros_separador++;
			if (encuentros_separador == (separador_actual - 1)) {
				return ii + 1;
			}
		}
	}
	return -1;
}

size_t leer_linea_csv(struct archivo_csv *archivo, size_t columnas,
		      bool (*funciones[])(const char *, void *), void *ctx[])
{
	if (archivo == NULL) {
		return 0;
	}
	if (!(archivo->linea_actual < archivo->cant_lineas)) {
		return 0;
	}
	size_t ii = 0;
	size_t columnas_leidas = 0;
	for (ii = 0; ii < columnas; ii++) {
		long int jj = 0;
		long int encuentros_separador = -1;
		for (jj = 0;
		     jj < strlen(archivo->lineas[archivo->linea_actual]);
		     jj++) {
			if ((archivo->lineas[archivo->linea_actual][jj] ==
			     archivo->separador) ||
			    (archivo->lineas[archivo->linea_actual][jj] ==
			     '\0') ||
			    (archivo->lineas[archivo->linea_actual][jj] ==
			     '\n') ||
			    (archivo->lineas[archivo->linea_actual][jj] ==
			     EOF)) {
				encuentros_separador++;
				if (encuentros_separador == ii) {
					long int separador_anterior =
						encontrar_separador_anterior(
							archivo->lineas
								[archivo->linea_actual],
							archivo->separador,
							ii + 1);
					if (separador_anterior == -1) {
						return 0;
					}
					char palabra[jj - separador_anterior];
					strncpy(palabra,
						&(archivo->lineas
							  [archivo->linea_actual]
							  [separador_anterior]),
						(size_t)(jj -
							 separador_anterior));
					palabra[jj - separador_anterior] = '\0';
					if (funciones[ii] != NULL) {
						bool funcion_funciona =
							funciones[ii](palabra,
								      ctx[ii]);
						if (funcion_funciona) {
							columnas_leidas++;
						}
					}
				}
			}
		}
	}
	archivo->linea_actual++;
	return columnas_leidas;
}

void cerrar_archivo_csv(struct archivo_csv *archivo)
{
	if (archivo != NULL) {
		int ii = 0;
		for (ii = 0; ii < archivo->cant_lineas; ii++) {
			free(archivo->lineas[ii]);
		}
		free(archivo->lineas);
		free(archivo);
	}
}

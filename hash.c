#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#define FACTOR_CARGA 0.40
#define CAPACIDAD_INICIAL 181

//Defino estructuras
typedef enum{VACIO,BORRADO,OCUPADO} estado_t;

typedef struct tabla_hash{
    estado_t estado;
    char* clave;
    void* dato;
}tabla_hash_t;

struct hash{
    size_t cantidad;
    size_t capacidad;
    size_t cant_borrados;
    size_t cant_ocupados;
    hash_destruir_dato_t destruir_dato;
    tabla_hash_t* tabla;
    
};

struct hash_iter{
    hash_t* hash;
    tabla_hash_t* act;
    tabla_hash_t* ant;
};
//===============================Funcion de Hash===================================
 /*static unsigned long funcion_hash(const char *str)
    {
        unsigned long hash = 0;
        int c = 0;

        while (c == *str++)
            hash = c + (hash << 6) + (hash << 16) - hash;

        return hash;
    }*/
long funcion_hash(const char *clave){
	long p=0;
	int i;
	for(i=0; i<strlen(clave); i++){
		p=p*31+clave[i];
	}
	p=abs(p);
	return p;
}
//===============================Funciones auxiliares===============================
bool es_primo(long int numero) {
  // Casos especiales
  if (numero == 0 || numero == 1 || numero == 4) return false;
  for (int x = 2; x < numero / 2; x++) {
    if (numero % x == 0) return false;
  }
  // Si no se pudo dividir por ninguno de los de arriba, sí es primo
  return true;
}

size_t _es_primo(long int numero){
    while(!es_primo(numero)){
        numero ++;
    }

    return numero;
}
//Creo tabla
tabla_hash_t* crear_tabla(hash_t* hash){
    tabla_hash_t* tabla = malloc(sizeof(tabla_hash_t)*CAPACIDAD_INICIAL);
    if(!tabla) return NULL;
    return tabla;
}
//Inicializo los campos de la tabla.
void iniciar_tabla(hash_t* hash){
    for(size_t i = 0; i < hash -> capacidad;i++){
        hash -> tabla[i].estado = VACIO;
        hash -> tabla[i].clave = NULL;
        hash -> tabla[i].dato = NULL;
    }
}

//Funcion para llenar campos de la tabla
void llenar_campo(hash_t* hash,const char* clave,size_t indice,void* dato){
    char* aux = strdup(clave);
    hash -> tabla[indice].clave = aux;
    hash -> tabla[indice].dato = dato;
    hash -> tabla[indice].estado = OCUPADO;
    //free(clave);
}

//Si pongo un dato distinto en una clave que ya existe,uso esta funcion.
void renovar_campo(hash_t* hash,void* dato,size_t indice){
    hash -> destruir_dato(hash -> tabla[indice].dato);
    hash -> tabla[indice].dato = dato;

}

//Funcion para buscar un lugar vacio en la tabla e insertar elemento.
size_t buscar_lugar(hash_t* hash,const char* clave,size_t indice,void* dato,bool insertar){
    indice += 1;
    size_t contador = 0;
    while(hash -> tabla[indice].estado == OCUPADO){
        if (hash -> tabla[indice].estado == OCUPADO && !strcmp(hash -> tabla[indice].clave,clave)){
            renovar_campo(hash,dato,indice);
            break;
        }
        indice ++;
        if(indice == hash -> capacidad -1) indice = 0;
        if (contador == hash -> capacidad -1) return -1;
    }
    if(insertar) llenar_campo(hash,clave,indice,dato);
    return indice;
}

//Funcion para buscar clave en la tabla 
//Devuelvo true si encuentro la clave
//Devuelvo false si determino que  la clave no esta
long int hash_buscar(const hash_t* hash,const char* clave){
    long int indice = funcion_hash(clave) % hash -> capacidad;
    long int contador;
    //Si la primer posicion que verifica es falsa,la clave no esta
    if(hash -> tabla[indice].estado == VACIO) return -1;
    else{
        for(contador = 0;contador < hash -> capacidad;contador ++){
            //Si esta ocupado y no es la clave que busco,continuo con la iteracion
            if ((hash -> tabla[indice].estado == OCUPADO && strcmp(hash -> tabla[indice].clave,clave)) || (hash -> tabla[indice].estado == BORRADO )) continue;
            //Si hay un lugar vacio,la clave no esta
            else if (hash -> tabla[indice].estado == VACIO) return -1;
            //Si una clave coincide,devuelvo que esta
            else if (!strcmp( hash -> tabla[indice].clave , clave)) return indice;
            
            if (indice == hash -> capacidad -1) {
                indice = 0;
                continue;
            }
            indice ++;
            
        }

    }

    return -1;



}
//Funcion para saber si es necesario redimensionar
bool densidad_tabla(hash_t* hash){
    float densidad = (float)(hash -> cant_ocupados + hash -> cant_borrados) / (float)hash -> capacidad;
    if (densidad > FACTOR_CARGA) return true;
    else return false;
}
//Funcion para sacar indice 
size_t hashing(hash_t* hash,const char* clave){
    size_t indice = funcion_hash(clave) % hash -> capacidad;
    return indice;
}
//Funcion para redimensionar hash
bool redimensionar_hash(hash_t* hash){
    tabla_hash_t* tabla_aux = hash -> tabla;
    size_t capacidad_aux = hash -> capacidad;
    tabla_hash_t* nueva_tabla = crear_tabla(hash);
    if(!nueva_tabla) return false;
    //Creo una capacidad nueva,cuyo valor sea un numero primo para tener menor probabilidad de colision
    size_t nueva_capacidad = (size_t)_es_primo((long int) hash -> capacidad*2);
    hash -> capacidad = nueva_capacidad;
    
    for(size_t i = 0;i < capacidad_aux;i++){
        if(tabla_aux[i].estado == OCUPADO){
            hash_guardar(hash,tabla_aux[i].clave,tabla_aux[i].dato);
            free(tabla_aux[i].clave);
        }

    }
    free(tabla_aux);
    return true;

    
}



//===========================Primitivas=============================


hash_t* hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;
    hash -> cantidad = 0;
    hash -> capacidad = CAPACIDAD_INICIAL;
    hash -> cant_borrados = 0;
    hash -> cant_ocupados = 0;
    hash -> destruir_dato = destruir_dato;
    hash -> tabla = crear_tabla(hash);
    iniciar_tabla(hash);
    
    return hash;
}

bool hash_guardar(hash_t* hash,const char* clave,void* dato){
    //Veo si es necesario redimensionar la tabla
    if(densidad_tabla(hash)){
        if(!redimensionar_hash(hash)) return false;
    }

    size_t indice = hashing(hash,clave);
    //Si la clave esta,renuevo el dato
    if (hash -> tabla[indice].estado == OCUPADO && !strcmp(hash -> tabla[indice].clave,clave)) renovar_campo(hash,dato,indice);
    //Si el indice de la tabla esta ocupado,busco lugar
    else if (hash -> tabla[indice].estado == OCUPADO){
        if (buscar_lugar(hash,clave,indice,dato,true) == -1) return false;
        hash -> cant_ocupados ++;
        hash -> cantidad ++;
    }
    //Si el indice en la tabla esta vacio,pongo la clave y el dato
    else if(hash -> tabla[indice].estado == VACIO || hash -> tabla[indice].estado == BORRADO){
        llenar_campo(hash,clave,indice,dato);
        hash -> cant_ocupados ++;
        hash -> cantidad ++;

    }
    return true;
}


void* hash_borrar(hash_t *hash, const char *clave){

    size_t indice =  hash_buscar(hash,clave);
    if (indice == -1) return NULL;
    void* aux = hash -> tabla[indice].dato;
    hash -> tabla[indice].clave = NULL;
    hash -> tabla[indice].dato = NULL;
    hash -> tabla[indice].estado = BORRADO;
    hash -> cantidad --;
    hash -> cant_ocupados --;
    hash -> cant_borrados ++;


    return aux;
}


void* hash_obtener(const hash_t *hash, const char *clave){
    size_t indice = hash_buscar(hash,clave);
    if (indice == -1) return NULL;
    return hash -> tabla[indice].dato;
}


bool hash_pertenece(const hash_t *hash, const char *clave){
    size_t indice = hash_buscar(hash,clave);
    if(indice == -1) return false;

    return true;
}


size_t hash_cantidad(const hash_t *hash){
    return hash -> cantidad;
}


void hash_destruir(hash_t *hash){
    return;
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if (!iter) return NULL;
    iter -> hash = hash;
    iter ->ant = NULL;
    iter ->act = NULL;
    size_t i = 0;
    while (hash ->tabla[i].estado != VACIO || hash ->tabla[i].estado != BORRADO){
        i++;
    }
    iter -> act = &hash -> tabla[i];
    return iter;
}
// Avanza iterador
bool hash_iter_avanzar(hash_iter_t *iter){
    size_t i = 0;
    while (iter -> hash ->tabla[i].estado != VACIO || iter -> hash ->tabla[i].estado != BORRADO){
        i++;
    }
    if (iter -> hash -> tabla[i].estado == VACIO || iter -> hash -> tabla[i].estado == BORRADO) return false;
    iter -> ant = iter -> act;
    iter -> act = &iter -> hash -> tabla[i];
    return true;
}
// Devuelve clave actual, esa clave no se puede modificar ni liberar.
const char *hash_iter_ver_actual(const hash_iter_t *iter){
    char* clave;
    if (iter -> act) return NULL;
    clave = malloc(sizeof(char)*strlen(iter->act->clave));
    strcpy(clave,iter->act->clave);
    return clave;
}
// Comprueba si terminó la iteración
bool hash_iter_al_final(const hash_iter_t *iter);

// Destruye iterador
void hash_iter_destruir(hash_iter_t* iter);
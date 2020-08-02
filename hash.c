#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#define FACTOR_CARGA 0.40
#define CAPACIDAD_INICIAL 1000
#define NO_ENCONTRADO -1
#define HASH_VACIO 0

//Defino estructuras
typedef enum{VACIO,BORRADO,OCUPADO} estado_t;

typedef struct campo_hash{
    estado_t estado;
    char* clave;
    void* dato;
}campo_hash_t;

struct hash{
    size_t capacidad;
    size_t cant_borrados;
    size_t cant_ocupados;
    hash_destruir_dato_t destruir_dato;
    campo_hash_t* tabla;
    
};

struct hash_iter{
    const hash_t* hash;
    size_t pos;
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
static unsigned long funcion_hash(const char *clave){
	int p=0;
	int i;
	for(i=0; i<strlen(clave); i++){
		p=p*31+clave[i];
	}
	p = abs(p);
	return p;
}
//===============================Funciones auxiliares===============================

//Creo tabla
campo_hash_t* crear_tabla(size_t capacidad){
    campo_hash_t* tabla = calloc(capacidad,sizeof(campo_hash_t));
    if(!tabla) return NULL;
    return tabla;
}
//Inicializo los campos de la tabla.
void iniciar_tabla(hash_t* hash){
    for(size_t i = 0; i < hash -> capacidad;i++){
        hash -> tabla[i].estado = VACIO;
    }
}

//Funcion para llenar campos de la tabla
void llenar_campo(const hash_t* hash,const char* clave,size_t indice,void* dato){
    char* aux = strdup(clave);
    hash -> tabla[indice].clave = aux;
    hash -> tabla[indice].dato = dato;
    hash -> tabla[indice].estado = OCUPADO;

}

//Si pongo un dato distinto en una clave que ya existe,uso esta funcion.
void renovar_campo(const hash_t* hash,void* dato,size_t indice){
    if(hash -> destruir_dato) hash -> destruir_dato(hash -> tabla[indice].dato);
    hash -> tabla[indice].dato = dato;

}

void destruir_campo(char* clave,void* dato,hash_destruir_dato_t destruir_dato){
    free(clave);
    if (destruir_dato) destruir_dato(dato);
}

//Funcion para buscar clave en la tabla 
//Devuelvo true si encuentro la clave
//Devuelvo false si determino que  la clave no esta
size_t hash_buscar(const hash_t* hash,const char* clave,bool insertar,void* dato){
    size_t indice = funcion_hash(clave);
    for(size_t contador = 0;contador < hash -> capacidad;contador ++){
        indice = (indice + contador) % hash->capacidad;
        //Si esta borrado, continuo con la busqueda
      
        //Si una clave coincide,devuelvo que esta
        if (hash -> tabla[indice].estado == OCUPADO && !strcmp( hash -> tabla[indice].clave , clave)){
            if(insertar) renovar_campo(hash,dato,indice);
            return indice;
        }
        else if(hash -> tabla[indice].estado != OCUPADO && insertar) {
            llenar_campo(hash,clave,indice,dato);
            return indice;
        }
  
    }
    return NO_ENCONTRADO;
}

//Funcion para saber si es necesario redimensionar
bool esta_sobrecargado(hash_t* hash){
    float densidad = (float)(hash -> cant_ocupados + hash -> cant_borrados) /(float) hash -> capacidad;
    if (densidad > FACTOR_CARGA) return true;
    else return false;
}
//Funcion para sacar indice 
size_t hashing(size_t capacidad,const char* clave){
    size_t indice = funcion_hash(clave) % capacidad;
    return indice;
}
//Funcion para redimensionar hash
bool redimensionar_hash(hash_t* hash){
    campo_hash_t* tabla_aux = hash -> tabla;
    size_t capacidad_aux = hash -> capacidad;
    //Creo una capacidad nueva,cuyo valor sea un numero primo para tener menor probabilidad de colision
    size_t nueva_capacidad = hash -> capacidad*2;
    campo_hash_t* nueva_tabla = calloc(nueva_capacidad,sizeof(campo_hash_t));
    if(!nueva_tabla) return false;
    //Pongo los nuevos datos del hash
    hash -> capacidad = nueva_capacidad;
    hash -> cant_borrados = 0;
    hash -> tabla = nueva_tabla;
    hash -> cant_ocupados = 0;
    //hash -> cantidad = 0;
    iniciar_tabla(hash);
    for(size_t i = 0;i < capacidad_aux;i++){
        if(tabla_aux[i].estado == OCUPADO){
            hash_guardar(hash,tabla_aux[i].clave,tabla_aux[i].dato);
            free(tabla_aux[i].clave);
            //destruir_campo(tabla_aux[i].clave,tabla_aux[i].dato,hash -> destruir_dato);
        }

    }
    free(tabla_aux);
    return true;

    
}

long buscar_posicion_iter(size_t indice,const hash_t* hash){
    for(size_t contador = indice;contador < hash -> capacidad;contador ++){
        if(hash -> tabla[contador].estado != OCUPADO){
            continue;
        }else return contador;
    }
    return hash -> capacidad;
}




//===========================Primitivas Hash=============================


hash_t* hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;
    hash -> capacidad = CAPACIDAD_INICIAL;
    campo_hash_t* tabla = calloc(CAPACIDAD_INICIAL,sizeof(campo_hash_t));
    if(!tabla) return false;
    hash -> cant_borrados = 0;
    hash -> cant_ocupados = 0;
    hash -> destruir_dato = destruir_dato;
    hash -> tabla = tabla;
    iniciar_tabla(hash);
    
    return hash;
}

bool hash_guardar(hash_t* hash,const char* clave,void* dato){
    //Veo si es necesario redimensionar la tabla
    if(esta_sobrecargado(hash)){
        if(!redimensionar_hash(hash)) return false;
    }

    size_t indice = hashing(hash -> capacidad,clave);
    //Si la clave esta,renuevo el dato
    if (hash -> tabla[indice].estado == OCUPADO && !strcmp(hash -> tabla[indice].clave,clave)) renovar_campo(hash,dato,indice);
    //Si el indice de la tabla esta ocupado,busco lugar
    else if(hash -> tabla[indice].estado != OCUPADO){
        llenar_campo(hash,clave,indice,dato);

        hash -> cant_ocupados ++;
    }
    else if (hash -> tabla[indice].estado == OCUPADO){
        if (hash_buscar(hash,clave,true,dato) == NO_ENCONTRADO) return false;
        hash -> cant_ocupados ++;
    }
    return true;
}

void* hash_borrar(hash_t *hash, const char *clave){

    size_t indice =  hash_buscar(hash,clave,false,NULL);
    if (indice == NO_ENCONTRADO) return NULL;
    void* aux = hash -> tabla[indice].dato;
    destruir_campo(hash -> tabla[indice].clave,hash -> tabla[indice].dato,NULL);
    hash -> tabla[indice].clave = NULL;
    hash -> tabla[indice].dato = NULL;
    hash -> tabla[indice].estado = BORRADO;
    hash -> cant_ocupados --;
    hash -> cant_borrados ++;
    return aux;
}


void* hash_obtener(const hash_t *hash, const char *clave){
    size_t indice = hash_buscar(hash,clave,false,NULL);
    if (indice == NO_ENCONTRADO) return NULL;
    return hash -> tabla[indice].dato;
}


bool hash_pertenece(const hash_t *hash, const char *clave){
    size_t indice = hash_buscar(hash,clave,false,NULL);
    if(indice == NO_ENCONTRADO) return false;

    return true;
}


size_t hash_cantidad(const hash_t *hash){
   return hash -> cant_ocupados;
}


void hash_destruir(hash_t *hash){
    for (size_t i = 0;i < hash -> capacidad;i++){
        if (hash -> tabla[i].estado == OCUPADO){
            destruir_campo(hash -> tabla[i].clave,hash -> tabla[i].dato,hash -> destruir_dato);
        }
        
    }
    free(hash -> tabla);
    free(hash);
    
}
//===========================Primitivas Iter=============================

hash_iter_t *hash_iter_crear(const hash_t *hash){
    size_t i = 0;
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if (!iter) return NULL;
    iter -> hash = hash;
    i = buscar_posicion_iter(i,hash);//Hice esta funcion que creo que la podemos usar para modularizar un poquito mas en el resto de las funciones
    iter -> pos = i;
    return iter;
}
// Avanza iterador
bool hash_iter_avanzar(hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return false;
    //size_t i = iter -> pos;
    size_t i = buscar_posicion_iter(iter -> pos + 1,iter -> hash);
    //if (i == NO_ENCONTRADO) return false;
    iter -> pos = i;
    return true;
}
// Devuelve clave actual, esa clave no se puede modificar ni liberar.
const char *hash_iter_ver_actual(const hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return NULL;
    
    return iter -> hash -> tabla[iter -> pos].clave;
}
// Comprueba si terminó la iteración
bool hash_iter_al_final(const hash_iter_t *iter){
    return iter -> pos == iter -> hash -> capacidad;
}
// Destruye iterador
void hash_iter_destruir(hash_iter_t* iter){
    free(iter);
}



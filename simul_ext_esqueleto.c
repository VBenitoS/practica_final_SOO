#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps); // HECHO
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup); // HECHO
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos); // HECHO
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
             EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];

	 int i,j;
    unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos;
    FILE *fent;

    // Lectura del fichero completo de una sola vez


    fent = fopen("particion.bin","r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);


    memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);

    // Buce de tratamiento de comandos
    for (;;){
    do {
    printf (">> ");
    fflush(stdin);
    fgets(comando, LONGITUD_COMANDO, stdin);
    } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
      if (strcmp(orden, "info") == 0) { //ver info de superbloque
        LeeSuperBloque(&ext_superblock);
        continue;
      }

      if (strcmp(orden, "bytemaps") == 0) {
        Printbytemaps(&ext_bytemaps); // Llama a la función y pasa el bytemap
        continue;
      }

      if (strcmp(orden,"dir")==0) {
        Directorio(&directorio,&ext_blq_inodos);
        continue;
      }
        // Escritura de metadatos en comandos rename, remove, copy
        Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
        GrabarByteMaps(&ext_bytemaps,fent);
        GrabarSuperBloque(&ext_superblock,fent);
        if (grabardatos)
          GrabarDatos(&memdatos,fent);
        grabardatos = 0;
        //Si el comando es salir se habrán escrito todos los metadatos
        //faltan los datos y cerrar
        if (strcmp(orden,"salir")==0){
          GrabarDatos(&memdatos,fent);
          fclose(fent);
          printf("Has salido correctamente!\n");
          return 0;
        }
    }
}

// ESTE ES PARA EL INFO
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
  printf("Bloque %u Bytes\n", psup->s_block_size);
  printf("Inodos particion = %u\n", psup->s_inodes_count);
  printf("Inodos libres = %u\n", psup->s_free_inodes_count);
  printf("Bloques particion = %u\n", psup->s_blocks_count);
  printf("Bloques libres = %u\n", psup->s_free_blocks_count);
  printf("Primer bloque de datos = %u\n", psup->s_first_data_block);
}

// ESTE ES EL BYTEMAPS
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodos: ");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    printf("Bloques [0-25]: ");
    for (int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

// ESTE ES PARA VER EL DIRECTORIO
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            continue;
        }
        EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[i].dir_inodo];
        printf("%s\ttamaño:%u\tinodo:%u\tbloques:",
            directorio[i].dir_nfich,
            inodo->size_fichero,
            directorio[i].dir_inodo);
        for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
            if (inodo->i_nbloque[j] != NULL_BLOQUE) {
                printf(" %u", inodo->i_nbloque[j]);
            }
        }
        printf("\n");
    }
}

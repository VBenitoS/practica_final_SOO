#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps); // HECHO
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2); //HECHO
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup); // HECHO
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombre); // HECHO
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos); // HECHO
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombreantiguo, char *nombrenuevo); // HECHO
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
             EXT_DATOS *memdatos, char *nombre); // HECHO
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich); // HECHO
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich); // HECHO
int CrearFichero(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
                 EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
                 EXT_DATOS *memdatos, char *nombre_fichero, FILE *fich); // HECHO
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich); //HECHO
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich); //HECHO
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich); // HECHO
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich); // HECHO
int BuscarInodoLibre(EXT_BYTE_MAPS *ext_bytemaps); // HECHO


int main() {
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];

    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    FILE *fent;

    // Abrir el archivo de partición binaria
    fent = fopen("particion.bin", "r+b");
    if (!fent) {
        perror("Error al abrir el fichero 'particion.bin'");
        return -1;
    }
    // Leer la estructura completa desde el archivo
    fread(&ext_superblock, SIZE_BLOQUE, 1, fent);
    fread(&ext_bytemaps, SIZE_BLOQUE, 1, fent);
    fread(&ext_blq_inodos, SIZE_BLOQUE, 1, fent);
    fread(&directorio, SIZE_BLOQUE, 1, fent);
    fread(&memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fent);

    // Bucle de procesamiento de comandos
    while (1) {
        printf(">> ");
        fgets(comando, sizeof(comando), stdin);
        if (ComprobarComando(comando, orden, argumento1, argumento2) != 0) {
            printf("ERROR: Comando no válido\n");
            continue;
        }

        if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
        } else if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
        } else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        } else if (strcmp(orden, "rename") == 0) {
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
            Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
        } else if (strcmp(orden, "remove") == 0) {
            Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
        } else if (strcmp(orden, "imprimir") == 0) {
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        } else if (strcmp(orden, "copy") == 0) {
            Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);
        } else if (strcmp(orden, "crear") == 0) {
            CrearFichero(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, fent);
        } else if (strcmp(orden, "salir") == 0) {
            fclose(fent);
            printf("Has salido correctamente!\n");
            break;
        } else {
            printf("ERROR: Comando desconocido: %s\n", orden);
        }
    }
    return 0;
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
        printf("%s\ttamanio:%u\tinodo:%u\tbloques:",
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

// ESTE ES RENOMBRAR FICHERO
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombreAntiguo, char *nuevoNombre) {
    int i, archivo_origen = -1, archivo_destino = -1;
    for (i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombreAntiguo) == 0) {
            archivo_origen = i;
        }
        if (strcmp(directorio[i].dir_nfich, nuevoNombre) == 0) {
            archivo_destino = i;
        }
    }
    if (archivo_origen == -1) {
        printf("ERROR: Fichero %s no encontrado\n", nombreAntiguo);
        return -1;
    }
    if (archivo_destino != -1) {
        printf("ERROR: El fichero %s ya existe\n", nuevoNombre);
        return -1;
    }
    strncpy(directorio[archivo_origen].dir_nfich, nuevoNombre, LEN_NFICH - 1);
    directorio[archivo_origen].dir_nfich[LEN_NFICH - 1] = '\0';
    printf("El fichero %s ahora se llama %s\n", nombreAntiguo, nuevoNombre);
    return 0;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre, FILE *fich) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            if (directorio[i].dir_inodo != NULL_INODO) {

                EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[i].dir_inodo];
                for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                    if (inodo->i_nbloque[j] != NULL_BLOQUE) {
                        ext_bytemaps->bmap_bloques[inodo->i_nbloque[j]] = 0;
                        ext_superblock->s_free_blocks_count++;
                        inodo->i_nbloque[j] = NULL_BLOQUE;
                    }
                }
                inodo->size_fichero = 0;
                ext_bytemaps->bmap_inodos[directorio[i].dir_inodo] = 0;
                ext_superblock->s_free_inodes_count++;

                memset(directorio[i].dir_nfich, 0, LEN_NFICH);
                directorio[i].dir_inodo = NULL_INODO;

                GrabarByteMaps(ext_bytemaps, fich);
                GrabarSuperBloque(ext_superblock, fich);
                Grabarinodosydirectorio(directorio, inodos, fich);
                return 0;
            }
        }
    }
    return -1;
}



int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
             EXT_DATOS *memdatos, char *nombre) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            int inodo_id = directorio[i].dir_inodo;
            if (inodo_id != NULL_INODO) {
                EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodo_id];
                for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                    if (inodo->i_nbloque[j] != NULL_BLOQUE) {
                        printf("%s", memdatos[inodo->i_nbloque[j]].dato);
                    }
                }
                return 0;
            }
        }
    }
    return -1; // no encuentra el fichero
}


int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    int indice_origen = -1, indice_destino = -1;

    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombreorigen) == 0) indice_origen = i;
        if (directorio[i].dir_inodo == NULL_INODO) {
            indice_destino = i;
            break;
        }
    }
    if (indice_origen == -1) return -1; // origen no encontrado
    if (indice_destino == -1) return -1; // no hay espacio suficiente

    int nuevo_inodo = -1;
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            nuevo_inodo = i;
            break;
        }
    }
    if (nuevo_inodo == -1) return -1;

    EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[directorio[indice_origen].dir_inodo];
    EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[nuevo_inodo];
    inodo_destino->size_fichero = inodo_origen->size_fichero;
    int bloques_copiados = 0;
    for (int j = 0; j < MAX_NUMS_BLOQUE_INODO && inodo_origen->i_nbloque[j] != NULL_BLOQUE; j++) {
        for (int k = 0; k < MAX_BLOQUES_DATOS; k++) {
            if (ext_bytemaps->bmap_bloques[k] == 0) {
                inodo_destino->i_nbloque[j] = k;
                memcpy(&memdatos[k], &memdatos[inodo_origen->i_nbloque[j]], SIZE_BLOQUE);
                ext_bytemaps->bmap_bloques[k] = 1;
                bloques_copiados++;
                break;
            }
        }
        if (bloques_copiados <= j) {
            return -1;
        }
    }

    strncpy(directorio[indice_destino].dir_nfich, nombredestino, LEN_NFICH);
    directorio[indice_destino].dir_inodo = nuevo_inodo;
    ext_bytemaps->bmap_inodos[nuevo_inodo] = 1;

    GrabarByteMaps(ext_bytemaps, fich);
    GrabarSuperBloque(ext_superblock, fich);
    Grabarinodosydirectorio(directorio, inodos, fich);
    GrabarDatos(memdatos, fich);

    return 0;
}

int CrearFichero(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
                 EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
                 EXT_DATOS *memdatos, char *nombre_fichero, FILE *fich) {
    int indice_directorio = -1, nuevo_inodo = -1, nuevo_bloque = -1;

    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            indice_directorio = i;
            break;
        }
    }
    if (indice_directorio == -1) {
        printf("Error: No hay espacio en el directorio.\n");
        return -1;
    }

    nuevo_inodo = BuscarInodoLibre(ext_bytemaps);
    if (nuevo_inodo == -1) {
        printf("Error: No hay inodos libres.\n");
        return -1;
    }

    for (int i = 0; i < MAX_BLOQUES_DATOS; i++) {
        if (ext_bytemaps->bmap_bloques[i] == 0) {
            nuevo_bloque = i;
            break;
        }
    }
    if (nuevo_bloque == -1) {
        printf("Error: No hay bloques de datos libres.\n");
        return -1;
    }

    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[nuevo_inodo];
    inodo->size_fichero = 0;
    inodo->i_nbloque[0] = nuevo_bloque;
    for (int i = 1; i < MAX_NUMS_BLOQUE_INODO; i++) {
        inodo->i_nbloque[i] = NULL_BLOQUE;
    }


    strncpy(directorio[indice_directorio].dir_nfich, nombre_fichero, LEN_NFICH - 1);
    directorio[indice_directorio].dir_nfich[LEN_NFICH - 1] = '\0';
    directorio[indice_directorio].dir_inodo = nuevo_inodo;

    ext_bytemaps->bmap_inodos[nuevo_inodo] = 1;
    ext_bytemaps->bmap_bloques[nuevo_bloque] = 1;
    ext_superblock->s_free_inodes_count--;
    ext_superblock->s_free_blocks_count--;

    GrabarSuperBloque(ext_superblock, fich);
    GrabarByteMaps(ext_bytemaps, fich);
    Grabarinodosydirectorio(directorio, inodos, fich);
    GrabarDatos(memdatos, fich);

    return 0;
}



int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    int num_args = sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);
    if (num_args < 1) {
        return 1;
    }
    return 0;
}

// GUARDAR LOS DAOS EN EL ARCHIVO BINARIO
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    fseek(fich, SIZE_BLOQUE * 2, SEEK_SET);
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
    fseek(fich, SIZE_BLOQUE * 3, SEEK_SET);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    fseek(fich, SIZE_BLOQUE * PRIM_BLOQUE_DATOS, SEEK_SET);
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}

int BuscarInodoLibre(EXT_BYTE_MAPS *ext_bytemaps) {
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            return i;  // devuelv el indice del primer inodo libre encontrado
        }
    }
    return -1;  // devuelve -1 si no hay inodos libres
}

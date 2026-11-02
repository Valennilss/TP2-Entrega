// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

int len(char* s) {
    int cantidad = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        cantidad++;
    }
    return cantidad;
}

char* strDuplicate(char* src) {
    // TODO: Contar los caracteres de src, reservar con malloc (no olvidar el '\0'),
    //       copiar todo y devolver el puntero nuevo.
    int longitud = len(src);
    char* duplicado = (char*) malloc((longitud + 1) * sizeof(char));
    for (int i = 0; i <= longitud; i++) {
        duplicado[i] = src[i];
    }
    return duplicado;
}

int strCompare(char* s1, char* s2) {
    // TODO: Devolver 0 si son iguales, 1 si s1 < s2, -1 si s1 > s2.
    //       Ojo: es la convencion INVERSA a strcmp.
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] < s2[i]) {
            return 1;
        }
        if (s1[i] > s2[i]) {
            return -1;
        }
        i++;
    }

    if (s1[i] == '\0' && s2[i] == '\0') {
        return 0;
    }
    if (s1[i] == '\0') {
        return 1;
    }
    return -1;
}

char* strConcatenate(char* src1, char* src2) {
    // TODO: Reservar un string nuevo con src1 seguido de src2, y liberar src1 y src2.
    //       Precondicion: src1 y src2 vienen de malloc y son punteros distintos.
    int longitud1 = len(src1);
    int longitud2 = len(src2);
    
    char* concatenado = (char*) malloc((longitud1 + longitud2 + 1) * sizeof(char));
    
    for (int i = 0; i < longitud1; i++) {
        concatenado[i] = src1[i];
    }
    
    for (int j = 0; j < longitud2; j++) {
        concatenado[longitud1 + j] = src2[j];
    }

    concatenado[longitud1 + longitud2] = '\0';
    free(src1);
    free(src2);
    return concatenado;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

// Funcion auxiliar
RowNode* rowNew() {
    RowNode* row = malloc(sizeof(RowNode));         // Pido memoria para crear las filas. Por cada nodo hago malloc y se inicializan todas las celdas.
    if (row == NULL){                               // Escribimos next=NULL porque malloc devuelve memoria con basura.
        return NULL;} 
    for (int c = 0; c < GRID_COLS; c++) {
        row->cells[c] = 0;
        row->next = NULL;
    } return row;
}

GameBoard* gameBoardNew() {
    // TODO: Reservar memoria para el GameBoard.
    // TODO: Usar un bucle para hacer malloc de las 20 filas (RowNode) y conectarlas.
    // TODO: El puntero first_row debe apuntar al primero de esos nodos.
    // TODO: Inicializar queue_head, queue_tail y first_particle en NULL.
    GameBoard* board = malloc(sizeof(GameBoard));
    if (board == NULL) return NULL;
 
    board->first_row = NULL;
    board->queue_head = NULL;
    board->queue_tail = NULL;
    board->first_particle = NULL;
    board->current_piece.col = 0;
    board->current_piece.row = 0;
    board->current_piece.type = 0;
    board->current_piece.rotation = 0;
    board->current_piece.active = 0;    // sin pieza activa
    board->score = 0;
    board->tick_timer = 0;

    // gameBoardNew: 1 malloc para el GameBoard + 20 mallocs (uno por RowNode).
    // Cada fila nueva se inserta por la cabeza (fila->next = first_row; first_row = fila),
    // asi first_row termina apuntando a la cima sin guardar puntero al ultimo nodo.
    
    for (int i = 0; i < GRID_ROWS; i++) {
        RowNode* row = rowNew();
        if (row == NULL) {
            free(board);    // Fallo un malloc: free libera lo que ya se reservo (filas + board).
            return NULL;
        }
        row->next = board->first_row;  // 1) la fila nueva apunta a la que era la primera
        board->first_row = row;        // 2) la fila nueva pasa a ser la primera
    }
    return board;
}

void gameBoardDelete(GameBoard* board) {
    // TODO: Debe tolerar board == NULL.
    // TODO: Recorrer y liberar todos los RowNode con free().
    // TODO: Recorrer y liberar todos los PieceQueueNode.
    // TODO: Recorrer y liberar todos los ParticleNode.
    // TODO: Finalmente, liberar la estructura GameBoard.
     if (board == NULL){
        return; 
     } 
 
    // En los tres recorridos se guarda next ANTES de hacer free: despues del free
    // el nodo ya no se puede leer.
    
    RowNode* row = board->first_row;
    while (row != NULL) {
        RowNode* next = row->next;
        free(row);
        row = next;
    }
 
    PieceQueueNode* piece = board->queue_head;
    while (piece != NULL) {
        PieceQueueNode* next = piece->next;
        free(piece);
        piece = next;
    }
 
    ParticleNode* part = board->first_particle;
    while (part != NULL) {
        ParticleNode* next = part->next;
        free(part);
        part = next;
    }
 
    free(board);    // El board se libera al final: contiene los punteros de las tres listas.
}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    // TODO: Crear un ParticleNode con memoria dinámica.
    // TODO: Inicializar x, y, velocidades aleatorias y un life_timer.
    // TODO: Agregarlo al principio de la lista enlazada simple de partículas.
    if (board == NULL) {
        return;
    }

    ParticleNode* nueva = malloc(sizeof(ParticleNode));      // Un malloc por partícula. Si falla se descarta: es solo un efecto visual, el juego puede seguir sin ella.
    if (nueva == NULL) {
        return;
    }

    nueva->x = x;
    nueva->y = y;   // Velocidades aleatorias: vel_y siempre negativa (sale hacia arriba), después la gravedad de gameBoardUpdate la hace caer.
    nueva->vel_x = ((rand() % 100) / 10.0f) - 5.0f;
    nueva->vel_y = ((rand() % 50) / -10.0f) - 2.0f;
    nueva->color = color;
    nueva->life_timer = 30 + (rand() % 30);

    nueva->next = board->first_particle;
    board->first_particle = nueva;
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    // TODO: Recorrer la lista de RowNode.
    // TODO: Si un RowNode tiene todas sus celdas > 0, desenlazarlo y hacer free().
    // TODO: Crear un nuevo RowNode vacío (lleno de ceros).
    // TODO: Insertar el nuevo RowNode en la CABEZA del tablero (board->first_row).
    // TODO: Por cada celda destruida, invocar a gameBoardAddParticle().
    if (board == NULL){
        return 0;
    }
    int cleared = 0;
    int index = 0;                // numero de fila (para ubicar las particulas en pantalla)
    RowNode* prev = NULL;         // nodo anterior al actual (NULL si el actual es first_row)
    RowNode* curr = board->first_row;
 
    while (curr != NULL) {
        int full = 1;
        for (int c = 0; c < GRID_COLS; c++) {
            if (curr->cells[c] == 0) { 
                full = 0; 
            }
        }
 
        if (full) {
            // Una particula por celda, del color de la celda.
            for (int c = 0; c < GRID_COLS; c++) {
                gameBoardAddParticle(board, (float)(OFFSET_X + c * CELL_SIZE),(float)(OFFSET_Y + index * CELL_SIZE), curr->cells[c]);
            }
            // Desenlazar: el anterior (o first_row si no hay anterior) salta al siguiente.
            RowNode* to_delete = curr;
            if (prev == NULL){
                 board->first_row = curr->next;
            } else {
                 prev->next = curr->next;
            }
            curr = curr->next;
            free(to_delete);
            cleared++;                // cuenta las filas eliminadas
        } else {
            prev = curr;
            curr = curr->next;
        }
        index++;                      // actualiza la posicion d ela fila eliminada
    }
 
    // Las filas nuevas se insertan DESPUES del recorrido. Si se insertaran durante el
    // recorrido, prev == NULL dejaria de significar "curr es first_row" y se podria
    // pisar la fila recien insertada.
    
    for (int i = 0; i < cleared; i++) {
        RowNode* row = rowNew();
        if (row == NULL) {
            break;
        }   // sin memoria: no hay nada mas razonable que hacer
        row->next = board->first_row;
        board->first_row = row;
    }
    return cleared;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    // TODO: Crear un PieceQueueNode con el 'type'.
    // TODO: Insertarlo al final de la cola (queue_tail) y actualizar punteros.
    if (board == NULL) {
        return;
    }
    PieceQueueNode* node = malloc(sizeof(PieceQueueNode));  // Un malloc por pieza en la cola
    if (node == NULL) {
        return;
    }
    node->type = type;
    node->next = NULL;                             // El nuevo nodo siempre es el ultimo
 
    if (board->queue_tail == NULL) {               // Si la cola estaba vacia (tail == NULL), el nodo es tambien head.
        board->queue_head = node;
    } else {
        board->queue_tail->next = node;
    }
    board->queue_tail = node;
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    // TODO: Si la cola está vacía, retornar 0.
    // TODO: Extraer el primer nodo (queue_head).
    // TODO: Guardar el type en la variable out_type, liberar el nodo y actualizar el head.
    if (board == NULL || board->queue_head == NULL) {
        return 0;           // La cola esta vacia
    }
    PieceQueueNode* node = board->queue_head;
    *out_type = node->type;           // Guardo el type en out_type
    board->queue_head = node->next;   
    if (board->queue_head == NULL) {  // Si era el unico nodo, tail quedaria apuntando a memoria liberada: hay que ponerlo en NULL.
        board->queue_tail = NULL;
    }
    free(node);
    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========
    // TODO: Implementar los casos minimos del enunciado. Por ejemplo:
    //  - strDuplicate: "", "x", un string con distintos caracteres.
    //  - strCompare: "" vs "", "a" vs "a", y una desigualdad en ambos sentidos (s1/s2 y s2/s1).
    //  - strConcatenate: "" + "abc" y "hola " + "mundo". Los argumentos deben crearse con
    //    strDuplicate, porque strConcatenate los libera (no pasar literales).
    //  - Cola: 5 push y 6 pop; el 6to debe devolver 0 y dejar queue_head y queue_tail en NULL.
    //    Verificar tambien que un push posterior funcione.
    //  - Tablero: llenar la fila de indice 19 (la de abajo), llamar a
    //    gameBoardCheckAndClearLines y comprobar que first_row es la fila nueva vacia y que
    //    la lista sigue teniendo 20 filas.
    // Liberar toda la memoria pedida en los tests (strings y tableros).

void testsTablero() {
    GameBoard* b = gameBoardNew();
    if (b == NULL) {
        printf("gameBoardNew devolvio NULL: no se pueden correr los tests\n");
        return;
    }

    // --- Cola: 5 push y 6 pop ---
    printf("-- Cola --\n");
    int tipos[5] = {3, 1, 4, 1, 5};
    for (int i = 0; i < 5; i++) gameBoardEnqueuePiece(b, tipos[i]);
    int v = -99;
    for (int i = 0; i < 5; i++) {
        int ok = gameBoardDequeuePiece(b, &v);
        printf("pop %d: devuelve esperado 1 obtenido %d | tipo esperado %d obtenido %d\n",
               i + 1, ok, tipos[i], v);
    }
    v = -99;
    int ok = gameBoardDequeuePiece(b, &v);
    printf("pop 6: devuelve esperado 0 obtenido %d | out_type sin tocar esperado -99 obtenido %d\n", ok, v);
    printf("head NULL esperado: 1 obtenido: %d | tail NULL esperado: 1 obtenido: %d\n",
           b->queue_head == NULL, b->queue_tail == NULL);
    gameBoardEnqueuePiece(b, 6);
    ok = gameBoardDequeuePiece(b, &v);
    printf("push + pop despues de vaciar: devuelve esperado 1 obtenido %d | tipo esperado 6 obtenido %d\n", ok, v);

    // --- Una linea: fila 19 ---
    printf("-- Una linea (fila 19) --\n");
    RowNode* fila = getRowAt(b, 19);
    for (int c = 0; c < GRID_COLS; c++) fila->cells[c] = 1;
    RowNode* vieja_cima = b->first_row;
    int n = gameBoardCheckAndClearLines(b);

    int filas = 0;
    for (RowNode* r = b->first_row; r != NULL; r = r->next) filas++;
    int vacia = 1;
    for (int c = 0; c < GRID_COLS; c++) if (b->first_row->cells[c] != 0) vacia = 0;
    printf("lineas eliminadas      esperado: 1  obtenido: %d\n", n);
    printf("first_row es nueva     esperado: 1  obtenido: %d\n", b->first_row != vieja_cima);
    printf("first_row vacia        esperado: 1  obtenido: %d\n", vacia);
    printf("filas en la lista      esperado: %d obtenido: %d\n", GRID_ROWS, filas);
    gameBoardDelete(b);

    // --- Dos lineas: filas 0 y 1 ---
    printf("-- Dos lineas (filas 0 y 1) --\n");
    b = gameBoardNew();
    if (b == NULL) return;
    fila = getRowAt(b, 0);
    for (int c = 0; c < GRID_COLS; c++) fila->cells[c] = 1;
    fila = getRowAt(b, 1);
    for (int c = 0; c < GRID_COLS; c++) fila->cells[c] = 1;
    n = gameBoardCheckAndClearLines(b);

    filas = 0;
    for (RowNode* r = b->first_row; r != NULL; r = r->next) filas++;
    printf("lineas eliminadas      esperado: 2  obtenido: %d\n", n);
    printf("filas en la lista      esperado: %d obtenido: %d\n", GRID_ROWS, filas);
    gameBoardDelete(b);
}

void correrTests() {

    // -- strDuplicate --
    char* original = "Hola TD2!";
    char* d1= strDuplicate("");
    char* d2= strDuplicate("x");
    char* d3= strDuplicate(original);
    printf("strDuplicate(\"\")          esperado: \"\"          obtenido: \"%s\"\n", d1);
    printf("strDuplicate(\"x\")         esperado: \"x\"         obtenido: \"%s\"\n", d2);
    printf("strDuplicate(\"%s\") esperado: \"%s\" obtenido: \"%s\"\n", original, original, d3);
    free(d1);
    free(d2);
    free(d3);
 
    // -- strCompare --
    printf("strCompare(\"\", \"\")             esperado: 0  obtenido: %d\n", strCompare("", ""));
    printf("strCompare(\"a\", \"a\")           esperado: 0  obtenido: %d\n", strCompare("a", "a"));
    printf("strCompare(\"abeja\", \"abejas\")  esperado: 1  obtenido: %d\n", strCompare("abeja", "abejas"));
    printf("strCompare(\"abejas\", \"abeja\")  esperado: -1  obtenido: %d\n", strCompare("abejas", "abeja"));

    // -- strConcatenate --
    char* c1=strConcatenate(strDuplicate(""), strDuplicate("abc"));
    char* c2=strConcatenate(strDuplicate("hola "), strDuplicate("mundo"));
    printf("strConcatenate(\"\", \"abc\")        esperado: \"abc\"        obtenido: \"%s\"\n", c1);
    printf("strConcatenate(\"hola \", \"mundo\")        esperado: \"hola mundo\"        obtenido: \"%s\"\n", c2);
    free(c1);
    free(c2);

    // -- Cola y Tablero --
    testsTablero();
}


// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

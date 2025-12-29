typedef enum{
    TO_DO,
    DOING,
    DONE
}CardStatus;

typedef struct{
    int id;
    CardStatus colonna;
    char testo[256];
    int porta_utente;
    time_t last_update;
} Card;
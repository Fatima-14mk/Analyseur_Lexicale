#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 100
#define MAX_LENGTH 32

// Liste des mots-cl�s connus
const char *keywords[] = {"if", "else", "while", "int", "float", "return"};
const int numKeywords = sizeof(keywords) / sizeof(keywords[0]);

// Liste des symboles connus
const char *symbols[] = {"+", "-", "*", "/", "(", ")", "{", "}", "=", ";", ",", "==", "<=", ">=", "!="};
const int numSymbols = sizeof(symbols) / sizeof(symbols[0]);

// D�finition d'un jeton
typedef struct {
    char type[MAX_LENGTH];
    char value[MAX_LENGTH];
    int position;
} Token;

// �tats possibles
enum State { START, IDENTIFIER, NUMBER, SYMBOL, ERROR, FINAL };

// Fonction pour v�rifier si un mot est un mot-cl�
int isKeyword(const char *word) {
    for (int i = 0; i < numKeywords; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Fonction pour v�rifier si un caract�re ou une cha�ne est un symbole
int isSymbol(const char *str) {
    for (int i = 0; i < numSymbols; i++) {
        if (strcmp(str, symbols[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Fonction pour ajouter un jeton
void addToken(Token *tokens, int *tokenCount, const char *type, const char *value, int position) {
    strcpy(tokens[*tokenCount].type, type);
    strcpy(tokens[*tokenCount].value, value);
    tokens[*tokenCount].position = position;
    (*tokenCount)++;
}

// Fonction pour afficher les jetons trouv�s
void printTokens(Token *tokens, int tokenCount) {
    for (int i = 0; i < tokenCount; i++) {
        printf("Token: %-10s | Type: %-15s | Position: %d\n",
               tokens[i].value, tokens[i].type, tokens[i].position);
    }
}

// Fonction pour g�rer les erreurs
void handleError(const char *message, const char *token, int position) {
    printf("Erreur: '%s' %s � la position %d\n", token, message, position);
}

// Fonction principale d'analyse lexicale utilisant une table de transition
void lex(const char *input, Token *tokens, int *tokenCount) {
    enum State state = START;

    char buffer[MAX_LENGTH] = {0};
    int bufferIndex = 0;
    int i = 0;

    while (input[i] != '\0') {
        char currentChar = input[i];

        switch (state) {
            case START:
                if (isspace(currentChar)) {
                    // Ignorer les espaces
                    i++;
                } else if (isalpha(currentChar) || currentChar == '_') {
                    state = IDENTIFIER;
                    buffer[bufferIndex++] = currentChar;
                    i++;
                } else if (isdigit(currentChar)) {
                    state = NUMBER;
                    buffer[bufferIndex++] = currentChar;
                    i++;
                } else if (isSymbol((char[]){currentChar, '\0'})) {
                    state = SYMBOL;
                } else {
                    state = ERROR;
                }
                break;

            case IDENTIFIER:
                if (isalnum(currentChar) || currentChar == '_') {
                    buffer[bufferIndex++] = currentChar;
                    i++;
                } else {
                    buffer[bufferIndex] = '\0';
                    if (isKeyword(buffer)) {
                        addToken(tokens, tokenCount, "Keyword", buffer, i - bufferIndex);
                    } else {
                        addToken(tokens, tokenCount, "Identifier", buffer, i - bufferIndex);
                    }
                    bufferIndex = 0;
                    state = START;
                }
                break;
                case NUMBER:
                if (isdigit(currentChar)) {
                    buffer[bufferIndex++] = currentChar;
                    i++;
                } else if (currentChar == '.') {
                    if (strchr(buffer, '.')) {
                            while(isdigit(input[i])|| input[i]=='.'){
                        buffer[bufferIndex++] = input[i++];}
                        buffer[bufferIndex] = '\0';
                        handleError("Litt�ral num�rique invalide", buffer, i - bufferIndex);
                        bufferIndex = 0;
                        state = START;

                    } else {
                        buffer[bufferIndex++] = currentChar;
                        i++;
                    }
                } else if (isalpha(currentChar)|| currentChar=='_') {
                   while(isalnum(input[i])|| input[i]=='_'){
                    buffer[bufferIndex++] = input[i++];}
                    buffer[bufferIndex] = '\0';
                    handleError("Identifiant invalide", buffer, i - bufferIndex);
                    bufferIndex = 0;
                    state = START;

                } else {
                    buffer[bufferIndex] = '\0';
                    addToken(tokens, tokenCount, "Number", buffer, i - bufferIndex);
                    bufferIndex = 0;
                    state = START;
                }
                break;

            case SYMBOL:
                if (isSymbol((char[]){currentChar, '\0'})) {
                    buffer[0] = currentChar;
                    buffer[1] = '\0';
                    if (input[i + 1] != '\0') {
                        char potentialSymbol[3] = {currentChar, input[i + 1], '\0'};
                        if (isSymbol(potentialSymbol)) {
                            addToken(tokens, tokenCount, "Symbol", potentialSymbol, i);
                            i += 2;
                            state = START;
                            break;
                        }
                    }
                    addToken(tokens, tokenCount, "Symbol", buffer, i);
                    i++;
                    state = START;
                } else {
                    state = ERROR;
                }
                break;

            case ERROR:
                handleError("Caract�re non reconnu", (char[]){currentChar, '\0'}, i);
                i++;
                state = START;
                break;
        }
    }

    if (bufferIndex > 0) {
        buffer[bufferIndex] = '\0';
        if (state == IDENTIFIER) {
            if (isKeyword(buffer)) {
                addToken(tokens, tokenCount, "Keyword", buffer, i - bufferIndex);
            } else {
                addToken(tokens, tokenCount, "Identifier", buffer, i - bufferIndex);
            }
        } else if (state == NUMBER) {
            if (strchr(buffer, '.')) {

                handleError("Litt�ral num�rique invalide", buffer, i - bufferIndex);
            } else {
                addToken(tokens, tokenCount, "Number", buffer, i - bufferIndex);
            }
        } else {
            handleError("Token incomplet", buffer, i - bufferIndex);
        }
    }
    // V�rifier une fin de fichier inattendue apr�s un op�rateur d'affectation
    for (int j = *tokenCount - 1; j >= 0; j--) {
        // V�rifier si l'op�rateur est '='
        if (strcmp(tokens[j].type, "Symbol") == 0 && strcmp(tokens[j].value, "=") == 0) {
            // V�rifier si le jeton pr�c�dent est un identifiant (l'�l�ment concern� par l'affectation)
            if (j > 0 && strcmp(tokens[j - 1].type, "Identifier") == 0) {
                // R�cup�rer l'identifiant et sa position
                const char *identifier = tokens[j - 1].value;
                int position = tokens[j - 1].position;

                // V�rification si le jeton suivant est une valeur (Identifiant, Nombre, ou Mot-cl�)
                if (j == *tokenCount - 1 ||
                    (strcmp(tokens[j + 1].type, "Identifier") != 0 &&
                     strcmp(tokens[j + 1].type, "Number") != 0 &&
                     strcmp(tokens[j + 1].type, "Keyword") != 0)) {
                    // Si aucun jeton de valeur n'est trouv� apr�s '='
                    handleError("Fin de fichier inattendue, attente d'une valeur pour", identifier, position);
                }
            }
            break;
        }
    }


}

// Fonction principale
int main() {
    const char *input = "int main() { int a =2_pp22pi; float b =3.14.5; int x = 10.5; int r = @10.5; int c= }";
    Token tokens[MAX_TOKENS];
    int tokenCount = 0;

    lex(input, tokens, &tokenCount);
    printTokens(tokens, tokenCount);

    return 0;
}

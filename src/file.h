#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define isDelimiter(X)  ((X == '\n')||(X == '\t')||(X == '\r')||(X == 0))
#define isAlphabet(X)   (((X >= 'a')&&(X <= 'z'))||((X >= 'A')&&(X <= 'Z')))




static const char *parseStringDelimited(char *buffer, int &cursor, char delimiter = 0){
    while(!isAlphabet(buffer[cursor]))
        cursor++;
    const char *str = buffer + cursor;
    while(!isDelimiter(buffer[cursor]) && buffer[cursor]!= delimiter){
        cursor++;
    }
    if (buffer[cursor])
        buffer[cursor++] = 0;
    return(str);
}

static const char *parseString_fixedLength(char * buffer, int &cursor, int length){
    while(!isAlphabet(buffer[cursor]))
        cursor++;
    const char *str = buffer + cursor;
    cursor += length;
    buffer[cursor++] = 0;
    return(str);
}

static char * loadFileToBuffer(const char *filepath){
    FILE * f = NULL;
    fopen_s(&f, filepath,"rb");
    if (!f){
        printf("No file");
        return(NULL);
    }
        
    fseek(f, 0, SEEK_END);
    uint32_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = new char[size+1];
    fread(buffer, 1, size, f);
    buffer[size] = 0;
    fclose(f);
    return(buffer);
}





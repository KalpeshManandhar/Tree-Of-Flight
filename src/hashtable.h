#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <functional>

#include "ds.h"


template <typename T, uint32_t size>
struct HashTable{
    LinkedList<T> entries[size];
    uint32_t hashValues[size] = {0};


    uint32_t (*hashFunction);



    uint32_t adler32(const char * key, int len){
        uint32_t a = 1, b = 0;
        for (int i = 0; i<len; i++){
            a = (a + key[i])%65521;
            b = (a + b)%65521;
        }
        return(a|(b<<16));
    }



    void addEntry(const char *key, T value){    
        uint32_t hashValue = adler32(key, strlen(key));
        int index = hashValue%size;
        int probe = 0;
        while(hashValues[index] != hashValue && hashValues[index] != 0){
            index = (index + ++probe)%size;
        }
        hashValues[index] = hashValue;
        entries[index].insertBeginning(newListNode(value));
    }

    LinkedList<T>* getEntry(const char *key){
        uint32_t hashValue = adler32(key,strlen(key));
        int index = hashValue%size;
        int probe = 0;
        while (hashValue != hashValues[index]){
            if (probe == size)
                return(0);
            index = (index + ++probe)%size;
        }   
        return(&entries[index]);
    }

    T search(const char *key, std::function<bool (T)> condition){
        LinkedList<T> *list = getEntry(key);
        if (list) {
            while (auto node = list->iterate()) {
                if (condition(node->data)){
                    return(node->data);
                }
            }
        }
        return(NULL);
    }

};
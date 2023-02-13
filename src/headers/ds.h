#pragma once

#include <iostream>
#include <stdint.h>

template <typename T>
struct ListNode{
    T data;
    ListNode<T> *next;
};

template <typename T>
struct PriorityListNode{
    T data;
    uint32_t priority;
    PriorityListNode<T> *next;
};

template <typename T>
ListNode<T> * newListNode(T data){
    ListNode<T> *newnode = new ListNode<T>;
    newnode->data = data;
    newnode->next = NULL;
    return(newnode);
}

template <typename T>
PriorityListNode<T> * newPriorityNode(T data, uint32_t priority){
    PriorityListNode<T> *newnode = new PriorityListNode<T>;
    newnode->data = data;
    newnode->next = NULL;
    newnode->priority = priority;
    return(newnode);
}



template <typename T>
void deleteNode(ListNode<T> *node){
    delete(node);
}

template <typename T>
void deleteNode(PriorityListNode<T> *node){
    delete(node);
}


template <typename T>
struct Stack{
    ListNode<T> *top;

    Stack():top(NULL){}

    void push(T data){
        ListNode<T> * newNode = newListNode(data);
        newNode->next = top;
        top = newNode;
    }

    T pop(){
        if(top){
            ListNode<T> *ptr = top;
            T value = ptr->data;
            top = top->next;
            deleteNode(ptr);
            return(value);
        }
    }

    bool isEmpty(){
        return(!top);
    }

    void empty(){
        ListNode<T> *ptr = NULL;
        while(top){
            ptr = top;
            top = top->next;
            deleteNode(ptr);
        }
    }
};


template <typename T>
struct Queue{
    ListNode<T> *front, *rear;

    Queue():front(NULL),rear(NULL){}

    void enqueue(T data){
        ListNode<T> *node = newListNode(data);
        if (rear){
            rear->next = node;
            rear = node;
        }
        else{
            front = rear = node;
        }
    }

    T dequeue(){
        if (front){
            ListNode<T> *ptr = front;
            if (front == rear)
                rear = NULL;
            front = front->next;
            T data = ptr->data;
            deleteNode(ptr);
            return(data);
        }
        return(0);
    }

    bool isEmpty(){
        return(!front);
    }

    void empty(){
        ListNode<T> *ptr = NULL;
        while (front){
            ptr = front;
            front = front->next;
            deleteNode(ptr);
        }
        rear = NULL;
    }
};


template <typename T>
struct PriorityQueue{
    PriorityListNode<T> *front;

    PriorityQueue():front(NULL){}

    void enqueue(T data, uint32_t priority){
        PriorityListNode<T> *node = newPriorityNode(data, priority);
        if (front){
            node->next = front;
            front = node;
        }
        else{
            front = node;
        }
    }

    T dequeue(){
        if (front){
            PriorityListNode<T> *ptr = front, *lowestNode = front, *preptr = NULL, *preLowest = NULL;
            uint32_t lowest = -1;
            // if (front == rear)
            //     rear = NULL;
            while (ptr){
                if(ptr->priority < lowest){
                    lowest = ptr->priority;
                    lowestNode = ptr;
                    preLowest = preptr;
                }
                preptr = ptr;
                ptr = ptr->next;
            }
            if (preLowest) {
                preLowest->next = lowestNode->next;
            }
            else
                front = lowestNode->next;
            T data = lowestNode->data;
            deleteNode(lowestNode);
            return(data);
        }
        return(0);
    }

    void updateQueue(T data, uint32_t newPriority){
        PriorityListNode<T> *ptr = front;
        while (ptr){
            if (ptr->data == data){
                ptr->priority = newPriority;
                break;
            }
            ptr = ptr->next;
        }
    }

    bool isEmpty(){
        return(!front);
    }

    void empty() {
        PriorityListNode<T> *ptr = NULL;
        while (front){
            ptr = front;
            front = front->next;
            deleteNode(ptr);
        }
    }
};


template <typename T>
struct LinkedList{
    ListNode<T> * head, *iterator;

    LinkedList():head(NULL),iterator(NULL){}

    void insertBeginning(ListNode<T> *newNode){
        if (head){
            newNode->next = head;
        }
        head = newNode;
    }

    void insertEnd(ListNode<T> * newNode){
        if (head){
            ListNode<T> * ptr = head;
            while (ptr->next){
                ptr = ptr->next;
            }
            ptr->next = newNode;
        }
        else{
            head = newNode;
        }
    }

    void insertAfter(T data, ListNode<T> * newNode){
        ListNode<T> * ptr = head;
        if (ptr){
            while (ptr->data != data && ptr->next){
                ptr = ptr->next;
            }
            newNode->next = ptr->next;
            ptr->next = newNode;
        }
        else{
            head = newNode;
        }
    }

    void insertBefore(T data, ListNode<T> * newNode){
        ListNode<T> * ptr = head;
        if (ptr){
            while (ptr->next && ptr->next->data != data){
                ptr = ptr->next;
            }
            newNode->next = ptr->next;
            ptr->next = newNode;
        }
        else{
            head = newNode;
        }
    }

    void deleteBeginning(){
        if (head){
            ListNode<T> *ptr = head;
            head = head->next;
            deleteNode(ptr);
        }
    }

    void deleteEnd(){
        if (head){
            ListNode<T> *ptr = head;
            ListNode<T> *prev;
            while (ptr->next){
                prev = ptr;
                ptr = ptr->next;
            }
            if (prev)
                prev->next = NULL;
            else
                head = NULL;
            deleteNode(ptr);
        }
    }

    void deleteAfter(T data){
        if (head){
            ListNode<T> *ptr = head;
            while (ptr->next){
                if (ptr->data == data){
                    ListNode<T> *temp = ptr->next;
                    ptr->next = ptr->next->next;
                    deleteNode(temp);
                    break;
                }
                ptr = ptr->next;
            }
        }
    }

    ListNode<T> * search(T data){
        ListNode<T> *ptr = head;
        while (ptr){
            if (ptr->data == data)
                return(ptr);
            ptr = ptr->next;
        }
        return(NULL);
    }

    ListNode<T> * search(int pos){
        ListNode<T> *ptr = head;
        for (int i =0; i<pos;i++){
            if (!ptr)
                break;
            ptr = ptr->next;
        }
        return(ptr);
    }



    ListNode<T> * iterate(int n){

    }

    ListNode<T> *iterate(){
        if (!iterator){
            iterator = head;
        }
        else
            iterator = iterator->next;
        return(iterator);
    }


    void empty(){
        ListNode<T> *ptr = head;
        while (ptr){
            head = head->next;
            delete(ptr);
            ptr = head;
        }
    }
};


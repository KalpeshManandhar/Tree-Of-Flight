#include "ds.h"

#define MIN(a,b) ((a<b)?a:b)

template <typename T>
struct GraphNode;

template <typename T>
struct GraphEdge{
    GraphNode<T> *to;
    uint32_t weight;
};

template <typename T>
struct GraphNode{
    T data;
    uint32_t id;
    // placeholder values for operations
    uint32_t value;
    uint32_t visited=0;
    // list of neighbours
    LinkedList<GraphEdge<T>> neighbours;
};



template <typename T>
GraphNode<T> * newGraphNode(T data){
    GraphNode<T> *newnode = new GraphNode<T>;
    newnode->data = data;
    newnode->id = *((uint32_t*)(&data));
    newnode->neighbours.head = NULL;
    newnode->value = 0 - 1;
    return(newnode);
}



template <typename T>
struct Graph{
    // list of nodes
    LinkedList<GraphNode<T> *> nodes;
    uint32_t size;

    Graph():size(0){}

    GraphNode<T> *searchNode(T data){
        uint32_t id = *((uint32_t*)(&data));
        ListNode<GraphNode<T>*> *ptr = nodes.head;
        while (ptr){
            GraphNode<T> *node = ptr->data; 
            if (node->id == id)
                return(node);
            ptr = ptr->next;
        }
        return(NULL);
    }

    void addNode(GraphNode<T> *node){
        nodes.insertBeginning(newListNode(node));
        size++;
    }

    void addEdge(GraphNode<T> *a, GraphNode<T> *b, uint32_t weight = 0, bool directed = false){
        _ASSERT(a && b);
        a->neighbours.insertBeginning(newListNode(GraphEdge<T>{b, weight}));
        if (!directed)
            b->neighbours.insertBeginning(newListNode(GraphEdge<T>{a, weight}));
        if (!nodes.search(a))
            addNode(a);
        if (!nodes.search(b))
            addNode(b);
    }


    bool BreadthFirstSearch(GraphNode<T> *start, GraphNode<T> *end){
        LinkedList<GraphNode<T>*> path;
        Queue<GraphNode<T>*> toCheck;
        toCheck.enqueue(start);

        GraphNode<T> *current = start;
        // ListNode<GraphEdge<T>> *edges; 
        while (true){
            if (toCheck.isEmpty()){

                return false;
            }
            current = toCheck.dequeue();
            current->visited = 1;
            if (current == end){
                // TODO: add the nodes to the path idk how rn
                return true;
                break;
            }
            // edges = current->neighbours.head;
            while (auto edges = current->neighbours.iterate()){
                GraphNode<T> *neighbour = edges->data.to;
                if (!neighbour->visited)
                    toCheck.enqueue(edges->data.to);
                // edges = edges->next;
            }
        }
        //
        return(false);

    }

    uint32_t Dijkstra(GraphNode<T> *start, GraphNode<T> *end){
        uint32_t weight = 0;
        GraphNode<T> *current = start;
        Stack<GraphNode<T>*> path;
        PriorityQueue<GraphNode<T>*> queue;
        while (auto node = nodes.iterate()){
            if (node->data == start)
                continue;
            queue.enqueue(node->data, -1);
        }
        while (true){
            // GraphNode<T> *next = NULL;
            // uint32_t minWt = -1; 
            while(auto edges = current->neighbours.iterate()){
                GraphNode<T> *to = edges->data.to;
                if (!to->visited){
                    uint32_t wt = weight + edges->data.weight;
                    to->value = MIN(wt, to->value);
                    // if (to->value < minWt){
                    //     minWt = to->value;
                    //     next = to;
                    // }
                    queue.updateQueue(to, to->value);
                }
            }
            // weight = minWt;
            current->visited = 1;
            // if (!next){
            //     if (path.isEmpty()){
            //         return(-1);
            //     }
            //     current = path.pop();
            // }
            // else{
            //     path.push(current);
            // }
            // if (next == end){
            //     return(weight);
            // }
            if (!queue.isEmpty()){
                current = queue.dequeue();
                weight = current->value;
            }
            else{
                return(-1);
            }
            if (current == end) {
                queue.empty();
                return(weight);
            }
        }
    }

};



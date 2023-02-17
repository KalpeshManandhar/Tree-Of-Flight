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
    void *values;
    // list of neighbours
    LinkedList<GraphEdge<T>> neighbours;
};



template <typename T>
GraphNode<T> * newGraphNode(T data){
    GraphNode<T> *newnode = new GraphNode<T>;
    newnode->data = data;
    newnode->id = *((uint32_t*)(&data));
    newnode->neighbours.head = NULL;
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

    void addEdge(GraphNode<T> *a, GraphNode<T> *b, uint32_t weight = 1, bool directed = false){
        _ASSERT(a && b);
        a->neighbours.insertBeginning(newListNode(GraphEdge<T>{b, weight}));
        if (!directed)
            b->neighbours.insertBeginning(newListNode(GraphEdge<T>{a, weight}));
        if (!nodes.search(a))
            addNode(a);
        if (!nodes.search(b))
            addNode(b);
    }


    uint32_t BreadthFirstSearch(GraphNode<T> *start, GraphNode<T> *end, LinkedList<GraphNode<T>*> *path = NULL){
        if (start == end)
            return(0);

        // algorithm specific data for each node
        struct NodeData{
            void *from = NULL;
            uint32_t visited = false;
            uint32_t cost = 0;
        };
        NodeData *info = new NodeData[size];

        // assigning a NodeData struct to each node
        int i =0;
        while (auto node = nodes.iterate()){
            node->data->values = &info[i++];
        }
        
        
        Queue<GraphNode<T>*> toCheck;
        toCheck.enqueue(start);

        GraphNode<T> *current = start;
        while (true){
            // if no nodes are remaining to process, no path exists
            if (toCheck.isEmpty()){
                delete [] info;
                return false;
            }
            current = toCheck.dequeue();
            ((NodeData*)current->values)->visited = 1;
            // if node to be processed is the end node, end algorithm
            if (current == end){
                uint32_t weight = ((NodeData*)current->values)->cost;
                // if path != NULL, return the path 
                if (path){
                    while (current){
                        path->insertBeginning(newListNode(current));
                        current = (GraphNode<T>*)((NodeData*)current->values)->from;
                    }
                }
                toCheck.empty();
                delete[] info;
                return weight;

            }
            // iterate through the neighbours of current node
            while (auto edge = current->neighbours.iterate()){
                GraphNode<T> *neighbour = edge->data.to;
                // if not visited, add to queue
                if (!((NodeData*)neighbour->values)->visited){
                    ((NodeData*)neighbour->values)->from = current;
                    ((NodeData*)neighbour->values)->cost = ((NodeData*)current->values)->cost + edge->data.weight;
                    toCheck.enqueue(neighbour);
                }
            }
        }
    }

    uint32_t DepthFirstSearch(GraphNode<T> *start, GraphNode<T> *end, LinkedList<GraphNode<T>*> *returnPath = NULL){

    }


    uint32_t Dijkstra(GraphNode<T> *start, GraphNode<T> *end, LinkedList<GraphNode<T>*> *returnPath = NULL){
        _ASSERT(start && end);
        
        // algorithm specific node data for each node
        struct NodeData{
            void *from = NULL;
            uint32_t costSoFar = -1;
            uint32_t visited = 0;
        };
        NodeData * nodeList = new NodeData[size];

        uint32_t weight = 0;
        GraphNode<T> *current = start;

        // lowest priority queue
        PriorityQueue<GraphNode<T>*> queue;
        int i = 0;
        // assigning nodeData pointers to each node
        while (auto node = nodes.iterate()){
            node->data->values = &nodeList[i++];
        }
        // costSoFar of start node = 0
        ((NodeData*)current->values)->costSoFar = 0;

        while (true){
            while(auto edges = current->neighbours.iterate()){
                GraphNode<T> *to = edges->data.to;
                NodeData *values = (NodeData*)to->values;
                if (!values->visited){
                    // if node hasnt been seen before
                    if(values->costSoFar == -1){
                        queue.enqueue(to, -1);
                    }
                    uint32_t wt = weight + edges->data.weight;
                    if(wt < values->costSoFar){
                        values->costSoFar = wt;
                        values->from = current;
                        queue.updateQueue(to, values->costSoFar);
                    }
                }
            }
            ((NodeData*)current->values)->visited = 1;

            // get next node to be considered
            if (!queue.isEmpty()){
                current = queue.dequeue();
                weight = ((NodeData*)current->values)->costSoFar;
            }
            // if queue is empty, no path exists from start to end
            else{
                delete[] nodeList;
                return(-1);
            }

            // if end is the lowest cost node is queue, end the algorithm
            if (current == end) {
                queue.empty();
                // get path from start to end
                if (returnPath){
                    while (current){
                        returnPath->insertBeginning(newListNode(current));
                        current = (GraphNode<T>*)((NodeData*)current->values)->from;
                    }
                }
                delete[] nodeList;
                return(weight);
            }
        }
    }

};



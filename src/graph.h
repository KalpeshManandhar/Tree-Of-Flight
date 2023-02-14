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

    uint32_t Dijkstra(GraphNode<T> *start, GraphNode<T> *end, LinkedList<GraphNode<T>*> *returnPath = NULL){
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



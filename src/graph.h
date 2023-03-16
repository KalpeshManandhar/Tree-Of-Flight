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
struct Path{
    GraphNode<T> *start;
    LinkedList<GraphEdge<T>*> edges;
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
    using Node = GraphNode<T>;
    using Edge = GraphEdge<T>;
    using Path = Path<T>;
    // list of nodes
    uint32_t size;
    LinkedList<Node *> nodes;

    Graph():size(0){}

    Node *searchNode(T data){
        uint32_t id = *((uint32_t*)(&data));
        ListNode<Node*> *ptr = nodes.head;
        while (ptr){
            Node *node = ptr->data; 
            if (node->id == id)
                return(node);
            ptr = ptr->next;
        }
        return(NULL);
    }

    void addNode(Node *node){
        nodes.insertBeginning(newListNode(node));
        size++;
    }

    void addEdge(Node *a, Node *b, uint32_t weight = 1, bool directed = false){
        _ASSERT(a && b);
        a->neighbours.insertBeginning(newListNode(Edge{b, weight}));
        if (!directed)
            b->neighbours.insertBeginning(newListNode(Edge{a, weight}));
        if (!nodes.search(a))
            addNode(a);
        if (!nodes.search(b))
            addNode(b);
    }


    uint32_t BreadthFirstSearch(Node *start, Node *end, Path *returnpath = NULL){
        _ASSERT(start && end);
        if (start == end)
            return(0);
        returnpath->start = start;

        // algorithm specific data for each node
        struct NodeData{
            Node *from = NULL;
            Edge *path = NULL;
            uint32_t visited = false;
            uint32_t cost = 0;
        };
        NodeData *info = new NodeData[size];

        // assigning a NodeData struct to each node
        int i =0;
        while (auto node = nodes.iterate()){
            node->data->values = &info[i++];
        }
        
        
        Queue<Node*> toCheck;
        toCheck.enqueue(start);

        Node *current = start;
        while (true){
            // if no nodes are remaining to process, no path exists
            if (toCheck.isEmpty()){
                delete [] info;
                return -1;
            }
            current = toCheck.dequeue();
            ((NodeData*)current->values)->visited = 1;
            // if node to be processed is the end node, end algorithm
            if (current == end){
                uint32_t weight = ((NodeData*)current->values)->cost;
                // if path != NULL, return the path 
                if (returnpath){
                    while (current){
                        if (auto p = ((NodeData*)current->values)->path)
                            returnpath->edges.insertBeginning(newListNode(p));
                        current = (Node*)((NodeData*)current->values)->from;
                    }
                }
                toCheck.empty();
                delete[] info;
                return weight;

            }
            // iterate through the neighbours of current node
            while (auto edge = current->neighbours.iterate()){
                Node *neighbour = edge->data.to;
                // if not visited, add to queue
                if (!((NodeData*)neighbour->values)->visited){
                    ((NodeData*)neighbour->values)->from = current;
                    ((NodeData*)neighbour->values)->path = &edge->data;
                    ((NodeData*)neighbour->values)->cost = ((NodeData*)current->values)->cost + edge->data.weight;
                    toCheck.enqueue(neighbour);
                }
            }
        }
    }

    uint32_t DepthFirstSearch(Node *start, Node *end, Path *returnpath = NULL){
        _ASSERT(start && end);
        if (start == end)
            return(0);
        returnpath->start = start;

        // algorithm specific data for each node
        struct NodeData{
            void *from = NULL;
            Edge *path = NULL;
            uint32_t visited = false;
            uint32_t cost = 0;
        };
        NodeData *info = new NodeData[size];

        // assigning a NodeData struct to each node
        int i =0;
        while (auto node = nodes.iterate()){
            node->data->values = &info[i++];
        }
        
        
        Stack<Node*> toCheck;
        toCheck.push(start);

        Node *current = start;
        while (true){
            // if no nodes are remaining to process, no path exists
            if (toCheck.isEmpty()){
                delete [] info;
                return false;
            }
            current = toCheck.pop();
            ((NodeData*)current->values)->visited = 1;
            // if node to be processed is the end node, end algorithm
            if (current == end){
                uint32_t weight = ((NodeData*)current->values)->cost;
                // if path != NULL, return the path 
                if (returnpath){
                    while (current){
                        if (auto p = ((NodeData*)current->values)->path)
                            returnpath->edges.insertBeginning(newListNode(p));
                        current = (Node*)((NodeData*)current->values)->from;
                    }
                }
                toCheck.empty();
                delete[] info;
                return weight;

            }
            // iterate through the neighbours of current node
            while (auto edge = current->neighbours.iterate()){
                Node *neighbour = edge->data.to;
                // if not visited, add to queue
                if (!((NodeData*)neighbour->values)->visited){
                    ((NodeData*)neighbour->values)->from = current;
                    ((NodeData*)neighbour->values)->path = &edge->data;
                    ((NodeData*)neighbour->values)->cost = ((NodeData*)current->values)->cost + edge->data.weight;
                    toCheck.push(neighbour);
                }
            }
        } 
    }


    uint32_t Dijkstra(Node *start, Node *end, Path *returnPath = NULL){
        _ASSERT(start && end);
        if (start == end)
            return(0);
        returnPath->start = start;
        
        // algorithm specific node data for each node
        struct NodeData{
            void *from = NULL;
            Edge *path = NULL;
            uint32_t costSoFar = -1;
            uint32_t visited = 0;
        };
        NodeData * nodeList = new NodeData[size];

        uint32_t weight = 0;
        Node *current = start;

        // lowest priority queue
        PriorityQueue<Node*> queue;
        int i = 0;
        // assigning nodeData pointers to each node
        while (auto node = nodes.iterate()){
            node->data->values = &nodeList[i++];
        }
        // costSoFar of start node = 0
        ((NodeData*)current->values)->costSoFar = 0;

        while (true){
            while(auto edges = current->neighbours.iterate()){
                Node *to = edges->data.to;
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
                        values->path = &edges->data;
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
                        if (auto p = ((NodeData*)current->values)->path)
                            returnPath->edges.insertBeginning(newListNode(p));
                        current = (Node*)((NodeData*)current->values)->from;
                    }
                }
                delete[] nodeList;
                return(weight);
            }
        }
    }


    uint32_t AStar(Node *start, Node *end,uint32_t (*heuristic)(Node*, Node*), Path* returnpath = NULL){
        _ASSERT(start && end);
        if (start == end)
            return 0;
        returnpath->start = start;
        
        // algorithm specific node data for each node
        struct NodeData{
            void *from = NULL;
            Edge *path = NULL;
            uint32_t costSoFar = -1;
            uint32_t heuristicCost = -1;
            uint32_t visited = 0;
        };
        NodeData * nodeList = new NodeData[size];

        uint32_t weight = 0;
        Node *current = start;

        // lowest priority queue
        PriorityQueue<Node*> queue;
        int i = 0;
        // assigning nodeData pointers to each node
        while (auto node = nodes.iterate()){
            nodeList[i].heuristicCost = heuristic(node->data, end);
            node->data->values = &nodeList[i++];
        }
        // costSoFar of start node = 0
        ((NodeData*)current->values)->costSoFar = 0;

        while (true){
            while(auto edges = current->neighbours.iterate()){
                Node *to = edges->data.to;
                NodeData *values = (NodeData*)to->values;
                if (!values->visited){
                    // if node hasnt been seen before
                    if(values->costSoFar == -1){
                        queue.enqueue(to, -1);
                    }
                    // new wt = current costsofar + edge wt
                    uint32_t wt = ((NodeData*)current->values)->costSoFar + edges->data.weight;
                    // new total cost = new wt + heuristic value of the node
                    uint32_t newTotalCost = wt + values->heuristicCost;
                    // total cost current = heuristic cost + costsofar of node
                    uint32_t totalCost = (values->costSoFar == -1)?values->costSoFar:values->heuristicCost + values->costSoFar; 

                    // compare the total costs
                    if(newTotalCost < totalCost){
                        values->costSoFar = wt;
                        values->from = current;
                        values->path = &edges->data;
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
                if (returnpath){
                    while (current){
                        if (auto p = ((NodeData*)current->values)->path)
                            returnpath->edges.insertBeginning(newListNode(p));
                        current = (Node*)((NodeData*)current->values)->from;
                    }
                }
                delete[] nodeList;
                return(weight);
            }
        }
    }

};



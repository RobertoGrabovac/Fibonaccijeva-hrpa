#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>


struct node {
    struct node* parent;
    struct node* child;
    struct node* leftsibling;
    struct node* rightsibling;
    int key;
    int rank;
    bool marked;
    
    int vertex;
    int antecedent;
};

struct node* FibHeap = NULL;
int num_of_nodes = 0;

//koristit cemo vezanu listu susjedstva
struct list_node{
    struct node* drugi_vrh;
    int vrijednost;
    struct list_node* next;
};

struct AdjList {
    struct list_node* pocetak;
};

//graf je zapravo polje vezanih listi susjedstva
struct Graph {
    int V;
    struct AdjList* array;
};

//vraca adresu novog susjeda
struct list_node* new_list_node(struct node* drugi_vrh1, int vrijednost1) {
    struct list_node* nova = malloc(sizeof(struct list_node));
    nova->vrijednost = vrijednost1;
    nova->drugi_vrh = drugi_vrh1;
    nova->next = NULL;
    return nova;
}  

//vraca adresu novog (praznog) grafa
struct Graph* new_graph(int V) {
    struct Graph* novi = malloc(sizeof(struct Graph));
    novi->V = V;
    novi->array = malloc(V * sizeof(struct AdjList));
    int i;
    for (i = 0; i < V; i++)
        novi->array[i].pocetak = NULL;
    return novi;
}

//dodaje brid u graf graph
void add_edge(struct Graph* graph, struct node* prvi_kraj1, struct node* drugi_kraj1, int vrijednost1) {
    struct list_node* nova = new_list_node(drugi_kraj1, vrijednost1);
    nova->next = graph->array[prvi_kraj1->vertex].pocetak;
    graph->array[prvi_kraj1->vertex].pocetak = nova;
    //graf je po definiciji neusmjeren pa radimo sljedece
    nova = new_list_node(prvi_kraj1, vrijednost1);
    nova->next = graph->array[drugi_kraj1->vertex].pocetak;
    graph->array[drugi_kraj1->vertex].pocetak = nova;
}

//ispisuje graf (vazno za provjeru unosa)
void printGraph(struct Graph* graph) {
    int i;
    printf("GRAF:\n");
    for (i = 0; i < graph->V; i++) {
        printf("Susjedi vrha %d: ", i);
        struct list_node* temp = graph->array[i].pocetak;
        while (temp) {
            printf("%d(%d)  ", temp->drugi_vrh->vertex, temp->vrijednost);
            temp = temp->next;
        }
        printf("\n");
    }
}

//oslobadja memoriju
void deleteGraph(struct Graph* graph) {
    int i;
    for (i = 0; i < graph->V; i++) {
        struct list_node* temp1 = graph->array[0].pocetak;
        while (temp1) {
            struct list_node* temp2 = temp1;
            temp2 = temp1->next;
            free(temp1->drugi_vrh);
            free(temp1);
            temp1 = temp2;
        }
    }
    free(graph->array);
}

int no_sibl(struct node* node1){
    struct node* temp= node1->rightsibling;
    int count= 0;
    while(temp != node1){
        count++;
        temp= temp->rightsibling;
    }
    return count;
}

void printTabs(int count) {
    for (int i = 0; i < count; i++)
    {
        putchar('\t');
    }
}

void printTreeRecursive(struct node* node1, int level, int d) {
    for(int i= 0; i<= d; i++)
    {
        printTabs(level);
        printf("Node: %d\n", node1->key);

        if (node1->child != NULL)
        {
            printTabs(level);
            printf("Children:\n");
            printTreeRecursive(node1->child, level + 1, no_sibl(node1->child));
        }

        node1 = node1->rightsibling;
    }
}

void printFibHeap(struct node* node1) {
    printTreeRecursive(node1, 0, no_sibl(node1));
}


//funkcija prima Fibonaccijevu hrpu -> pointer na korijen s najmanjom vrijednosti key i broj cvorova
void merge(struct node* root, int num_nodes) {
    if (FibHeap == NULL) {
        FibHeap = root;
        num_of_nodes = num_nodes;
        return;
    }
    
    //linije 23-27: spajamo korijene nase Fibonaccijeve hrpe FibHeap i neke druge. Napomena: radi i kada FibHeap sadrzi samo jedan cvor.
    struct node* temp1 = FibHeap->rightsibling;
    FibHeap->rightsibling = root;
    root->leftsibling = FibHeap;
    root->rightsibling = temp1;
    temp1->leftsibling = root;

    int br_djece = FibHeap->rank;
    if (root->key < FibHeap->key) 
        FibHeap = root;
    
    num_of_nodes += num_nodes;
}

//ovdje smo radi jednostavnosti i potreba u algoritmu prosirili insert
//tako da dobiva vrijednost oznake i ime(oznaku) vrha
struct node* insert(int value, int vrh) {
    //linije 39-45: kreiramo cvor s key = value
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->marked= false;
    new_node->key = value;
    new_node->vertex = vrh;
    new_node->rank = 0;
    new_node->child = NULL;
    new_node->parent = NULL;
    new_node->leftsibling = new_node;
    new_node->rightsibling = new_node;
    //ubacivanje cvora je zapravo specijalan slucaj spajanja Fibonaccijevih hrpa
    merge(new_node, 1);
    return new_node;
}

//ova funkcija ce korijen root2 uciniti djetetom korijena root1
void link_roots(struct node* root2, struct node* root1) {
    //prvo zelimo odspojiti root2 iz liste korijena
    (root2->rightsibling)->leftsibling = root2->leftsibling;
    (root2->leftsibling)->rightsibling = root2->rightsibling;
    //kada smo ga odspojili, ucinit cemo ga djetetom od root1
    root2->parent = root1;
    if (root1->child == NULL) {
        root1->child = root2;
        //pravimo dvostruku cirkularnu vezanu listu
        root2->leftsibling = root2;
        root2->rightsibling = root2;

        root1->rank++; //moze i root1->rank = 1
        return;
    }
    root2->rightsibling = root1->child;
    root2->leftsibling = (root1->child)->leftsibling;
    ((root1->child)->leftsibling)->rightsibling = root2;
    (root1->child)->leftsibling = root2;
    //zelimo da roditelj pokazuje na dijete koje ima najmanju vrijednost key => if uvjet
    if (root2->key < (root1->child)->key)
        root1->child = root2;
    root1->rank++;
}

//funkcija provodi 2. korak od deleteMin => spaja korijene disjunktnih stabala koji imaju isti rang te se taj korak ponavlja
//sve dok postoje neka dva korijena s istim rangom
//u posljednjem djelu provodi se 3. korak od deleteMin => spajamo korijene dobivenih stabala (sva ta stabla su razlicitog ranga)
//u dvostruku cirkularnu vezanu listu korijena
void consolidate() {
    int max_rank = (int)(log2(num_of_nodes) / log2(1.61803398875));
    struct node** array = malloc((max_rank + 1) * sizeof(struct node*));
    int i;
    for (i = 0; i <= max_rank; i++)
        array[i] = NULL;
    struct node* temp1 = FibHeap;
    struct node* temp2;
    struct node* temp3;
    struct node* temp4;
    int temp_rank;
    int koliko = no_sibl(FibHeap) + 1;
    do {
        temp_rank = temp1->rank;
        temp4 = temp1->rightsibling;
        while (array[temp_rank] != NULL) {
            temp2 = array[temp_rank];
            if (temp1->key > temp2->key) {
                temp3 = temp1;
                temp1 = temp2;
                temp2 = temp3;
            }
            //zbog gornjeg if-a, u temp1 se uvijek nalazi key koji je manji od key-a u temp2. Zbog toga ce temp2 uvijek 
            //biti dijete od temp1. Zbog toga smo implementirali funkciju link_roots koja ce uciniti da temp2 postane 
            //dijete od temp1 (prvi argument postaje dijete drugog argumenta funkcije)
            if (temp2 == FibHeap) FibHeap = temp1;
            link_roots(temp2, temp1);
            array[temp_rank] = NULL;
            temp_rank++;
        }
        array[temp_rank] = temp1;
        temp1 = temp4;
        koliko--;
    } while (koliko);
    FibHeap = NULL;
    //u polju array sadrzani su pointeri na korijene svih disjunktnih stabala koja cine Fibonaccijevu hrpu, a svaki 
    //array[i] != NULL predstavlja pointer na neko stablo. Naravno, sva ona su razlicitog ranga.
    //for petlja: povezujemo cvorove koji predstavljaju korijene disjunktnih stabala koja cine Fibonaccijevu hrpu u 
    //dvostruku cirkularnu vezanu listu. Takodjer, pri tome trazimo node s najmanjom vrijednosti key. Na kraju for 
    //petlje ce FibHeap biti pointer na node s najmanjom vrijednosti key, kako i treba biti.
    for (i = 0; i <= max_rank; i++) {
        if (array[i] != NULL) {
            array[i]->leftsibling = array[i];
            array[i]->rightsibling = array[i];
            if (FibHeap != NULL) {
                //stavljamo array[i] u listu korijena
                (FibHeap->leftsibling)->rightsibling = array[i];
                array[i]->leftsibling = FibHeap->leftsibling;
                array[i]->rightsibling = FibHeap;
                FibHeap->leftsibling = array[i];
                //azuriramo FibHeap, ako za to ima potrebe => if uvjet
                if (array[i]->key < FibHeap->key) 
                    FibHeap = array[i];
            } else FibHeap = array[i];
        }
    }
}

void deleteMin(void) {
    if (FibHeap == NULL) {
        printf("Fibonaccijeva hrpa je prazna!\n");
        exit(1);
    }
    struct node* temp1;
    struct node* temp2;
    struct node* temp3;
    //sada provodimo prvi korak algoritma: ubacujemo djecu cvora s minimalnom oznakom u vezanu listu korijena (ako djeca postoje => if uvjet)
    //ako djeca ne postoje, 1. korak algoritma je trivijalno obavljen
    if (FibHeap->child != NULL) {
        temp1 = FibHeap->child;
        temp2 = temp1;
        temp3 = temp1;
        while (1) {
            temp2 = temp3->rightsibling;
            temp3->parent = NULL;
            merge(temp3, 0);
            temp3 = temp2;
            if (temp3 == temp1) break;
        }
        FibHeap->child = NULL;
        //nakon gornje while petlje, u vezanu listu korijena ubacili smo djecu cvora s minimalnom oznakom, tj. obavili smo 1. korak
    }
    //linije 73-74: izbacujemo minimalni element iz vezane liste korijena
    struct node* temp = FibHeap;
    (temp->leftsibling)->rightsibling = temp->rightsibling;
    (temp->rightsibling)->leftsibling = temp->leftsibling;
    FibHeap = temp->rightsibling;
    //gornjim nismo nista pametno napravili ako je Fibonaccijeva hrpa velicine 1 pa to zasebno rjesavamo iducim if uvjetom
    if (temp == FibHeap && temp->child == NULL) FibHeap == NULL;
    else consolidate();
    num_of_nodes--;

    return;
}

void DecreaseKey(struct node* node1, int n){            //n je nova vrijednost od node1
    if (n > node1->key) {
        printf("Nova vrijednost cvora nije manja!\n");
        exit(1);
    }
    node1->key= n;
    if(node1->parent== NULL && node1->key < FibHeap->key) FibHeap= node1;
    
    if(node1->parent!= NULL && n < node1->parent->key  &&  node1->parent->marked== false){      //ako roditelj nije oznacen
        if (node1->parent->child == node1 && node1->rightsibling != node1){
            struct node* temp2= node1->parent;
            temp2->child= node1->rightsibling;    //novo dijete bi trebalo biti novi minimalni ali za sad
        }
        if (node1->parent->child == node1 && node1->rightsibling == node1) node1->parent->child= NULL;
        node1->parent->rank--;
        node1->parent->marked= true;
        node1->leftsibling->rightsibling= node1->rightsibling;
        node1->rightsibling->leftsibling= node1->leftsibling;
        node1->parent= NULL;
        
        struct node* temp= FibHeap->rightsibling;           //ubacivanje node1 u listu korijena
        node1->leftsibling= FibHeap;
        FibHeap->rightsibling= node1;
        node1->rightsibling= temp;
        temp->leftsibling= node1;
        
        node1->marked= false;
        
        if (node1->key < FibHeap->key) FibHeap= node1;         //updateanje FibHeap ako treba
    }
    else if(node1->parent!= NULL && n < node1->parent->key  &&  node1->parent->marked== true){
        struct node* temp1= node1->parent;
        do{      //idemo prema gore sve dok je sljedeci roditelj oznacen
            if (node1->parent->child == node1 && node1->rightsibling != node1){
                struct node* temp2= node1->parent;
                temp2->child= node1->rightsibling;
            }
            if (node1->parent->child == node1 && node1->rightsibling == node1) node1->parent->child= NULL;
            node1->parent->rank--;
            node1->leftsibling->rightsibling= node1->rightsibling;
            node1->rightsibling->leftsibling= node1->leftsibling;
            node1->parent= NULL;
            
        
            struct node* temp= FibHeap->rightsibling;
            node1->leftsibling= FibHeap;
            FibHeap->rightsibling= node1;
            node1->rightsibling= temp;
            temp->leftsibling= node1;
            
            if (node1->key < FibHeap->key) FibHeap= node1;
            
            node1->marked= false;
            
            node1= temp1;
            temp1= temp1->parent;
            
        } while(node1->marked== true && temp1!= NULL);
        
        node1->marked= true;
    }
}

void delete(struct node* node1){
    DecreaseKey(node1, FibHeap->key - 1);
    deleteMin();
}

int min() {
    return FibHeap->key;
}

//vraca vrijednost minimalnog razapinjuceg stabla te ga pri tome ispisuje
int prim(struct Graph* graph) {
    int i;
    int mst_value = 0;
    FibHeap->antecedent = 0;
    printf("Vrh1:        Vrh2:        Vrijednost brida (Vrh1, Vrh2):\n");
    //u FibHeap vec imamo sve vrhove grafa pa odmah nastavljamo sa sustinom algoritma
    for (i = 0; i < graph->V - 1; i++) {
        struct node* u = FibHeap;
        int vrh = u->vertex, prethodnik;
        int min_vrijednost_brida, flag = 1;
        struct list_node* temp = graph->array[vrh].pocetak;
        while (temp) {
            if (temp->drugi_vrh->key > temp->vrijednost) {
                //printf("aa %d %d\n", temp->drugi_vrh->key, temp->vrijednost);
                DecreaseKey(temp->drugi_vrh, temp->vrijednost);
                temp->drugi_vrh->antecedent = vrh;
            }
            if (flag && temp->drugi_vrh->key != -INT_MAX) {
                    min_vrijednost_brida = temp->drugi_vrh->key;
                    flag = 0;
            } else if (temp->vrijednost < min_vrijednost_brida && temp->drugi_vrh->key != -INT_MAX) {
                    min_vrijednost_brida = temp->drugi_vrh->key;
            }
            temp = temp->next;
        }
        deleteMin();
        mst_value += FibHeap->key;
        printf("%d            %d            %d\n", FibHeap->vertex, FibHeap->antecedent, min_vrijednost_brida);
        DecreaseKey(FibHeap, -INT_MAX);
    }
    return mst_value;
}


int main (void) {
    int V = 7;
    int vrijednost = 0;
    //PRVI GRAF SA V = 7 VRHOVA
    //NAPOMENA: vrhovi su oznaceni s 0, 1, 2, ..., V - 1
    struct Graph* graph = new_graph(V);
    //sada treba koristiti funkciju add_edge da se kreira graf. No, prije
    //toga potrebno je kreirati svaki vrh koji ce biti node, a to cinimo
    //tako da ih u isto vrijeme ubacujemo u Fibonaccijevu hrpu
    struct node* n0 = insert(-INT_MAX, 0); //s ovim vrhom krecemo u alg.
    struct node* n1 = insert(INT_MAX, 1);
    struct node* n2 = insert(INT_MAX, 2);
    struct node* n3 = insert(INT_MAX, 3);
    struct node* n4 = insert(INT_MAX, 4);
    struct node* n5 = insert(INT_MAX, 5);
    struct node* n6 = insert(INT_MAX, 6);
    add_edge(graph, n0, n1, 28); add_edge(graph, n0, n5, 10);
    add_edge(graph, n5, n4, 25);
    add_edge(graph, n4, n6, 24); add_edge(graph, n4, n3, 22);
    add_edge(graph, n6, n3, 18); add_edge(graph, n3, n2, 12);
    add_edge(graph, n1, n6, 14); add_edge(graph, n1, n2, 16);
    printGraph(graph);
    vrijednost = prim(graph);
    printf("Vrijednost minimalnog razapinjuceg stabla iznosi: %d.\n", vrijednost);
    deleteGraph(graph);


    //DRUGI GRAF SA V = 7 VRHOVA
    FibHeap = NULL;
    struct Graph* graph2 = new_graph(V);
    n0 = insert(-INT_MAX, 0);
    n1 = insert(INT_MAX, 1);
    n2 = insert(INT_MAX, 2);
    n3 = insert(INT_MAX, 3);
    n4 = insert(INT_MAX, 4);
    n5 = insert(INT_MAX, 5);
    n6 = insert(INT_MAX, 6);
    add_edge(graph2, n0, n1, 1); add_edge(graph2, n0, n3, 4);
    add_edge(graph2, n1, n3, 6); add_edge(graph2, n1, n2, 2); add_edge(graph2, n1, n4, 4);
    add_edge(graph2, n2, n4, 5); add_edge(graph2, n2, n5, 6);
    add_edge(graph2, n3, n4, 3); add_edge(graph2, n3, n6, 4);
    add_edge(graph2, n4, n5, 8); add_edge(graph2, n4, n6, 7);
    add_edge(graph2, n5, n6, 3);
    printGraph(graph2);
    vrijednost = prim(graph2);
    printf("Vrijednost minimalnog razapinjuceg stabla iznosi: %d.\n", vrijednost);
    deleteGraph(graph2);
    return 0;
}
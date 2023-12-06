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
    
    bool done;
    int number;
    struct node* origin;
};

struct node* FibHeap = NULL;
int num_of_nodes = 0;

int no_sibl(struct node* node1){
    struct node* temp= node1->rightsibling;
    int count= 0;
    while(temp != node1){
        count++;
        temp= temp->rightsibling;
    }
    return count;
}

void printTabs(int count)
{
    for (int i = 0; i < count; i++)
    {
        putchar('\t');
    }
}

void printTreeRecursive(struct node* node1, int level, int d)
{
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

void printFibHeap(struct node* node1)
{
    printTreeRecursive(node1, 0, no_sibl(node1));
    printf("\n");
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

struct node* insert(int value) {
    //linije 39-45: kreiramo cvor s key = value
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->marked= false;
    new_node->done= false;
    new_node->key = value;
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
 
//broj vrhova u grafu
#define V 5

void dijkstra(struct node* graph[V][V], int price[V][V], struct node* vertex) {
    //put iz izvora do izvora je naravno 0 pa tako postavljamo
    vertex->key = 0;
    vertex->origin= vertex;
 
    printf("Ime vrha:      Ime prethodnog vrha:    Cijena puta:\n");
    
    //for petlja se izvrsava |V| puta jer se u njoj upravo toliko puta
    //poziva operacija deleteMin, a svaki poziv znaci da smo pronasli
    //najkraci put od s do doticnog vrha kojeg brisemo
    int i;
    for (int count = 0; count < V ; count++) {
        i = 0;
        struct node* u = FibHeap;
        u->done = true;
        printf("%d                      %d                   %d\n",u->number,u->origin->number, u->key);
        //u while petlji azuriramo najkrace puteve tako sto prolazimo kroz sve trenutno dostupne cvorove
        while (graph[u->number][i] != NULL) {
            if (!graph[u->number][i]->done && u->key != INT_MAX
                && u->key + price[u->number][graph[u->number][i]->number] < graph[u->number][i]->key){
                DecreaseKey(graph[u->number][i], u->key + price[u->number][graph[u->number][i]->number]);
                graph[u->number][i]->origin= u;
            }
            i++;
        }
        deleteMin();
    }
}

int main(void){
    struct node* graph[V][V];
    int i, j;
    for (i = 0; i < V; i++)
        for (j = 0; j < V; j++)
            graph[i][j] = NULL;
    
    int price[V][V] = { { 0,10,0,5,0 },
                        { 0,0,1,2,0},
                        { 0,0,0,0,4 },
                        { 0,3,9,0,2 },
                        { 7,0,6,0,0 } };
    
    struct node* s= insert(INT_MAX);
    s->number= 0;
    struct node* n1= insert(INT_MAX);
    n1->number= 1;
    struct node* n2= insert(INT_MAX);
    n2->number= 2;
    struct node* n3= insert(INT_MAX);
    n3->number= 3;
    struct node* n4= insert(INT_MAX);
    n4->number= 4;

    graph[0][0] = n1; graph[0][1] = n3;
    graph[1][0] = n2; graph[1][1] = n3;
    graph[2][0] = n4;
    graph[3][0] = n1; graph[3][1] = n2; graph[3][2] = n4;
    graph[4][0] = s; graph[4][1] = n2;

    dijkstra(graph, price, s);
 
    return 0;
}
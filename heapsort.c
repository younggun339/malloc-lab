#include <stdio.h>


// typedef struct ListNode{
//     int val;
//     ListNode *next;
// }ListNode;

// typedef struct List{
//     ListNode *head;
//     int size;
// }List;

// typedef struct treeNode{
//     int val;
//     treeNode *left;
//     treeNode *right;
// }treeNode;


// List* List_init(){

//     List *list;

//     list = malloc(sizeof(List));
//     list->size = 0;
//     list->head = NULL;

//     return list;
// }

void down_heap(int *sequence, int left, int right){

    int temp = sequence[left]
    int parent = left;

    while(parent <(right+1)/2){
        int cl = (parent*2) +1;
        int cr = cl +1;
        if(cr <= right && sequence[cr] > sequence[cl]){
            int child = cr;
        }
        else{
            int child = cl;
        }
        if (temp >= sequence[child]){
            break;
        }
        sequence[parent] = sequence[child];
        parent = child;
    }
    sequence[parent] = temp;

}

void delete(int *sequence){


    for(int i = 0; i<= len(sequence); i++)
    {
        sequence[i] = sequence[i+1]
        if(sequence[i+1] == NULL){
            sequence[i] = 0;
            break;
        }
    }

    // ListNode *node;
    // node = sequence->head->next;

    // ListNode *first;

    // first = sequence->head;

    // free(first);

    // sequence->head = node;
    // sequence->size--;

}

void insert(int *sequence, int val){

    for(int i = 0; i <= len(sequence); i++)
    {
        int prev = sequence[i+1];
        sequence[i+2] = prev;

    }


    // ListNode *node = malloc(sizeof(ListNode));

    // node->next = NULL;
    // node->val = val;

    // ListNode *first = sequence->head;

    // node->next = first;
    // sequence->head = node;
    // sequence->size++;

}

int main(){

    int n;
    int a[1000];

    int x;

    scanf("%d", &n);

    for(int i = 0; i<=n; i++){
        scanf("%d", &x);

        if(x == 0){
            delete(a);
            down_heap(a, 0, (int)(len(a)));
        }
        else{
            insert(a, x);
            down_heap(a, 0, (int)(len(a)));
        }
    }

}
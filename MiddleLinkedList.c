//from http://technical-interview.com/c_questions_1.aspx http://technical-interview.com/How_do_you_find_the_middle_linked_list.aspx

#include<stdio.h>

#include<ctype.h>

typedef struct node

{

  int value;

  struct node *next;

  struct node *prev;

}mynode; 


void add_node(struct node **head, int value);

void print_list(char *listName, struct node *head);

void getTheMiddle(mynode *head);


int main()

{

  mynode *head;

  head = (struct node *)NULL;

  add_node(&head, 1);

  add_node(&head, 10);

  add_node(&head, 5);

  add_node(&head, 70);

  add_node(&head, 9);

  add_node(&head, -99);

  add_node(&head, 0);

  add_node(&head, 555);

  add_node(&head, 55);

  print_list("myList", head);

  getTheMiddle(head);

  //  getch();

  return(0);

}


void getTheMiddle(mynode *head)

{

  mynode *p = head;

  mynode *q = head;

  if(q!=NULL)

    {

      while((q->next)!=NULL && (q->next->next)!=NULL)

        {

	  p=(p!=(mynode *)NULL?p->next:(mynode *)NULL);

	  q=(q!=(mynode *)NULL?q->next:(mynode *)NULL);

	  q=(q!=(mynode *)NULL?q->next:(mynode *)NULL);

        }

      printf("The middle element is [%d]",p->value);

    }

}


void add_node(struct node **head, int value)

{

  mynode *temp, *cur;

  temp = (mynode *)malloc(sizeof(mynode));

  temp->next=NULL;

  temp->prev=NULL;

  if(*head == NULL)

    {

      *head=temp;

      temp->value=value;

    }

  else

    {

      for(cur=*head;cur->next!=NULL;cur=cur->next);

      cur->next=temp;

      temp->prev=cur;

      temp->value=value;

    }

}


void print_list(char *listName, struct node *head)

{

  mynode *temp;

  printf("\n[%s] -> ", listName);

  for(temp=head;temp!=NULL;temp=temp->next)

    {

      printf("[%d]->",temp->value);

    }

  printf("NULL\n");

}




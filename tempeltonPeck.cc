/* tempeltonPeck.cc
// Thu Apr 10 13:23:22 CEST 2003
// 
*/
#include <stdlib.h>
#include <iostream>
#include <string>
#include "tempeltonPeck.h"


//using namespace std;

template < class itemType > Item < itemType >::Item (itemType * i)
{
  item = i;
  next = NULL;
}

template < class itemType > Item < itemType >::~Item (void)
{
  if (item != NULL)
    {
      delete(item);
    }
}

template < class queueType > Queue < queueType >::Queue ()
{
  first = NULL;
  last = NULL;
}


template < class queueType > Queue < queueType >::Queue (string k)
{
  first = NULL;
  last = NULL;
  key = k;
}



template < class queueType > Queue < queueType >::~Queue ()
{
  while (first)
    remove ();
}

template < class queueType > void Queue < queueType >::add (queueType * i)
{
  Item < queueType > *newItem = new Item < queueType > (i);
  Item <queueType> *temp;
  
  if (first == NULL)
    {
      first = newItem;
      last = newItem;
    }
  else
    {
      temp = last;
      temp->next = newItem;
      last = newItem;
    }
}

template < class queueType > int Queue < queueType >::removeAndDelete (void)
{
  Item < queueType > *temp;
  if (!first)
    {
      cerr <<
	"tempeltonPeck.cc, Queue::remove(): trying to remove from empty queue\n";
      return (-1);
    }

  temp = first->next;
  
  
  delete first->item;
  first->item = NULL;
  delete first;
  first = temp;
  return (0);
}

template < class queueType > int Queue < queueType >::remove (void)
{
  Item < queueType > *temp;
  if (!first)
    {
      cerr <<
	"tempeltonPeck.cc, Queue::remove(): trying to remove from empty queue\n";
      return (-1);
    }
  temp = first->next;
  delete first;
  first = temp;
  return (0);
}

template < class queueType > queueType * Queue < queueType >::get (void)
{
  if (!first)
    {
      //cerr <<
      //"tempeltonPeck.cc, Queue::get(): trying to get from empty queue\n";
      return (NULL);
    }
  else
    return (first->item);
}

template < class queueType > queueType * Queue < queueType >::getLast (void)
{
  if (!first)
    {
      cerr <<
	"tempeltonPeck.cc, Queue::getLast(): trying to get from empty queue\n";
      return (NULL);
    }
  else
    return (last->item);
}

template <class queueType>
int Queue<queueType>::isEmpty(void)
{
  if(!first)
    {
      return(1);
    }
  else
    {
      return(0);
    }
}

template <class queueType>
void Queue<queueType>::writeToFile(void)
{
  ofstream logFile(key.c_str());
  Item<queueType> *temp;

  for (temp = first; temp; temp = temp->next)
    {
      logFile  << temp->item->getValue() << "\n";
    }
  //logFile.close();
}
  
  

template < class setType > Set < setType >::Set (void)
{
  first = NULL;
}

template < class setType > Set < setType >::Set (string k)
{
  first = NULL;
  key = k;
}



template < class setType > Set < setType >::~Set (void)
{
  Item < setType > *temp;
  while (first)
    {
      temp = first;
      first = first->next;
      delete temp;
    }
}


template < class setType > int Set < setType >::add (setType * member)
{
  Item < setType > *newItem = new Item < setType > (member);
  Item <setType> *temp, *previous;

  // Set emtpy
  if (first == NULL)
    {
      first = newItem;
      return(1);
    }
  
  // Not empty : find place
  
  // 1: item is the first
  
  if ( first->item < newItem->item)
    {
      newItem->next = first;
      first = newItem;
      return(1);
    }
  
  // find
  temp = first;
  while( temp->item > newItem->item && temp->next != NULL)
	{
	  
	  previous = temp;
	  temp = temp->next;
	}

  //in equal
  if (temp->item == newItem->item)
    return(0);
      
  // Last
  if (temp->next == NULL )
	{
	  temp->next = newItem;
	  return(1);
	}

  else
    {
      previous->next = newItem;
      newItem->next = temp;
    }
  return(1);
}
 

template <class setType> 
int Set<setType>::in(setType *member)
{
  Item<setType> *temp;

  for (temp = first; temp  ; temp = temp->next)
    {
      
      if (member == temp->item)
	{
	  return(1);
	}
    }
  
  return(0);
}

template <class setType> 
int Set<setType>::notIn(setType *member)
{
  if ( in(member))
	return(0);
  else
    return(1);
}

template <class setType>
Set<setType>* Set<setType>::intersectionOf(Set<setType> *S)
{
  Item<setType> *temp;
  Set<setType> *newSet = new Set<setType>;

  for (temp = first; temp; temp = temp->next)
    {
      if (S->in(temp->item))
	{
	  newSet->add(temp->item);
	}
    }
  return(newSet);
}

template <class setType> 
Set<setType>* Set<setType>::unionOf(Set<setType> *S)
{
  Set<setType> *newSet = new Set<setType>(*S);
  Item<setType> *temp;
  
  for (temp = first; temp; temp = temp->next)
  {
      newSet->add(temp->item);
  }
  return(newSet);
}



template <class setType>
void Set<setType>::removeN(int n)
{
    if (first->next != NULL)
    {
	first = first->next->next;
    }
}



template <class setType>
void Set<setType>::randomSubSet(double chance)
{
  Set<setType> *newSet = new Set<setType>();
  Item<setType> *temp, *previous;

  previous = first;

  for (temp = first; temp; temp = temp->next)
    {
      if (((double)rand() / RAND_MAX) > chance)
	{

	  if (temp == first)
	  {
	    first = first->next;
	    previous = temp;
	    
	  }
	  else
	  {
	    previous->next = temp->next;
	  }
	 
	}
      else
	{
	  previous = temp;
	}
      
    }


}
  

template < class setType>
template < class subSetType >
Set<subSetType>* Set<setType>::UnionOf(void)
{
  Set<subSetType> *newSet = new Set<subSetType>;
  Item< setType > *temp;
  
  for (temp = first; temp; temp = temp->next)
    {
      newSet = temp->item->unionOf(newSet);
    }
  return(newSet);
}

template < class setType>
template < class subSetType >
Set<subSetType>* Set<setType>::IntersectionOf(void)
{
  Set<subSetType> *newSet = new Set<subSetType>(*first->item);
  Item< setType > *temp;
  
  for (temp = first; temp; temp = temp->next)
    {
      
      newSet = temp->item->intersectionOf(newSet);
    }
  return(newSet);
}
  
template < class setType >
bool Set<setType>::subSet(Set<setType> *sub)
{
  Item<setType> *temp;
  for(temp = sub->first; temp; temp = temp->next)
    {
      if (!in(first->item))
	{
	  return(0);
	}
    }
  return(1);
}

template < class setType >
void Set<setType>::remove(Item<setType> *item)
{
   Item<setType> *temp;
   cout << "removing\n";
   if (item == first)
     {
       first = first->next;
       return;
     }
   
   for (temp = first; temp; temp = temp->next)
     {
       cout << "checking " << temp << temp->next << "\n";
       if (item == temp->next)
	 {
	   temp->next = temp->next->next;
	   cout << "found\n";
	   return;
	 }
     }
}


template < class setType >
setType* Set<setType>::getRandomItem(void)
{
  int size = 0;
  int choosen, index;
  Item<setType> *temp;
  
  for(temp = first; temp; temp = temp->next)
    {
      size++;
    }

  
  if (size == 0)
    {
      cout << "Getting random item form empty set\n";
      return(NULL);
      }
  

  choosen  =  rand() % size;
  temp = first;
  
  
  for(index = 0 ; index < choosen; index++)
  {
    temp = temp->next;
  }
  

  return(temp->item );
}


template < class setType >
  void Set < setType >::forallDo (void (setType::*function) (void))
{
  Item < setType > *temp;

  for (temp = first; temp; temp = temp->next)
    {
      (temp->item->*function) ();
    }
}

template < class setType >
template <class setterType>
void Set < setType >::forallSet (void (setType::*function) (setterType), setterType Value)
{
  Item < setType > *temp;

  for (temp = first; temp; temp = temp->next)
    {
      (temp->item->*function) (Value);
    }
}

template < class setType >
template <class getterType>
Set<getterType>* Set<setType>::forallGet (getterType* (setType::*function) (void))
{
  Item < setType > *temp;
  Set< getterType> *S = new Set<getterType>;

  for (temp = first; temp; temp = temp->next)
    {
      S->add((temp->item->*function) () );
    }
  return(S);
}

template < class setType >
template < class comparisonType >
Set < setType > *Set <  setType >::getSetByValue (comparisonType (setType::*function) (void), comparisonType Value)
{
  Item < setType > *temp;
  Set < setType > *newSet = new Set < setType >;
  int empty = 1;
  
  for (temp = first; temp; temp = temp->next)
    {
      if ((temp->item->*function) () == Value)
	{
	  empty = 0;
	  newSet->add (temp->item);
	}
    }
  
  if (empty)
    cerr << "Value not found" << Value;
  
  return (newSet);
}

template < class setType >
template < class comparisonType >
setType* Set <  setType >::getObjectByValue (comparisonType (setType::*function) (void), comparisonType Value)
{
  Item < setType > *temp;
  
  for (temp = first; temp; temp = temp->next)
    {
      if ((temp->item->*function) () == Value)
	return(temp->item);
    }
  
  cerr << "Object not Found: " << Value << "\n";
  return (NULL);
}


template <class setType> void Set<setType>::setKey(string k)
{
  key = k;
}

template <class setType> string Set<setType>::getKey(void)
{
  return key;
}





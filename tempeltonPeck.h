// tempeltonPeck.h


using namespace std;


class Int
{
 public:
  Int(int val): value(val) {};
  int getValue(void)
    {
      return(value);
    }
 private:
  int value;
};

class Double
{
 public:
  Double(double val): value(val) {};
  double getValue(void)
    {
      return(value);
    }
 private:
  double value;
};


template <class itemType>
class Item;
template <class queueType>
class Queue;
template <class setType>
class Set; 

template <class itemType>
class Item
{
 public:
  Item(itemType *i);
  ~Item();
  itemType *item;
  Item *next;
};

template <class queueType>
class Queue
{
 public:
  Queue();
  Queue(string k);
  ~Queue();
  void add(queueType *i);
  int remove(void);
  int removeAndDelete(void);
  queueType* get(void);
  queueType* getLast(void);
  void writeToFile();
  int isEmpty(void);
  // private:
  string key;
  Item<queueType> *first, *last;
};


template <class setType>
class Set
{
 public:
  Set(void);
  Set(string k);
  ~Set(void);

  
  int add(setType *member);
  int in(setType *member);
  int notIn(setType *member);

  setType* getRandomItem(void);

  Set<setType>* intersectionOf(Set<setType> *S);
  Set<setType>* unionOf(Set<setType> *S);

  void removeN(int n);
  void randomSubSet(double chance);

  
  template <class subSetType>    
    Set<subSetType>* UnionOf(void);
  template <class subSetType>
    Set<subSetType>* IntersectionOf(void);
  
  void remove(Item<setType> *item);
  bool subSet(Set<setType> *sub);

  void forallDo(void (setType::*function) (void) );
  template <class setterType> 
    void forallSet(void (setType::*function) (setterType), setterType Value);
  template <class getterType> 
    Set<getterType>* forallGet(getterType* (setType::*function) (void) );
  
  template <class comparisonType> 
    Set<setType>* getSetByValue( comparisonType (setType::*function) (void), comparisonType Value);
  template <class comparisonType> 
    setType* getObjectByValue( comparisonType (setType::*function) (void), comparisonType Value);

  void setKey(string k);
  string getKey(void);

  Item<setType> *first;
protected:
  string key;
};


#include <nanosoft/ring.h>
#include <stdio.h>

using namespace nanosoft;

class myobject
{
public:
	const char *name;
	neighbor<myobject> neigh;
	
	myobject(const char *n): neigh(this) {
		name = n;
		printf("create object: %s\n", name);
	}
	
	~myobject()
	{
		printf("destroy object: %s\n", name);
	}
};

int main()
{
	myobject *obj1, *obj2, *obj3;
	neighbor<myobject> *ring1, *ring2;
	
	obj1 = new myobject("first");
	ring1 = & obj1->neigh;
	
	obj2 = new myobject("second");
	ring1->attach( &obj2->neigh );
	
	obj3 = new myobject("third");
	ring1->attach( &obj3->neigh );
	
	
	ring2 = &obj2->neigh;
	ring2->detach();
	ring2->attach( &obj3->neigh );
	
	
	printf("ring1:\n");
	printf(" item: %s\n", ring1->getObject()->name);
	for(neighbor<myobject> *item = ring1->getNext(); item != ring1; item = item->getNext())
	{
		printf("  item: %s\n", item->getObject()->name);
	}
	
	printf("\n\nring2:\n");
	printf(" item: %s\n", ring2->getObject()->name);
	for(neighbor<myobject> *item = ring2->getNext(); item != ring2; item = item->getNext())
	{
		printf("  item: %s\n", item->getObject()->name);
	}
	
	
	printf("\n\nend\n");
	
	return 0;
}

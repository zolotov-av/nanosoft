
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
	ring<myobject> r, r2;
	myobject *obj, *obj2;
	neighbor<myobject> *first;
	
	obj = new myobject("first");
	r.add( &obj->neigh );
	
	obj2 = new myobject("second");
	r.add( &obj->neigh );
	
	obj = new myobject("third");
	r.add( &obj->neigh );
	
	r2.add( &obj2->neigh );
	//delete obj2;
	//r.remove(&obj2->neigh);
	
	printf("first ring:\n");
	first = r.getFirst();
	if ( first == 0 )
	{
		printf("empty ring\n");
		return 0;
	}
	
	printf("  item: %s\n", first->getObject()->name);
	for(neighbor<myobject> *item = first->getNext(); item != first; item = item->getNext())
	{
		printf("  item: %s\n", item->getObject()->name);
	}
	
	printf("\n\nsecond ring:\n");
	first = r2.getFirst();
	if ( first == 0 )
	{
		printf("empty ring\n");
		return 0;
	}
	
	printf("  item: %s\n", first->getObject()->name);
	for(neighbor<myobject> *item = first->getNext(); item != first; item = item->getNext())
	{
		printf("  item: %s\n", item->getObject()->name);
	}
	
	printf("end\n");
	
	return 0;
}

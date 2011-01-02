
#include <nanosoft/hash.h>
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
	myobject *obj1, *obj2;
	hashtab<const char *, myobject> h(10);
	
	obj1 = new myobject("first");
	obj2 = new myobject("second");
	
	h.add(obj1->name, &obj1->neigh );
	h.add(obj1->name, &obj1->neigh );
	h.add(obj2->name, &obj2->neigh );
	h.remove(obj2->name, &obj2->neigh );
	
	neighbor<myobject> *ring1 = h.lookup("first");
	if ( ring1 )
	{
		printf(" item: %s\n", ring1->getObject()->name);
		for(neighbor<myobject> *item = ring1->getNext(); item != ring1; item = item->getNext())
		{
			printf("  item: %s\n", item->getObject()->name);
		}
	}
	else
	{
		printf("  no items\n");
	}
	
	printf("\n\nend\n");
	
	return 0;
}

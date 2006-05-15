#include <qobject.h>
#include <stdio.h>

#include "testit.h"

void Foo::setValue(int v)
{
	if (v != val) {
		val = v;
		emit valueChanged(v);
	}
}

Foo::Foo()
{
}

void main()
{
  Foo a, b;

  QObject::connect(&a, SIGNAL(valueChanged(int)), 
		&b, SLOT(setValue(int)));
  b.setValue(11);
  a.setValue(79);
  printf("b is now: %d\n",b.value());
}
#include "testit.moc"

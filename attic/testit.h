#include <qobject.h>

class Foo: public QObject
{
	Q_OBJECT;
public:
	Foo();
	int value() const { return val; }
public slots:
	void setValue(int);
signals:
	void valueChanged(int);
private:
	int val;
};

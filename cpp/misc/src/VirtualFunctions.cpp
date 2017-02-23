/*
Testing for virtual functions.
*/

#include "stdafx.h"
#include <stdio.h>

class A
{
public:
    virtual void Foo()
    {
        printf("Calling A::Foo()\n");
    }

    void Foo(int n)
    {
        printf("Calling A::Foo(int n);\n");
    }
};

class B : public A
{
public:
    void Foo(int n=0)
    {
        printf("Calling B::Foo(int n);\n");
    }
};

class C : public B
{
public:
    void Foo()
    {
        printf("Calling C::Foo();\n");
    }
};

int main()
{
    C objTestC;
    A* pTestA = &objTestC;
    B* pTestB = &objTestC;

    printf("pTestA->Foo() : ");
    pTestA->Foo();

	printf("pTestA->Foo(0) : ");
	pTestA->Foo(0);

    printf("pTestB->Foo(0) : ");
    pTestB->Foo(0);

    getchar();
    
    return 0;
}


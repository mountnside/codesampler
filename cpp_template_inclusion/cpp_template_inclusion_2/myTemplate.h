#ifndef MY_TEMPLATE_H
#define MY_TEMPLATE_H

//------------------------------------------------------------------------------
// By placing both the declaration and definition of our template in its
// header file, we can avoid the linker problem demonstrated in the 
// "cpp_template_inclusion_1" sample. In short, our header now contains enough 
// information to allow the compiler to instantiate code from our template using 
// any type.
//------------------------------------------------------------------------------

template < typename T >
void printTypeofInfo( T const& x )
{
    std::cout << typeid(x).name() << std::endl;
}

#endif // MY_TEMPLATE_H

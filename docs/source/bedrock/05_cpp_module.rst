Write a Bedrock module in C++
=============================

If your component is written in C++, it might be easier to
write the Bedrock module library in C++. We still recommend
reading the previous section to understand how a C module works.

The following code sample showcases a C++ module.

.. literalinclude:: ../../../code/bedrock/05_cpp_module/module.cpp
   :language: cpp

In C++, a Bedrock module is created by writing a class that
inherits from :code:`bedrock::AbstractServiceFactory`.
The class must satisfy the interface of its abstract parent
class. What in the C module were functions now become member
functions of our class, and some of the constructs differ
slightly. For example, the :code:`bedrock::Dependency` class
uses :code:`std::string` for the name and type of the dependency,
and the arguments provided to the provider registration method
and client initialization method is not opaque.

The :code:`BEDROCK_REGISTER_MODULE_FACTORY` macro should be
called in place of the C module's :code:`BEDROCK_REGISTER_MODULE`.

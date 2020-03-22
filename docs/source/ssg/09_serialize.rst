Serializing an SSG group
========================

It may sometimes be useful to serialize a group id into
a buffer to send this buffer to another process.
The following code sample illustrates how to do this
using :code:`ssg_group_id_serialize()` and :code:`ssg_group_id_deserialize()`.

.. container:: toggle

    .. container:: header
    
       .. container:: btn btn-info

          main.c (show/hide)

    .. literalinclude:: ../../../code/ssg/09_serialize/main.c
       :language: cpp

.. note::
   The :code:`ssg_group_id_serialize` takes a pointer to a buffer
   and will perform an allocation of this buffer. It is the caller's
   responsibility to free this buffer.

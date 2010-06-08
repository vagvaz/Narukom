message ( STATUS "")
message ( STATUS  " _..::: TinyXml 2-6-1 :::.._ " )
message ( STATUS "")


add_definitions ( -DTINYXML_USE_STL )


add_library ( tinyxml 
        tinystr.cpp tinyxml.cpp 
        tinyxmlerror.cpp tinyxmlparser.cpp
)


#install ( TARGETS M_tinyxml 
#            RUNTIME DESTINATION   bin
#            LIBRARY DESTINATION   lib
#            ARCHIVE DESTINATION   lib/static )


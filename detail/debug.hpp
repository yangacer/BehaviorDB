#define BDB_CERR std::cerr << __FILE__ << ":" << \
         <<__FUNCTION__ << "(): " std::dec << __LINE__ << std::endl

#define BDB_ERR(F) fprintf(F, __FILE__ ":" __FUNCTION__ "():" __LINE__ "\n")

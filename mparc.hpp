#ifndef _MXPSQL_MPARC_CXX
#define _MXPSQL_MPARC_CXX

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstddef>

#include <cstdlib>

#include "./mparc.h"

namespace MXPSQL{
    namespace MPARC{
        class MPARC_Error{
            private:
            MXPSQL_MPARC_err err = MPARC_OK;

            public:
            /**
             * @brief Construct a new mparc error object with default values
             * 
             */
            MPARC_Error() : MPARC_Error(MPARC_OK) {}
            /**
             * @brief Construct a new mparc error object with OUR Values
             * 
             * @param in OUR Values
             */
            MPARC_Error(MXPSQL_MPARC_err in) : err(in) {}

            /**
             * @brief Get the Err enumeration
             * 
             * @return MXPSQL_MPARC_err Err enumeration
             */
            MXPSQL_MPARC_err getErr() {return err;}
            /**
             * @brief Set the Err enumeration
             * 
             * @param err Err enumeration
             */
            void setErr(MXPSQL_MPARC_err err) {this->err = err;}
            /**
             * @brief Die by abortion if I am not fine
             * 
             */
            void OrDie() {
                if(this->getErr() != MPARC_OK){
                    std::abort();
                }
            }

            /**
             * @brief Throw a std::runtime_error if I am not fine
             * 
             */
            void OrThrow(){
                if(this->getErr() != MPARC_OK){
                    std::string str = std::string("MPARC Runtime Error with code of ") + std::to_string(((int)this->getErr()));
                    throw std::runtime_error(str.c_str());
                }
            }

            bool isOk(){return (this->getErr() == MPARC_OK);}
        };

        class MPARC{
            private:
            MXPSQL_MPARC_t* archive = NULL;

            public:
            MPARC() {
                MXPSQL_MPARC_err err = MPARC_init(&archive);
                MPARC_Error(err).OrThrow(); // throw
            }

            MPARC(const char* path) : MPARC() {
                MXPSQL_MPARC_err err = MPARC_parse_filename(archive, path);
                MPARC_Error(err).OrThrow();
            }

            MPARC(std::string path) : MPARC(path.c_str()) {}

            MPARC(MPARC& other) : MPARC() {
                MXPSQL_MPARC_t* Ptr = other.getInstance();
                MXPSQL_MPARC_err err = MPARC_copy(&Ptr, &archive);
                MPARC_Error(err).OrThrow();
            }

            ~MPARC(){
                MPARC_destroy(&archive);
            }



            /**
             * @brief Get the internal MXPSQL_MPARC_t instance
             * 
             * @return MXPSQL_MPARC_t* pointer to the internal instance
             * 
             * @note Whatever you do, resist the temptation to free this manually
             */
            MXPSQL_MPARC_t* getInstance(){
                return archive;
            }


            MPARC_Error list(std::vector<std::string>& out){
                MPARC_Error err(MPARC_OK);

                {
                    MXPSQL_MPARC_iter_t* iter = NULL;
                    MXPSQL_MPARC_err ierr = MPARC_list_iterator_init(&archive, &iter);
                    if(ierr != MPARC_OK){
                        err.setErr(ierr);
                        return err;
                    }

                    const char* outnam = NULL;

                    while((ierr = MPARC_list_iterator_next(&iter, &outnam)) == MPARC_OK){
                        out.push_back(std::string(outnam));
                    }

                    MPARC_list_iterator_destroy(&iter);

                    if(ierr != MPARC_KNOEXIST){
                        err.setErr(ierr);
                        return err;
                    }
                }

                return err;
            }            
        };
    }
}



#endif

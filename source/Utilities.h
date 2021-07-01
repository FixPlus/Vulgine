//
// Created by Бушев Дмитрий on 01.07.2021.
//

#ifndef VULGINE_UTILITIES_H
#define VULGINE_UTILITIES_H
#include <string>
#include <ostream>

namespace Vulgine{
    class Logger{
        std::ostream* output;
        bool enabled = true;
    public:

        explicit Logger(std::ostream* stream): output(stream){}
        void operator()(std::string const& log){
            if(enabled)
                *output << log << std::endl;
        }

        void changeLogFile(std::ostream* new_out){
            output = new_out;
        }

        [[nodiscard]] bool hasNullLogFile() const{
            return output == nullptr;
        }

        void enable(){
            enabled = true;
        }

        void disable(){
            enabled = false;
        }
    };

    extern Logger logger;

    class Errs{
        std::string lastError;
    public:
        void operator()(std::string const& err) {
            logger("[ERROR]: " + err);
            lastError = err;
        }

        [[nodiscard]] std::string get() const{
            return lastError;
        }
    };

    extern Errs errs;
}

#endif //VULGINE_UTILITIES_H
